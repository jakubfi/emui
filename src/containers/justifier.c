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
#include <math.h>

#include "tile.h"
#include "event.h"

// -----------------------------------------------------------------------
void emui_justifier_draw(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
int emui_justifier_update_geometry(struct emui_tile *t)
{
	int ch_width = -1;
	int ch_count = 0;
	struct emui_tile *ch;

	// calculate total children width
	ch = t->ch_first;
	while (ch) {
		emui_tile_unhide(ch);
		ch_width += ch->rw + 1;
		ch_count++;
		ch->properties |= P_GEOM_FORCED;
		ch = ch->next;
	}

	// hide children which won't fit (start from the end of ch list)
	ch = t->ch_last;
	while (ch) {
		if (ch_width > t->w) {
			ch_width -= ch->w + 1;
			emui_tile_hide(ch);
		} else {
			emui_tile_unhide(ch);
		}
		ch = ch->prev;
	}

	// justify children
	ch = t->ch_first;
	float per_ch = (float) (t->w - ch_width) / (float) (ch_count-1);
	float offset = 0;
	while (ch) {
		ch->dx = ch->parent->x + offset;
		ch->dy = ch->parent->y;
		ch->dw = ch->rw;
		ch->dh = ch->parent->h;
		offset += ch->rw + 1 + per_ch;
		ch = ch->next;
	}

	return 0;
}

// -----------------------------------------------------------------------
int emui_justifier_event_handler(struct emui_tile *t, struct emui_event *ev)
{   
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_justifier_destroy_priv_data(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_justifier_drv = {
	.draw = emui_justifier_draw,
	.update_geometry = emui_justifier_update_geometry,
	.event_handler = emui_justifier_event_handler,
	.destroy_priv_data = emui_justifier_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_justifier(struct emui_tile *parent)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, -1, &emui_justifier_drv, F_CONTAINER, 0, 0, parent->w, parent->h, 0, 0, 0, 0, "Justify", P_MAXIMIZED);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
