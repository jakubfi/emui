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

#include "widgets/lineedit.h"
#include "widgets/textview.h"
#include "widgets/label.h"
#include "widgets/misc.h"

// containers

struct emui_tile * emui_screen();
struct emui_tile * emui_dummy_cont(struct emui_tile *parent, int x, int y, int w, int h);
struct emui_tile * emui_splitter(struct emui_tile *parent, int edge, int min1, int max1, int min2);
struct emui_tile * emui_tabs(struct emui_tile *parent);
struct emui_tile * emui_frame(struct emui_tile *parent, int x, int y, int w, int h, char *name, int properties);
struct emui_tile * emui_justifier(struct emui_tile *parent);
struct emui_tile * emui_grid(struct emui_tile *parent, int cols, int rows, int col_width, int row_height, int col_spacing);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
