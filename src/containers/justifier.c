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
int emui_justifier_update_geometry(EMTILE *t)
{
	int ch_width = -1;
	int ch_count = 0;
	EMTILE *ch;

	// calculate total children width
	ch = t->ch_first;
	while (ch) {
		ch->properties &= ~P_HIDDEN;
		ch_width += ch->r.w + 1;
		ch_count++;
		ch->properties |= P_GEOM_FORCED;
		ch = ch->ch_next;
	}

	// hide children which won't fit (start from the end of ch list)
	ch = t->ch_last;
	while (ch) {
		if (ch_width > t->i.w) {
			ch_width -= ch->i.w + 1;
			ch->properties |= P_HIDDEN;
		} else {
			ch->properties &= ~P_HIDDEN;
		}
		ch = ch->ch_prev;
	}

	// justify children
	ch = t->ch_first;
	float per_ch = (float) (t->i.w - ch_width) / (float) (ch_count-1);
	float offset = 0;
	while (ch) {
		ch->e.x = ch->parent->i.x + offset;
		ch->e.y = ch->parent->i.y;
		ch->e.w = ch->r.w;
		ch->e.h = ch->parent->i.h;
		offset += ch->r.w + 1 + per_ch;
		ch = ch->ch_next;
	}

	return 0;
}

// -----------------------------------------------------------------------
struct emtile_drv emui_justifier_drv = {
	.draw = NULL,
	.update_children_geometry = emui_justifier_update_geometry,
	.event_handler = NULL,
	.focus_handler = NULL,
	.destroy_priv_data = NULL,
};

// -----------------------------------------------------------------------
EMTILE * emui_justifier(EMTILE *parent)
{
	EMTILE *t;

	t = emtile(parent, -1, &emui_justifier_drv, 0, 0, parent->i.w, parent->i.h, 0, 0, 0, 0, "Justify", P_CONTAINER | P_MAXIMIZED);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
