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
void emui_tabs_draw(struct emui_tile *t)
{
	struct emui_tile *ch = t->child_h;
	wmove(t->ncdeco, 0, 0);
	while (ch) {
		// TODO: check if we have enough sapce
		wprintw(t->ncdeco, " [%s] ", ch->name);
		ch = ch->next;
	}
}

// -----------------------------------------------------------------------
void emui_tabs_debug(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
int emui_tabs_update_geometry(struct emui_tile *t)
{
	return 0;
}

// -----------------------------------------------------------------------
int emui_tabs_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_tabs_destroy_priv_data(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_tabs_drv = {
	.draw = emui_tabs_draw,
	.debug = emui_tabs_debug,
	.update_geometry = emui_tabs_update_geometry,
	.event_handler = emui_tabs_event_handler,
	.destroy_priv_data = emui_tabs_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_tabs_new(struct emui_tile *parent)
{
	struct emui_tile *t = emui_tile_create(parent, &emui_tabs_drv, T_CONTAINER, parent->x, parent->y, parent->h, parent->w, 1, 0, 0, 0, "Tabs", P_MAXIMIZED);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
