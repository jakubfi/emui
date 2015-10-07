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

struct label {
	char *txt;
	int style;
	int align;
};

// -----------------------------------------------------------------------
void emui_label_draw(struct emui_tile *t)
{
	struct label *d = t->priv_data;

	int loc_offset = 0;
	int txt_offset = 0;

	emuifillbg(t, d->style);

	if (d->align == AL_RIGHT) {
		loc_offset = t->w - strlen(d->txt);
	} else if (d->align == AL_CENTER) {
		loc_offset = (t->w - strlen(d->txt)) / 2;
	} else { // AL_LEFT and unknown aligns
		// nothing
	}

	if (loc_offset < 0) {
		txt_offset = -loc_offset;
		loc_offset = 0;
	}

	emuixyprt(t, 0 + loc_offset, 0, d->style, d->txt + txt_offset);
}

// -----------------------------------------------------------------------
void emui_label_debug(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
int emui_label_update_geometry(struct emui_tile *t)
{
	return 0;
}

// -----------------------------------------------------------------------
int emui_label_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_label_destroy_priv_data(struct emui_tile *t)
{
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_label_drv = {
	.draw = emui_label_draw,
	.debug = emui_label_debug,
	.update_geometry = emui_label_update_geometry,
	.event_handler = emui_label_event_handler,
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
void emui_label_set_style(struct emui_tile *t, int style)
{
	struct label *d = t->priv_data;
	d->style = style;
}

// -----------------------------------------------------------------------
struct emui_tile * emui_label_new(struct emui_tile *parent, int x, int y, int w, int align, int style, char *txt)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, &emui_label_drv, T_WIDGET, x, y, w, 1, 0, 0, 0, 0, NULL, P_NONE);

	t->priv_data = calloc(1, sizeof(struct label));
	struct label *d = t->priv_data;
	d->style = style;
	d->align = align;
	emui_label_set_text(t, txt);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
