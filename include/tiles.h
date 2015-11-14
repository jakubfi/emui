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

#ifndef EMUI_TILES_H
#define EMUI_TILES_H

#include "tile.h"
#include "widgets/lineedit.h"
#include "widgets/textview.h"
#include "widgets/label.h"
#include "widgets/misc.h"

// containers

EMTILE * emui_screen();
EMTILE * emui_dummy_cont(EMTILE *parent, int x, int y, int w, int h);
EMTILE * emui_splitter(EMTILE *parent, int edge, int min1, int max1, int min2);
EMTILE * emui_tabs(EMTILE *parent);
EMTILE * emui_frame(EMTILE *parent, int x, int y, int w, int h, char *name, int properties);
EMTILE * emui_justifier(EMTILE *parent);
EMTILE * emui_grid(EMTILE *parent, int cols, int rows, int col_width, int row_height, int col_spacing);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
