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
#include <errno.h>

#include "event.h"
#include "tiles.h"
#include "style.h"
#include "focus.h"

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
	layout = emui_screen();

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
static int emui_evq_update(struct timeval *tv)
{
	static fd_set rfds;
	int retval;
	int ch;
	struct emui_event *ev = NULL;

	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	retval = select(1, &rfds, NULL, NULL, tv);

	if (retval == 0) {
		return 0;
	}

	ev = calloc(1, sizeof(struct emui_event));

	// we have a keypress
	if (retval > 0) {
		ch = getch();
		ev->type = EV_KEY;
		ev->sender = ch;
	// this may be a resize event
	} else if ((errno == EINTR) && ((ch = getch() == KEY_RESIZE))) {
		ev->type = EV_RESIZE;
	// error
	} else {
		ev->type = EV_ERROR;
		ev->sender = errno;
	}

	emui_evq_append(ev);

	return 1;
}

// -----------------------------------------------------------------------
static void emui_draw(struct emui_tile *t, int force)
{
	struct emui_tile *focused_child = NULL;

	if (!t) return;

	// do we need to update tile geometry?
	int geometry_update = (t->geometry_changed || force);
	if (geometry_update) {
		// update common tile geometry stuff
		emui_tile_update_geometry(t);
		// do tile-specific geometry updates
		t->drv->update_geometry(t);
		t->geometry_changed = 0;
	}

	// draw the tile
	if (emui_tile_draw(t)) {
		// nothing drawn, tile is hidden, give up on children too
		return;
	}

	// draw children
	struct emui_tile *child = t->ch_first;
	while (child) {
		// draw non-focused tiles first, store focused
		if (!emui_has_focus(child)) {
			emui_draw(child, geometry_update);
		} else {
			focused_child = child;
		}
		child = child->next;
	}

	// finally, draw focused tile
	emui_draw(focused_child, geometry_update);
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
	emui_draw(layout, 0);
	doupdate();
	emui_frame++;
}

// -----------------------------------------------------------------------
static int emui_handle_user_focus_keys(struct emui_tile *fg, int key)
{
	struct emui_tile *t = fg->fg_first;

	while (t) {
		if (t->key == key) {
			emui_focus(t);
			return 0;
		}
		t = t->fg_next;
	}

	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
static int emui_handle_neighbour_focus(struct emui_tile *t, int key)
{
	switch (key) {
		case 9: // TAB
			emui_focus_list_neighbour(t, FC_NEXT);
			return 0;
		case KEY_BTAB:
			emui_focus_list_neighbour(t, FC_PREV);
			return 0;
		case KEY_UP:
			emui_focus_physical_neighbour(t, FC_UP);
			return 0;
		case KEY_DOWN:
			emui_focus_physical_neighbour(t, FC_DOWN);
			return 0;
		case KEY_LEFT:
			emui_focus_physical_neighbour(t, FC_LEFT);
			return 0;
		case KEY_RIGHT:
			emui_focus_physical_neighbour(t, FC_RIGHT);
			return 0;
		default:
			return 1;
	}
}

// -----------------------------------------------------------------------
static int emui_handle_focus(struct emui_tile *t, int key)
{
	if (!t) return 1;

	// if tile is a top of a focus group, search for user-set focus keys handled by it
	if (t->properties & P_FOCUS_GROUP) {
		if (!emui_handle_user_focus_keys(t, key)) {
			return 0;
		}
	// if tile is a widget, handle neighbourhood focus change
	} else if (t->family == F_WIDGET) {
		if (!emui_handle_neighbour_focus(t, key)) {
			return 0;
		}
	}

	// finally, if above didn't work, propagate the event to the focus group above
	if (!emui_handle_focus(t->fg, key)) {
		return 0;
	}

	// event has not been handled, bummer
	return 1;
}

// -----------------------------------------------------------------------
static int emui_process_event(struct emui_event *ev)
{
	struct emui_tile *t = emui_focus_get();

	// resize goes straight to the top tile
	if ((ev->type == EV_RESIZE) && !layout->drv->event_handler(layout, ev)) {
		return 0;
	}

	// TODO: temporary
	if ((ev->type == EV_KEY) && (ev->sender == 'q')) {
		struct emui_event *ev = malloc(sizeof(struct emui_event));
		ev->type = EV_QUIT;
		emui_evq_prepend(ev);
		return 0;
	}

	// try running user key handler
	if ((ev->type == EV_KEY) && t->user_key_handler && !t->user_key_handler(t, ev->sender)) {
		return 0;
	}

	// run tile's own event handler
	if (!t->drv->event_handler(t, ev)) {
		return 0;
	}

	// try the key to change focus
	if ((ev->type == EV_KEY) && !emui_handle_focus(t, ev->sender)) {
		return 0;
	}

	return 1;
}

// -----------------------------------------------------------------------
void emui_loop()
{
	struct timeval *ft = NULL;
	struct emui_event *ev;

	// init focus
	emui_focus(layout);

	// init frametime
	if (emui_fps > 0) {
		ft = calloc(1, sizeof(struct timeval));
	}

	while (1) {
		// update the screen if:
		//  * loop has just started
		//  * FPS = 0 (so we update as soon as event is processed)
		//  * frame timeout reached (so we don't update more than FPS)
		if ((emui_fps == 0) || emui_need_screen_update(ft)) {
			emui_update_screen();
			emui_adjust_frametime(ft, emui_fps);
		}

process_event:
		// get event from the queue
		ev = emui_evq_get();

		if (ev) {
			if (ev->type == EV_QUIT) {
				free(ev);
				break;
			} else {
				// process the event through the focus path
				//emui_tile_handle_event(emui_focus_get(), ev);
				emui_process_event(ev);
				free(ev);
			}
		} else {
			// get another event (or wait ft.tv_usec)
			if (emui_evq_update(ft)) {
				// if there is an event waiting:
				//  * we want to process it as soon as possible
				//  * we want to process it before screen update in case of FPS=0
				//  * there is no need for screen update (timeout not reached,
				//    emui_need_screen_update() would skip the update anyway)
				goto process_event;
			}
		}
	};

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
