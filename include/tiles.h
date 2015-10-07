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

// containers

struct emui_tile * emui_screen_new();
struct emui_tile * emui_dummy_cont_new(struct emui_tile *parent, int x, int y, int w, int h);
struct emui_tile * emui_splitter_new(struct emui_tile *parent, int edge, int min1, int max1, int min2);
struct emui_tile * emui_tabs_new(struct emui_tile *parent);
struct emui_tile * emui_window_new(struct emui_tile *parent, int x, int y, int w, int h, char *name, int properties);

// widgets

struct emui_tile * emui_framecounter_new(struct emui_tile *parent, int x, int y, int style);
struct emui_tile * emui_fpscounter_new(struct emui_tile *parent, int x, int y, int style);

struct emui_tile * emui_lineedit_new(struct emui_tile *parent, int x, int y, int w, int maxlen, int type, int properties);
int emui_lineedit_set_text(struct emui_tile *t, char *text);

struct emui_tile * emui_label_new(struct emui_tile *parent, int x, int y, int w, int align, int style, char *txt);
int emui_label_set_text(struct emui_tile *t, char *txt);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
