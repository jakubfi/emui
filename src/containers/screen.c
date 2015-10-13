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
#include <string.h>
#include <ncurses.h>

#include "tile.h"
#include "event.h"

// -----------------------------------------------------------------------
void emui_screen_draw(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
void emui_screen_debug(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
int emui_screen_update_geometry(struct emui_tile *t)
{
	// handle resize here, because if terminal is resized quickly enough,
	// window size may change more during resize event, leaving UI
	// not fully scaled to the terminal size
	t->x = t->rx = t->dx = 0;
	t->y = t->ry = t->dy = 0;
	t->mr = t->ml = t->mt = t->mb = 0;
	// it seems that getmaxyx() without preceding wrefresh() can sometimes
	// spit wrong numbers after screen resize
	wrefresh(t->ncwin);
	getmaxyx(t->ncwin, t->dh, t->dw);
	t->h = t->rh = t->dh;
	t->w = t->rw = t->dw;

	return 0;
}

// -----------------------------------------------------------------------
int emui_screen_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	if (ev->type == EV_RESIZE) {
		// handle resize here, so if terminal is resized when fps is low,
		// UI reacts quickly
		t->geometry_changed = 1;
	} else if ((ev->type == EV_KEY) && (ev->sender == 'q')) {
		struct emui_event *ev = malloc(sizeof(struct emui_event));
		ev->type = EV_QUIT;
		emui_evq_prepend(ev);
		return 0;
	} else if (ev->type == EV_QUIT) {
		// TODO: call user's quit routine
		struct emui_event *ev = malloc(sizeof(struct emui_event));
		ev->type = EV_DIE;
		emui_evq_prepend(ev);
		return 0;
	}

	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_screen_destroy_priv_data(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_screen_drv = {
	.draw = emui_screen_draw,
	.debug = emui_screen_debug,
	.update_geometry = emui_screen_update_geometry,
	.event_handler = emui_screen_event_handler,
	.destroy_priv_data = emui_screen_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_screen()
{
	struct emui_tile *t = calloc(1, sizeof(struct emui_tile));
	t->ncdeco = NULL;
	t->ncwin = stdscr;
	t->family = F_CONTAINER;
	t->drv = &emui_screen_drv;
	t->name = strdup("SCREEN");
	t->properties = P_FOCUS_GROUP;

	emui_screen_update_geometry(t);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
