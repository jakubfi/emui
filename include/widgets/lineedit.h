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

#ifndef EMUI_LINEEDIT_H
#define EMUI_LINEEDIT_H

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

#endif
