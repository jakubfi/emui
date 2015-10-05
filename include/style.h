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

#ifndef STYLE_H
#define STYLE_H

#define EMUI_MIN_STYLE 0
#define EMUI_MAX_STYLE (64-2)
#define EMUI_FIRST_UI_STYLE (EMUI_MAX_STYLE-32)

enum emui_styles {
	S_DEFAULT = EMUI_FIRST_UI_STYLE,
	S_DEBUG,
	S_FRAME,
	S_FRAME_FOCUSED,
	S_TITLE,
	S_TITLE_FOCUSED,
	S_TAB,
	S_TAB_FOCUSED,
	S_TEXT,
	S_TEXT_BOLD,
	S_TEXT_FOCUSED,
	S_TEXT_EDITED,
	S_TEXT_CHANGED,
};

struct emui_style {
	int id;
	int bg;
	int fg;
	int attr;
};

void emui_style_set(unsigned id, int bg, int fg, int attr);
int emui_style_add(unsigned id, int bg, int fg, int attr);
int emui_style_get(unsigned id);
void emui_scheme_set(struct emui_style *s);
void emui_scheme_default();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
