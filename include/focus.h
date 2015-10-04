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

#ifndef EMUI_FOCUS_H
#define EMUI_FOCUS_H

#include "tile.h"

enum focus_change_direction {
	FC_BEG,
	FC_END,
	FC_NEXT,
	FC_PREV,
	FC_LEFT,
	FC_RIGHT,
	FC_UP,
	FC_DOWN
};

void emui_focus(struct emui_tile *t);
int emui_has_focus(struct emui_tile *t);
int emui_is_focused(struct emui_tile *t);
struct emui_tile * emui_focus_get();
int emui_focus_neighbour(struct emui_tile *t, int dir);
int emui_focus_group_add(struct emui_tile *parent, struct emui_tile *t);
void emui_focus_group_unlink(struct emui_tile *t);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
