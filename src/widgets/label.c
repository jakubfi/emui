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
#include "text.h"
#include "focus.h"

struct label {
	EMTEXT *txt;
};

// -----------------------------------------------------------------------
void emui_label_draw(EMTILE *t)
{
	struct label *d = t->priv_data;

	emtext_print(d->txt, t);
}

// -----------------------------------------------------------------------
void emui_label_destroy_priv_data(EMTILE *t)
{
	struct label *d = t->priv_data;
	emtext_delete(d->txt);
	free(d);
}

// -----------------------------------------------------------------------
struct emtile_drv emui_label_drv = {
	.draw = emui_label_draw,
	.update_children_geometry = NULL,
	.event_handler = NULL,
	.destroy_priv_data = emui_label_destroy_priv_data,
};

// -----------------------------------------------------------------------
EMTEXT * emui_label_get_emtext(EMTILE *t)
{
	struct label *d = t->priv_data;
	return d->txt;
}

// -----------------------------------------------------------------------
EMTILE * emui_label(EMTILE *parent, int x, int y, int w, int style, char *str)
{
	EMTILE *t;

	t = emtile(parent, &emui_label_drv, x, y, w, 1, 0, 0, 0, 0, "Label", P_NONE);

	t->priv_data = calloc(1, sizeof(struct label));
	struct label *d = t->priv_data;
	d->txt = emtext();
	emtext_append_str(d->txt, style, str);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
