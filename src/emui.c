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
#include <signal.h>
#include <stdio.h>
#include <sys/select.h>
#include <ncurses.h>
#include <sys/time.h>
#include <errno.h>

#include "dbg.h"
#include "event.h"
#include "tiles.h"
#include "style.h"
#include "focus.h"

#define EMUI_FPS_CAP 60

static SCREEN *s;
static EMTILE *layout;

static int fps_min;
static int fps_frame_mod;
static float fps_current;
static unsigned long frame_current;
static volatile int terminal_resized;

// -----------------------------------------------------------------------
static void _emui_sigwinch_handler(int signum)
{
	if (signum == SIGWINCH) {
		terminal_resized = 1;
	}
	while (signal(SIGWINCH, _emui_sigwinch_handler));
}

// -----------------------------------------------------------------------
EMTILE * emui_init(unsigned fps)
{
	// initialize ncurses
	s = newterm(NULL, stdout, stdin);
	set_term(s);
	//initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	timeout(0);
	curs_set(0);
	set_escdelay(100);
	start_color();
	emui_style_init(NULL);

	// initialize emui
	if (fps > EMUI_FPS_CAP) {
		fps_min = EMUI_FPS_CAP;
	} else {
		fps_min = fps;
	}

	// modulo for fps counter
	fps_frame_mod = fps_min / 4;
	if (fps_frame_mod <= 0) {
		fps_frame_mod = 1;
	}
	layout = emui_screen();

	if (signal(SIGWINCH, _emui_sigwinch_handler) == SIG_ERR) {
		return NULL;
	}

	return layout;
}

// -----------------------------------------------------------------------
void emui_destroy()
{
	_emtile_really_delete(layout);
	endwin();
	//_nc_free_and_exit();
	delscreen(s);
	emui_evq_clear();
}

// -----------------------------------------------------------------------
static void emui_draw(EMTILE *t)
{
	EMTILE *focused_child = NULL;

	// physically delete tile marked P_DELETED
	if (t->properties & P_DELETED) {
		_emtile_really_delete(t);
		return;
	}

	// update tile geometry
	int geometry_changed = t->geometry_changed;
	if (geometry_changed) {
		EDBG(t, 0, "Tile geometry changed");
		emtile_fit(t);

		// if the focused tile is hidden after geometry change,
		// and there is no scroll handler in tile's focus group,
		// search for a new unhidden tile:
		// go up, then left, then from the beggining of focus group
		EMTILE *f = emui_focus_get();
		if (f->properties & P_HIDDEN) {
			if (f->fg->drv->scroll_handler) {
				// TODO: ???
			} else {
				EDBG(f, 0, "Tile is hidden after geometry change, moving focus");
				f = emtile_get_physical_neighbour(f->fg, FC_ABOVE, P_INTERACTIVE, P_HIDDEN);
				if (f->properties & P_HIDDEN) {
					f = emtile_get_physical_neighbour(f->fg, FC_LEFT, P_INTERACTIVE, P_HIDDEN);
					if (f->properties & P_HIDDEN) {
						f = emtile_get_list_neighbour(f->fg, FC_FIRST, P_INTERACTIVE, P_HIDDEN);
					}
				}
				emui_focus(f);
			}
		}
	}

	// draw the tile
	emtile_draw(t);

	// draw tile's children
	EMTILE *child = t->ch_first;
	while (child) {
		child->geometry_changed |= geometry_changed;
		// store focused tile to draw it later
		if (!emui_has_focus(child)) {
			emui_draw(child);
		} else {
			focused_child = child;
		}
		child = child->ch_next;
	}

	// draw focused tile last, so it's always on top
	if (focused_child) {
		emui_draw(focused_child);
	}
}

// -----------------------------------------------------------------------
static void emui_update_screen()
{
	struct timeval work_start;
	static struct timeval work_start_old;
	const long resolution = 1000000L;

	// calculate the real fps
	gettimeofday(&work_start, NULL);
	if (frame_current % fps_frame_mod == 0) {
		long frame_time = resolution * (work_start.tv_sec - work_start_old.tv_sec);
		frame_time += work_start.tv_usec - work_start_old.tv_usec;
		fps_current = (float) fps_frame_mod * resolution / frame_time;
		work_start_old = work_start;
	}

	if (terminal_resized > 0) {
		terminal_resized = 0;
		layout->geometry_changed = 1;
	}

	emui_draw(layout);
	doupdate();
	frame_current++;
}

// -----------------------------------------------------------------------
static int emui_process_event(struct emui_event *ev)
{
	EMTILE *t = emui_focus_get();

	while (t) {
		if (emtile_event(t, ev) == E_HANDLED) {
			return E_HANDLED;
		}
		t = t->parent;
	}

	// TODO: temporary
	if ((ev->type == EV_KEY) && (ev->sender == 'q')) {
		struct emui_event *ev = malloc(sizeof(struct emui_event));
		ev->type = EV_QUIT;
		emui_evq_prepend(ev);
		return E_HANDLED;
	}

	return E_UNHANDLED;
}

// -----------------------------------------------------------------------
static int emui_process_queue()
{
	struct emui_event *ev;

	while ((ev = emui_evq_get())) {
		if (ev->type != EV_QUIT) {
			emui_process_event(ev);
			free(ev);
		} else {
			free(ev);
			return -1;
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
static int emui_do_events(int fps)
{
	static fd_set rfds;
	int retval;
	int ch;
	struct emui_event *ev = NULL;

	struct timeval work_start, work_end;

	// wait up to 1000000/fps usec for an event
	struct timeval tv;
	int ftime = 1000000/fps;
	tv.tv_sec = 0;
	tv.tv_usec = ftime;

	// respond to evens in 1000000/61 usec max...
	while (1000000L*tv.tv_sec + tv.tv_usec > (ftime-(1000000L/61))) {

		FD_ZERO(&rfds);
		FD_SET(0, &rfds);

		// ...but wait up to 1000000/fps usec for an event
		retval = select(1, &rfds, NULL, NULL, &tv);

		// nothing to do (timeout)
		if (retval == 0) {
			continue;
		}

		gettimeofday(&work_start, NULL);

		ev = calloc(1, sizeof(struct emui_event));

		// we have a keypress
		if (retval > 0) {
			ch = getch();
			ev->type = EV_KEY;
			ev->sender = ch;
		// error
		} else {
			ev->type = EV_ERROR;
			ev->sender = errno;
		}

		// append the event
		emui_evq_append(ev);

		// process all events (processing an event may cause enqueuing another event)
		if (emui_process_queue()) {
			return -1;
		}

		// subtract time wasted on processing
		gettimeofday(&work_end, NULL);
		long work_time = 1000000L * (work_end.tv_usec - work_start.tv_usec) + work_end.tv_usec - work_start.tv_usec;
		tv.tv_usec -= work_time;
	}
	return 0;
}

// -----------------------------------------------------------------------
void emui_loop()
{
	emui_focus(layout);

	while (1) {
		emui_update_screen();
		if (emui_do_events(fps_min)) {
			break;
		}
	}

	EDBG(layout, 0, "QUIT");
}

// -----------------------------------------------------------------------
EMTILE * emui_get_layout()
{
	return layout;
}

// -----------------------------------------------------------------------
unsigned emui_get_target_fps()
{
	return fps_min;
}

// -----------------------------------------------------------------------
float emui_get_current_fps()
{
	return fps_current;
}

// -----------------------------------------------------------------------
unsigned long emui_get_current_frame()
{
	return frame_current;
}

// vim: tabstop=4 shiftwidth=4 autoindent
