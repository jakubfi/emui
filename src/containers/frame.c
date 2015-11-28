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

// -----------------------------------------------------------------------
static char * _make_title(char *title, int width)
{
	// case: +-----+
	if (width <= 5) {
		return NULL;
	}

	int twidth = strlen(title) + 4;
	char *otitle = malloc(twidth+1);

	// case: +[ Tit~ ]+
	if (width < twidth) {
		sprintf(otitle, "[ %s", title);
		sprintf(otitle+width-3, "~ ]");
		return otitle;
	}

	// case: +[ Title ]---+
	sprintf(otitle, "[ %s ]", title);

	return otitle;
}

// -----------------------------------------------------------------------
void emui_frame_draw(EMTILE *t)
{
	int title_style = S_TITLE_NN;
	int frame_style = S_FRAME_NN;

	if (emui_has_focus(t)) {
		title_style = S_TITLE_FN;
		frame_style = S_FRAME_FN;
	}

	emuibox(t, frame_style);
	char *title = _make_title(t->name, t->e.w - 2);
	if (title) {
		emuixyprt(t, 1, 0, title_style, "%s", title);
	}
	free(title);
}

// -----------------------------------------------------------------------
struct emtile_drv emui_frame_drv = {
	.draw = emui_frame_draw,
	.update_children_geometry = NULL,
	.event_handler = NULL,
	.focus_handler = NULL,
	.destroy_priv_data = NULL,
};

// -----------------------------------------------------------------------
EMTILE * emui_frame(EMTILE *parent, int x, int y, int w, int h, char *name, int properties)
{
	EMTILE *t;

	t = emtile(parent, &emui_frame_drv, x, y, w, h, 1, 1, 1, 1, name, P_CONTAINER | P_FOCUS_GROUP);

	emtile_set_properties(t, properties);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
