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

static int emui_color[EMUI_COLORS][EMUI_COLORS];
static int emui_style[EMUI_STYLES];

// -----------------------------------------------------------------------
struct emui_style_def _emui_scheme_inv[] = {
	{ S_DEFAULT,		COLOR_WHITE,	COLOR_BLACK,	A_NORMAL },
	{ S_FRAME_NN,		COLOR_WHITE,	COLOR_BLACK,	A_NORMAL },
	{ S_FRAME_FN,		COLOR_WHITE,	COLOR_BLACK,	A_BOLD },
	{ S_TITLE_NN,		COLOR_WHITE,	COLOR_BLACK,	A_NORMAL },
	{ S_TITLE_FN,		COLOR_WHITE,	COLOR_BLACK,	A_BOLD|A_REVERSE },
	{ S_TAB_NN,			COLOR_WHITE,	COLOR_BLACK,	A_BOLD },
	{ S_TAB_FN,			COLOR_WHITE,	COLOR_BLACK,	A_BOLD|A_REVERSE },
	{ S_EDIT_NN,		COLOR_WHITE,	COLOR_BLACK,	A_BOLD },
	{ S_TEXT_NN,		COLOR_WHITE,	COLOR_BLACK,	A_NORMAL },
	{ S_TEXT_NI,		COLOR_WHITE,	COLOR_RED,		A_BOLD },
	{ S_TEXT_FN,		COLOR_CYAN,		COLOR_BLUE,		A_BOLD },
	{ S_TEXT_FI,		COLOR_CYAN,		COLOR_RED,		A_BOLD },
	{ S_TEXT_EN,		COLOR_CYAN,		COLOR_BLUE,		A_NORMAL },
	{ S_TEXT_EI,		COLOR_CYAN,		COLOR_RED,		A_NORMAL },
	{ -1, 0, 0, 0 }
};

// -----------------------------------------------------------------------
struct emui_style_def _emui_scheme_blue[] = {
	{ S_DEFAULT,		COLOR_BLUE,		COLOR_WHITE,	A_NORMAL },
	{ S_FRAME_NN,		COLOR_BLUE,		COLOR_WHITE,	A_NORMAL },
	{ S_FRAME_FN,		COLOR_BLUE,		COLOR_WHITE,	A_BOLD },
	{ S_TITLE_NN,		COLOR_BLUE,		COLOR_WHITE,	A_NORMAL },
	{ S_TITLE_FN,		COLOR_BLACK,	COLOR_WHITE,	A_BOLD|A_REVERSE },
	{ S_TAB_NN,			COLOR_BLUE,		COLOR_WHITE,	A_BOLD },
	{ S_TAB_FN,			COLOR_BLACK,	COLOR_WHITE,	A_BOLD|A_REVERSE },
	{ S_EDIT_NN,		COLOR_BLUE,		COLOR_WHITE,	A_BOLD },
	{ S_TEXT_NN,		COLOR_BLUE,		COLOR_WHITE,	A_NORMAL },
	{ S_TEXT_NI,		COLOR_BLUE,		COLOR_RED,		A_BOLD },
	{ S_TEXT_FN,		COLOR_CYAN,		COLOR_BLACK,	A_BOLD },
	{ S_TEXT_FI,		COLOR_CYAN,		COLOR_RED,		A_BOLD },
	{ S_TEXT_EN,		COLOR_CYAN,		COLOR_BLACK,	A_NORMAL },
	{ S_TEXT_EI,		COLOR_CYAN,		COLOR_RED,		A_NORMAL },
	{ -1, 0, 0, 0 }
};

// -----------------------------------------------------------------------
struct emui_style_def _emui_scheme_default[] = {
	{ S_DEFAULT,		COLOR_BLACK,	COLOR_WHITE,	A_NORMAL },
	{ S_FRAME_NN,		COLOR_BLACK,	COLOR_WHITE,	A_NORMAL },
	{ S_FRAME_FN,		COLOR_BLACK,	COLOR_WHITE,	A_BOLD },
	{ S_TITLE_NN,		COLOR_BLACK,	COLOR_WHITE,	A_NORMAL },
	{ S_TITLE_FN,		COLOR_BLACK,	COLOR_CYAN,	A_BOLD|A_REVERSE },
	{ S_TAB_NN,			COLOR_BLACK,	COLOR_WHITE,	A_BOLD },
	{ S_TAB_FN,			COLOR_BLACK,	COLOR_WHITE,	A_BOLD|A_REVERSE },
	{ S_EDIT_NN,		COLOR_BLACK,	COLOR_WHITE,	A_BOLD },
	{ S_TEXT_NN,		COLOR_BLACK,	COLOR_WHITE,	A_NORMAL },
	{ S_TEXT_NI,		COLOR_BLACK,	COLOR_RED,		A_BOLD },
	{ S_TEXT_FN,		COLOR_CYAN,		COLOR_BLACK,	A_BOLD },
	{ S_TEXT_FI,		COLOR_CYAN,		COLOR_RED,		A_BOLD },
	{ S_TEXT_EN,		COLOR_CYAN,		COLOR_BLACK,	A_NORMAL },
	{ S_TEXT_EI,		COLOR_CYAN,		COLOR_RED,		A_NORMAL },
	{ -1, 0, 0, 0 }
};

// -----------------------------------------------------------------------
void emui_style_init(struct emui_style_def *scheme)
{
	for (int bg=0 ; bg<EMUI_COLORS ; bg++) {
		for (int fg=0 ; fg<EMUI_COLORS ; fg++) {
			int pair = bg*EMUI_COLORS + fg;
			if (pair == 0) {
				emui_color[bg][fg] = 0;
			} else {
				init_pair(pair, fg, bg);
				emui_color[bg][fg] = COLOR_PAIR(pair);
			}
		}
	}

	emui_scheme_set(_emui_scheme_default);
	if (scheme) {
		emui_scheme_set(scheme);
	}
}

// -----------------------------------------------------------------------
int emui_style_set(unsigned id, int bg, int fg, int attr)
{
	if (id >= EMUI_STYLES) {
		return 1;
	}

	emui_style[id] = emui_color[bg][fg] | (attr);

	return 0;
}

// -----------------------------------------------------------------------
int emui_scheme_set(struct emui_style_def *s)
{
	while (s->id >= 0) {
		if (emui_style_set(s->id, s->bg, s->fg, s->attr)) {
			return 1;
		}
		s++;
	}
	return 0;
}

// -----------------------------------------------------------------------
int emui_style_get(unsigned id)
{
	if (id < EMUI_STYLES) {
		return emui_style[id];
	} else {
		return 0;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
