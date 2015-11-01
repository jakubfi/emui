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

struct emui_tile * emui_screen();
struct emui_tile * emui_dummy_cont(struct emui_tile *parent, int x, int y, int w, int h);
struct emui_tile * emui_splitter(struct emui_tile *parent, int edge, int min1, int max1, int min2);
struct emui_tile * emui_tabs(struct emui_tile *parent);
struct emui_tile * emui_window(struct emui_tile *parent, int x, int y, int w, int h, char *name, int properties);
struct emui_tile * emui_justifier(struct emui_tile *parent);

// widgets

struct emui_tile * emui_framecounter(struct emui_tile *parent, int x, int y, int style);
void emui_framecounter_set_style(struct emui_tile *t, int style);
struct emui_tile * emui_fpscounter(struct emui_tile *parent, int x, int y, int style);
void emui_fpscounter_set_style(struct emui_tile *t, int style);

struct emui_tile * emui_lineedit(struct emui_tile *parent, int id, int x, int y, int w, int maxlen, int type, int mode);
int emui_lineedit_set_text(struct emui_tile *t, char *text);
char * emui_lineedit_get_text(struct emui_tile *t);
void emui_lineedit_set_int(struct emui_tile *t, int v);
int emui_lineedit_get_int(struct emui_tile *t);
void emui_lineedit_set_mode(struct emui_tile *t, int mode);
void emui_lineedit_set_pos(struct emui_tile *t, unsigned pos);
void emui_lineedit_set_mode(struct emui_tile *t, int mode);
void emui_lineedit_edit(struct emui_tile *t, int state);
void emui_lineedit_invalid(struct emui_tile *t);

struct emui_tile * emui_label(struct emui_tile *parent, int x, int y, int w, int align, int style, char *txt);
int emui_label_set_text(struct emui_tile *t, char *txt);
void emui_label_set_style(struct emui_tile *t, int style);

struct emui_tile * emui_textview(struct emui_tile *parent, int x, int y, int w, int h);
int emui_textview_append(struct emui_tile *t, int style, char *str);
void emui_textview_clear(struct emui_tile *t);


#endif

// vim: tabstop=4 shiftwidth=4 autoindent
