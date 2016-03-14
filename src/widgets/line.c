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

struct line {
	int align;
};

// -----------------------------------------------------------------------
void emui_line_draw(EMTILE *t)
{
	struct line *d = t->priv_data;

	if (d->align == AL_HORIZONTAL) {
		emuihline(t, 0, 0, t->i.w, t->style);
	} else if (d->align == AL_VERTICAL) {
		emuivline(t, 0, 0, t->i.h, t->style);
	}
}

// -----------------------------------------------------------------------
void emui_line_destroy_priv_data(EMTILE *t)
{
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emtile_drv emui_line_drv = {
	.draw = emui_line_draw,
	.update_children_geometry = NULL,
	.event_handler = NULL,
	.destroy_priv_data = emui_line_destroy_priv_data,
};

// -----------------------------------------------------------------------
EMTILE * emui_line(EMTILE *parent, int align, int x, int y, int len)
{
	EMTILE *t;
	int w, h;

	if (align == AL_HORIZONTAL) {
		w = len;
		h = 1;
	} else if (align == AL_VERTICAL) {
		w = 1;
		h = len;
	} else {
		return NULL;
	}

	t = emtile(parent, &emui_line_drv, x, y, w, h, 0, 0, 0, 0, "Line", P_NONE);

	t->priv_data = calloc(1, sizeof(struct line));
	struct line *d = t->priv_data;
	d->align = align;

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
