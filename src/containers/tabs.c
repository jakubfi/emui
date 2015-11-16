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
#include "focus.h"
#include "style.h"
#include "print.h"

struct tabs {
	EMTILE *last_selected;
};

// -----------------------------------------------------------------------
void emui_tabs_draw(EMTILE *t)
{
	EMTILE *ch = t->ch_first;
	emuixy(t, 0, 0);
	while (ch) {
		int style = S_TAB_NN;
		if (emui_has_focus(ch)) style = S_TAB_FN;
		// TODO: check if we have enough sapce
		emuiprt(t, style, " %s ", ch->name);
		ch = ch->ch_next;
	}
}

// -----------------------------------------------------------------------
int emui_tabs_update_geometry(EMTILE *t)
{
	struct tabs *d = t->priv_data;
	int tab_selected = 0;

	EMTILE *ch = t->ch_first;
	while (ch) {
		ch->properties |= P_GEOM_FORCED;
		ch->e = t->i;
		if (emui_has_focus(ch)) {
			ch->properties &= ~P_HIDDEN;
			d->last_selected = ch;
			tab_selected = 1;
		} else {
			ch->properties |= P_HIDDEN;
		}
		ch = ch->ch_next;
	}

	// always leave the last selected tab unhidden
	if (!tab_selected && d && d->last_selected) {
		d->last_selected->properties &= ~P_HIDDEN;
	}
	return 0;
}

// -----------------------------------------------------------------------
void emui_tabs_destroy_priv_data(EMTILE *t)
{
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emtile_drv emui_tabs_drv = {
	.draw = emui_tabs_draw,
	.update_children_geometry = emui_tabs_update_geometry,
	.event_handler = NULL,
	.focus_handler = NULL,
	.destroy_priv_data = emui_tabs_destroy_priv_data,
};

// -----------------------------------------------------------------------
EMTILE * emui_tabs(EMTILE *parent)
{
	EMTILE *t = emtile(parent, &emui_tabs_drv, 0, 0, parent->i.w, parent->i.h, 1, 0, 0, 0, "Tabs", P_CONTAINER | P_MAXIMIZED | P_FOCUS_GROUP);

	t->priv_data = calloc(1, sizeof(struct tabs));

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
