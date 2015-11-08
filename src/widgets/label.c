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

struct label {
	char *txt;
	int align;
};

// -----------------------------------------------------------------------
void emui_label_draw(struct emui_tile *t)
{
	struct label *d = t->priv_data;

	int loc_offset = 0;
	int txt_offset = 0;

	if (d->align == AL_RIGHT) {
		loc_offset = t->i.w - strlen(d->txt);
	} else if (d->align == AL_CENTER) {
		loc_offset = (t->i.w - strlen(d->txt)) / 2;
	} else { // AL_LEFT and unknown aligns
		// nothing
	}

	if (loc_offset < 0) {
		txt_offset = -loc_offset;
		loc_offset = 0;
	}

	emuifillbg(t, t->style);
	emuixyprt(t, 0 + loc_offset, 0, t->style, d->txt + txt_offset);
}

// -----------------------------------------------------------------------
void emui_label_destroy_priv_data(struct emui_tile *t)
{
	struct label *d = t->priv_data;
	free(d->txt);
	free(d);
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_label_drv = {
	.draw = emui_label_draw,
	.update_children_geometry = NULL,
	.event_handler = NULL,
	.destroy_priv_data = emui_label_destroy_priv_data,
};

// -----------------------------------------------------------------------
int emui_label_set_text(struct emui_tile *t, char *txt)
{
	struct label *d = t->priv_data;

	free(d->txt);
	d->txt = strdup(txt);

	return 0;
}

// -----------------------------------------------------------------------
struct emui_tile * emui_label(struct emui_tile *parent, int x, int y, int w, int align, int style, char *txt)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, -1, &emui_label_drv, F_WIDGET, x, y, w, 1, 0, 0, 0, 0, NULL, P_NONE);

	t->style = style;
	t->priv_data = calloc(1, sizeof(struct label));
	struct label *d = t->priv_data;
	d->align = align;
	emui_label_set_text(t, txt);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
