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

#include "tile.h"
#include "event.h"
#include "style.h"
#include "print.h"

struct W_NAME {

};

// -----------------------------------------------------------------------
void emui_W_NAME_draw(struct emui_tile *t)
{
	struct label *d = t->priv_data;
}

// -----------------------------------------------------------------------
void emui_W_NAME_debug(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
int emui_W_NAME_update_geometry(struct emui_tile *t)
{
	return 0;
}

// -----------------------------------------------------------------------
int emui_W_NAME_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_W_NAME_destroy_priv_data(struct emui_tile *t)
{
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_W_NAME_drv = {
	.draw = emui_W_NAME_draw,
	.debug = emui_W_NAME_debug,
	.update_geometry = emui_W_NAME_update_geometry,
	.event_handler = emui_W_NAME_event_handler,
	.destroy_priv_data = emui_W_NAME_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_W_NAME(struct emui_tile *parent, int x, int y, int w, int h)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, -1, &emui_W_NAME_drv, F_WIDGET, x, y, w, h, 0, 0, 0, 0, NULL, P_INTERACTIVE);

	t->priv_data = calloc(1, sizeof(struct W_NAME));
	struct W_NAME *d = t->priv_data;

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
