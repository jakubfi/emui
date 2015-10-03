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

#include "emui.h"
#include "tile.h"
#include "event.h"
#include "style.h"
#include "print.h"

// -----------------------------------------------------------------------
void emui_framecounter_draw(struct emui_tile *t)
{
	emuixyprt(t, 0, 0, S_DEFAULT, "%i", emui_get_frame());
}

// -----------------------------------------------------------------------
void emui_framecounter_debug(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
int emui_framecounter_update_geometry(struct emui_tile *t)
{
	return 0;
}

// -----------------------------------------------------------------------
int emui_framecounter_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_framecounter_destroy_priv_data(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_framecounter_drv = {
	.draw = emui_framecounter_draw,
	.debug = emui_framecounter_debug,
	.update_geometry = emui_framecounter_update_geometry,
	.event_handler = emui_framecounter_event_handler,
	.destroy_priv_data = emui_framecounter_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_framecounter_new(struct emui_tile *parent, int x, int y)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, &emui_framecounter_drv, T_WIDGET, x, y, 1, 20, 0, 0, 0, 0, NULL, P_NONE);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
