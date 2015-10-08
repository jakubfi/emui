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

#include <ncurses.h>

#include "style.h"

static int emui_style[EMUI_MAX_STYLE];

// -----------------------------------------------------------------------
static struct emui_style _emui_scheme_default[] = {
	{ S_DEFAULT,			COLOR_BLACK,	COLOR_WHITE,	A_NORMAL },
	{ S_DEBUG,				COLOR_RED,		COLOR_YELLOW,	A_BOLD },

	{ S_FRAME,				COLOR_BLACK,	COLOR_WHITE,	A_NORMAL },
	{ S_FRAME_FOCUSED,		COLOR_BLACK,	COLOR_WHITE,	A_BOLD },

	{ S_TITLE,				COLOR_BLACK,	COLOR_WHITE,	A_NORMAL },
	{ S_TITLE_FOCUSED,		COLOR_BLACK,	COLOR_WHITE,	A_BOLD|A_REVERSE },

	{ S_TAB,				COLOR_BLACK,	COLOR_WHITE,	A_BOLD },
	{ S_TAB_FOCUSED,		COLOR_BLACK,	COLOR_WHITE,	A_BOLD|A_REVERSE },

	{ S_TEXT,				COLOR_BLACK,	COLOR_WHITE,	A_NORMAL },
	{ S_TEXT_BOLD,			COLOR_BLACK,	COLOR_WHITE,	A_BOLD },
	{ S_TEXT_INVERSED,		COLOR_BLACK,	COLOR_WHITE,	A_NORMAL|A_REVERSE },
	{ S_TEXT_INVERSED_BOLD,	COLOR_BLACK,	COLOR_WHITE,	A_BOLD|A_REVERSE },
	{ S_TEXT_UNDERLINED,	COLOR_BLACK,	COLOR_WHITE,	A_NORMAL|A_UNDERLINE },
	{ S_TEXT_FOCUSED,		COLOR_CYAN,		COLOR_BLACK,	A_BOLD },
	{ S_TEXT_EDITED,		COLOR_CYAN,		COLOR_BLACK,	A_NORMAL },
	{ S_TEXT_CHANGED,		COLOR_RED,		COLOR_WHITE,	A_BOLD },

	{ -1, 0, 0, 0 }
};

// -----------------------------------------------------------------------
void emui_style_set(unsigned id, int bg, int fg, int attr)
{
	init_pair(id+1, fg, bg);
	emui_style[id] = COLOR_PAIR(id+1) | attr;
}

// -----------------------------------------------------------------------
int emui_style_add(unsigned id, int bg, int fg, int attr)
{
	if (id >= EMUI_FIRST_UI_STYLE) return 1;
	emui_style_set(id, bg, fg, attr);
	return 0;
}

// -----------------------------------------------------------------------
void emui_scheme_set(struct emui_style *s)
{
	while (s->id >= 0) {
		emui_style_set(s->id, s->bg, s->fg, s->attr);
		s++;
	}
}

// -----------------------------------------------------------------------
void emui_scheme_default()
{
	emui_scheme_set(_emui_scheme_default);
}

// -----------------------------------------------------------------------
int emui_style_get(unsigned id)
{
	return emui_style[id];
}

// vim: tabstop=4 shiftwidth=4 autoindent
