//  Copyright (c) 2015 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/select.h>
#include <ncurses.h>
#include <sys/time.h>

#include "event.h"
#include "tile.h"
#include "style.h"

#define EMUI_FPS_DEFAULT 30
#define EMUI_FPS_CAP 1000

#define EMUI_ESC_TIMEOUT 100
#define EMUI_CURSOR 0

SCREEN *s;
static struct emui_tile *layout;

static int emui_fps;
static long emui_frame;
long emui_ft;

// -----------------------------------------------------------------------
void edbg(char *format, ...)
{
	va_list vl;
	va_start(vl, format);
	FILE *f = fopen("/home/amo/emui.log", "a");
	vfprintf(f, format, vl);
	fclose(f);
	va_end(vl);
}

// -----------------------------------------------------------------------
struct emui_tile * emui_init(unsigned fps)
{
	// initialize ncurses
	s = newterm(NULL, stdout, stdin);
	set_term(s);
	//initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	timeout(0);
	curs_set(EMUI_CURSOR);
	set_escdelay(EMUI_ESC_TIMEOUT);
	start_color();

	// initialize emui
	if (fps > EMUI_FPS_CAP) {
		emui_fps = EMUI_FPS_CAP;
	} else {
		emui_fps = fps;
	}
	emui_scheme_default();
	layout = emui_screen_new();

	return layout;
}

// -----------------------------------------------------------------------
void emui_destroy()
{
	emui_tile_destroy(layout);
	endwin();
	//_nc_free_and_exit();
	delscreen(s);
	emui_evq_clear();
}

// -----------------------------------------------------------------------
static void emui_evq_update(struct timeval *tv)
{
	static fd_set rfds;
	int retval;
	int ch;
	struct emui_event *ev = NULL;

	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	retval = select(1, &rfds, NULL, NULL, tv);

	if (retval != 0) {
		ev = calloc(1, sizeof(struct emui_event));
		ch = getch();
		if (ch == KEY_RESIZE) {
			ev->type = EV_RESIZE;
		} else {
			ev->type = EV_KEY;
			ev->data.key = ch;
		}
		emui_evq_append(ev);
	}
}

// -----------------------------------------------------------------------
static int emui_event_router(struct emui_event *ev)
{
	edbg("--- Got event: %i %i\n", ev->type, ev->data.key);
	struct emui_tile *t = emui_tile_focus_get();
	while (t) {
		edbg("\"%s\" handling event: %i %i\n", t->name, ev->type, ev->data.key);
		if (emui_tile_handle_event(t, ev) == 0) {
			edbg("success\n");
			// 0 means event has been handled by the tile
			return 0;
		}
		edbg("switching to parent\n");
		t = t->parent;
	}

	edbg("EVENT FALLS OF THE EDGE: %i %i\n", ev->type, ev->data.key);
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
static void emui_draw(struct emui_tile *t)
{
	struct emui_tile *focused_child = NULL;

	if (!t) return;

	// update common tile geometry stuff
	emui_tile_update_geometry(t);
	// draw the tile
	emui_tile_draw(t);

	// draw children
	struct emui_tile *child = t->child_h;
	while (child) {
		// draw non-focused tiles first, store focused
		if (!emui_tile_has_focus(child)) {
			emui_draw(child);
		} else {
			focused_child = child;
		}
		child = child->next;
	}

	// finally, draw focused tile
	emui_draw(focused_child);
}

// -----------------------------------------------------------------------
static void emui_adjust_frametime(struct timeval *ft, unsigned fps)
{
	static struct timeval work_old;
	static long correction = 0;
	static const long resolution = 1000000;

	struct timeval work;
	long ft_actual;
	long ft_requested;

	// not much to do for framerate 0
	if (!ft) {
		return;
	}

	ft_requested = resolution/fps;

	// get the actual work time
	gettimeofday(&work, NULL);
	ft_actual = work.tv_usec - work_old.tv_usec + resolution * (work.tv_sec - work_old.tv_sec);
	work_old.tv_sec = work.tv_sec;
	work_old.tv_usec = work.tv_usec;

	// work took longer than expected
	if (ft_actual > ft_requested) {
		if (correction > -ft_requested/2) {
			correction -= 13;
		}
	// work took shorter than expected
	} else {
		if (correction < ft_requested/2) {
			correction += 14;
		}
	}

	// set the frametime
	ft->tv_sec = 0;
	ft->tv_usec = ft_requested + correction;
	emui_ft = correction;
}

// -----------------------------------------------------------------------
static inline int emui_need_screen_update(struct timeval *ft)
{
	// update is needed if:
	//  - frametime is not being used (UI is event-driven only)
	//  - or if frametime has expired
	return (!ft) || ((ft->tv_sec <= 0) && (ft->tv_usec <= 0));
}

// -----------------------------------------------------------------------
static void emui_update_screen()
{
	emui_draw(layout);
	doupdate();
	emui_frame++;
}

// -----------------------------------------------------------------------
void emui_loop()
{
	struct timeval *ft = NULL;
	struct emui_event *ev;

	// init focus
	emui_tile_focus(layout);

	// init frametime
	if (emui_fps > 0) {
		ft = calloc(1, sizeof(struct timeval));
	}

	// initial screen draw
	emui_update_screen();

	while (1) {
		// get event from the queue
		ev = emui_evq_get();

		// process the event, if any
		if (ev) {
			if (ev->type == EV_DIE) {
				free(ev);
				break;
			} else {
				emui_event_router(ev);
			}
			free(ev);
		// or wait for another event
		// TODO: we may starve screen updater here
		} else {
			emui_evq_update(ft);
			// update screen if needed
			if (emui_need_screen_update(ft)) {
				emui_update_screen();
				emui_adjust_frametime(ft, emui_fps);
			}
		}
	}

	free(ft);
}

// -----------------------------------------------------------------------
unsigned emui_get_fps()
{
	return emui_fps;
}

// -----------------------------------------------------------------------
long emui_get_frame()
{
	return emui_frame;
}

// vim: tabstop=4 shiftwidth=4 autoindent
