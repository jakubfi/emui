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

#include "emui.h"
#include "tile.h"
#include "event.h"
#include "style.h"
#include "print.h"

// -----------------------------------------------------------------------
void emui_framecounter_draw(EMTILE *t)
{
	emuixyprt(t, 0, 0, t->style, "%lu", emui_get_frame());
}

// -----------------------------------------------------------------------
struct emtile_drv emui_framecounter_drv = {
	.draw = emui_framecounter_draw,
	.update_children_geometry = NULL,
	.event_handler = NULL,
	.focus_handler = NULL,
	.destroy_priv_data = NULL,
};

// -----------------------------------------------------------------------
EMTILE * emui_framecounter(EMTILE *parent, int x, int y, int style)
{
	EMTILE *t;

	t = emtile(parent, &emui_framecounter_drv, x, y, 20, 1, 0, 0, 0, 0, NULL, P_NONE);

	t->style = style;

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
