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
void emui_splitter_draw(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
void emui_splitter_debug(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
int emui_splitter_update_geometry(struct emui_tile *t)
{
	return 0;
}

// -----------------------------------------------------------------------
int emui_splitter_event_handler(struct emui_tile *t, struct emui_event *ev)
{   
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_splitter_destroy_priv_data(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_splitter_drv = {
	.draw = emui_splitter_draw,
	.debug = emui_splitter_debug,
	.update_geometry = emui_splitter_update_geometry,
	.event_handler = emui_splitter_event_handler,
	.destroy_priv_data = emui_splitter_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_splitter_new(struct emui_tile *parent, int x, int y, int h)
{
	struct emui_tile *t;

	//t = emui_tile_create(parent, &emui_splitter_drv, T_CONTAINER, parent->x, parent->y, parent->h, parent->w, 0, 0, 0, 0, name_buf, P_MAXIMIZED | P_CHILD_CTRL);
	t = emui_tile_create(parent, &emui_splitter_drv, T_CONTAINER, parent->x+x+parent->ml, parent->y+y+parent->mt, parent->w-2, h, 0, 0, 0, 0, "Splitter", P_NONE);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
