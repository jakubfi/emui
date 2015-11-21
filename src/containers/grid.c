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

struct grid {
	int col_width;
	int row_height;
	int cols;
	int rows;
	int col_spacing;
};

// -----------------------------------------------------------------------
void emui_grid_update_geometry(EMTILE *t)
{
	struct grid *d = t->priv_data;
	EMTILE *ch;
	int x_offset = 0;
	int y_offset = 0;

	ch = t->ch_first;
	while (ch) {
		ch->properties |= P_GEOM_FORCED;
		// does a child fit y-wise?
		if (((d->rows <= 0) && (y_offset + d->row_height <= t->i.h))
		|| ((d->rows > 0) && (y_offset < d->rows * d->row_height))) {
			// does a child fit x-wise?
			if (((d->cols <= 0) && (x_offset + d->col_width <= t->i.w))
			|| ((d->cols > 0) && (x_offset < d->cols * (d->col_width)))) {
				ch->e.x = ch->parent->i.x + x_offset;
				ch->e.y = ch->parent->i.y + y_offset;
				ch->e.w = d->col_width;
				ch->e.h = d->row_height;
				ch->properties &= ~P_HIDDEN;
				x_offset += d->col_width + d->col_spacing;
				ch = ch->ch_next;
			// doesn't fit x-wise, skip to next row and repeat fitting
			} else {
				x_offset = 0;
				y_offset += d->row_height;
			}
		// doesn't fit y-wise, nothing to do but hide it
		} else {
			ch->properties |= P_HIDDEN;
			ch = ch->ch_next;
		}
	}
}

// -----------------------------------------------------------------------
void emui_grid_destroy_priv_data(EMTILE *t)
{
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emtile_drv emui_grid_drv = {
	.draw = NULL,
	.update_children_geometry = emui_grid_update_geometry,
	.event_handler = NULL,
	.focus_handler = NULL,
	.destroy_priv_data = emui_grid_destroy_priv_data,
};

// -----------------------------------------------------------------------
EMTILE * emui_grid(EMTILE *parent, int cols, int rows, int col_width, int row_height, int col_spacing)
{
	EMTILE *t;

	t = emtile(parent, &emui_grid_drv, 0, 0, parent->i.w, parent->i.h, 0, 0, 0, 0, "Grid", P_CONTAINER | P_MAXIMIZED);

	t->priv_data = calloc(1, sizeof(struct grid));

	struct grid *d = t->priv_data;

	d->cols = cols;
	d->rows = rows;
	d->col_width = col_width;
	d->row_height = row_height;
	d->col_spacing = col_spacing;

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent

