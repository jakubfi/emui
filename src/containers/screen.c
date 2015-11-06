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
static void _screen_update_geometry(struct emui_tile *t)
{
	// it seems that getmaxyx() without preceding wrefresh() can sometimes
	// spit wrong numbers after screen resize
	wrefresh(t->ncwin);
	getmaxyx(t->ncwin, t->e.h, t->e.w);
	t->i.h = t->r.h = t->e.h;
	t->i.w = t->r.w = t->e.w;
	t->geometry_changed = 1;
}

// -----------------------------------------------------------------------
int emui_screen_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	if (ev->type == EV_RESIZE) {
		_screen_update_geometry(t);
		return 0;
	}

	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_screen_drv = {
	.draw = NULL,
	.update_children_geometry = NULL,
	.event_handler = emui_screen_event_handler,
	.destroy_priv_data = NULL,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_screen()
{
	struct emui_tile *t = calloc(1, sizeof(struct emui_tile));
	t->ncwin = stdscr;
	t->family = F_CONTAINER;
	t->drv = &emui_screen_drv;
	t->name = strdup("SCREEN");
	t->properties = P_FOCUS_GROUP;
	t->id = -1;
	t->i.x = t->r.x = t->e.x = 0;
	t->i.y = t->r.y = t->e.y = 0;
	t->mr = t->ml = t->mt = t->mb = 0;
	_screen_update_geometry(t);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
