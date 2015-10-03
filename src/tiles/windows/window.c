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

// -----------------------------------------------------------------------
void emui_window_draw(struct emui_tile *t)
{
	int title_style = S_WINDOW_TITLE;
	int frame_style = S_WINDOW_FRAME;

	if (emui_tile_has_focus(t)) {
		title_style = S_WINDOW_TITLE_F;
		frame_style = S_WINDOW_FRAME_F;
	}

	if (!(t->properties & P_BORDERLESS)) {
		emuidbox(t, frame_style);
		emuixydprt(t, 2, 0, title_style, "[ %s ]", t->name);
	}
}

// -----------------------------------------------------------------------
void emui_window_debug(struct emui_tile *t)
{
	mvwprintw(t->ncwin, 0, 0, "Sample content");
}

// -----------------------------------------------------------------------
int emui_window_update_geometry(struct emui_tile *t)
{
	return 0;
}

// -----------------------------------------------------------------------
int emui_window_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_window_destroy_priv_data(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_window_drv = {
	.draw = emui_window_draw,
	.debug = emui_window_debug,
	.update_geometry = emui_window_update_geometry,
	.event_handler = emui_window_event_handler,
	.destroy_priv_data = emui_window_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_window_new(struct emui_tile *parent, int x, int y, int w, int h, char *name, int properties)
{
	struct emui_tile *t;
	int mt, mb, ml, mr;

	if (properties & P_BORDERLESS) {
		mt = 0; mb = 0;
		ml = 0; mr = 0;
	} else {
		mt = 1; mb = 1;
		ml = 1; mr = 1;
	}

	t = emui_tile_create(parent, &emui_window_drv, T_WINDOW, x, y, w, h, mt, mb, ml, mr, name, properties | P_FOCUS_GROUP);

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
