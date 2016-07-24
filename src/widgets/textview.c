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
#include "tiles.h"
#include "event.h"
#include "style.h"
#include "print.h"
#include "text.h"

struct textview {
	EMTEXT *txt;
};

// -----------------------------------------------------------------------
void emui_textview_draw(EMTILE *t)
{
	struct textview *d = t->priv_data;

	emuixy(t, 0, 0);
	emtext_print(d->txt, t);
}

// -----------------------------------------------------------------------
int emui_textview_event_handler(EMTILE *t, struct emui_event *ev)
{
	struct textview *d = t->priv_data;

	if (ev->type == EV_KEY) {
		switch (ev->sender) {
			case KEY_UP:
				emtext_line_skip(d->txt, -1);
				return E_HANDLED;
			case KEY_DOWN:
				emtext_line_skip(d->txt, 1);
				return E_HANDLED;
			case KEY_HOME:
				emtext_line_first(d->txt);
				return E_HANDLED;
			case KEY_END:
				emtext_line_skipto(d->txt, -t->i.h);
				return E_HANDLED;
			case KEY_PPAGE:
				emtext_line_skip(d->txt, -t->i.h);
				return E_HANDLED;
			case KEY_NPAGE:
				emtext_line_skip(d->txt, t->i.h);
				return E_HANDLED;
			default:
				break;
		}
	}

	return E_UNHANDLED;
}

// -----------------------------------------------------------------------
void emui_textview_destroy_priv_data(EMTILE *t)
{
	struct textview *d = t->priv_data;
	emtext_delete(d->txt);
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emtile_drv emui_textview_drv = {
	.draw = emui_textview_draw,
	.update_children_geometry = NULL,
	.event_handler = emui_textview_event_handler,
	.destroy_priv_data = emui_textview_destroy_priv_data,
};

// -----------------------------------------------------------------------
EMTILE * emui_textview(EMTILE *parent, int x, int y, int w, int h)
{
	EMTILE *t;

	t = emtile(parent, &emui_textview_drv, x, y, w, h, 0, 0, 0, 0, "TextView", P_INTERACTIVE);

	t->priv_data = calloc(1, sizeof(struct textview));

	struct textview *d = t->priv_data;
	d->txt = emtext();

	return t;
}

// -----------------------------------------------------------------------
EMTEXT * emui_textview_get_emtext(EMTILE *t)
{
	struct textview *d = t->priv_data;
	return d->txt;
}

// vim: tabstop=4 shiftwidth=4 autoindent
