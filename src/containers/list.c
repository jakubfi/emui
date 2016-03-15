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

#include "dbg.h"
#include "tile.h"
#include "event.h"

struct list {
	int start_offset;
};

// -----------------------------------------------------------------------
void emui_list_update_geometry(EMTILE *t)
{
	EMTILE *ch = t->ch_first;
	if (!ch) return;

	struct list *d = t->priv_data;
	int y_offset = d->start_offset;

	while (ch) {
		ch->properties |= P_GEOM_FORCED;
		ch->e.x = t->i.x;
		ch->e.y = t->i.y + y_offset;
		ch->e.w = t->i.w;
		ch->e.h = ch->r.h;
		y_offset += ch->r.h;
		if ((ch->e.y < t->i.y) || (ch->e.y >= t->i.y + t->i.h)) {
			EDBG(ch, 4, "list hiding tile");
			ch->properties |= P_HIDDEN;
		} else {
			EDBG(ch, 4, "list unhiding tile");
			ch->properties &= ~P_HIDDEN;
		}
		ch = ch->ch_next;
	}
}

// -----------------------------------------------------------------------
void emui_list_destroy_priv_data(EMTILE *t)
{
	free(t->priv_data);
}

// -----------------------------------------------------------------------
void emui_list_scroll(EMTILE *t, EMTILE *f)
{
	struct list *d = t->priv_data;

	// if f is below t view area
	if (f->e.y >= t->i.y + t->i.h) {
		d->start_offset += t->i.y + t->i.h - f->e.y - 1;
	// if f is above t view area
	} else if (f->e.y < t->i.y) {
		d->start_offset += t->i.y - f->e.y;
	}
	EDBG(f, 4, "list scroll wants to show tile, offset is now: %i", d->start_offset);
}

// -----------------------------------------------------------------------
struct emtile_drv emui_list_drv = {
	.draw = NULL,
	.update_children_geometry = emui_list_update_geometry,
	.event_handler = NULL,
	.focus_handler = NULL,
	.destroy_priv_data = emui_list_destroy_priv_data,
	.scroll_handler = emui_list_scroll,
};

// -----------------------------------------------------------------------
EMTILE * emui_list(EMTILE *parent)
{
	EMTILE *t;

	t = emtile(parent, &emui_list_drv, 0, 0, parent->i.w, parent->i.h, 0, 0, 0, 0, "List", P_CONTAINER | P_MAXIMIZE | P_FOCUS_GROUP);

	t->priv_data = calloc(1, sizeof(struct list));
	struct list *d = t->priv_data;
	d->start_offset = 0;

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
