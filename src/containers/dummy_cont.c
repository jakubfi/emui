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

#include "tile.h"
#include "event.h"

// -----------------------------------------------------------------------
struct emui_tile_drv emui_dummy_cont_drv = {
	.draw = NULL,
	.update_geometry = NULL,
	.event_handler = NULL,
	.destroy_priv_data = NULL,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_dummy_cont(struct emui_tile *parent, int x, int y, int w, int h)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, -1, &emui_dummy_cont_drv, F_CONTAINER, x, y, w, h, 0, 0, 0, 0, "DummyCont", P_NONE);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
