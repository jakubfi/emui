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
#include "focus.h"

// -----------------------------------------------------------------------
void emui_window_draw(struct emui_tile *t)
{
	int title_style = S_TITLE_NN;
	int frame_style = S_FRAME_NN;

	if (emui_has_focus(t)) {
		title_style = S_TITLE_FN;
		frame_style = S_FRAME_FN;
	}

	emuibox(t, frame_style);
	emuixyprt(t, 2, 0, title_style, "[ %s ]", t->name);
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_window_drv = {
	.draw = emui_window_draw,
	.update_children_geometry = NULL,
	.event_handler = NULL,
	.destroy_priv_data = NULL,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_window(struct emui_tile *parent, int x, int y, int w, int h, char *name, int properties)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, -1, &emui_window_drv, F_WINDOW, x, y, w, h, 1, 1, 1, 1, name, P_FOCUS_GROUP);

	emui_tile_set_properties(t, properties);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
