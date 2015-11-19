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

#ifndef TEXT_H
#define TEXT_H

struct emui_tchunk {
	int style;
	char *txt;
	struct emui_tchunk *next, *prev;
	int lines;
	int lineno;
};

struct emui_text {
	struct emui_tchunk *first, *last, *current;
};

typedef struct emui_text EMTEXT;

EMTEXT * emtext();
void emtext_delete_forward(EMTEXT *txt, struct emui_tchunk *c);
void emtext_delete_backward(EMTEXT *txt, struct emui_tchunk *c);
void emtext_clear(EMTEXT *txt);
void emtext_delete(EMTEXT *txt);
void emtext_append_chunk(EMTEXT *txt, struct emui_tchunk *chunk);
int emtext_append_str(EMTEXT *txt, int style, char *format, ...);
void emtext_line_skip(EMTEXT *txt, int lines);
void emtext_line_skipto(EMTEXT *txt, int line);
int emtext_get_current_line(EMTEXT *txt);
int emtext_get_lines(EMTEXT *txt);
void emtext_line_first(EMTEXT *txt);
void emtext_line_last(EMTEXT *txt);
int emtext_print_from(EMTEXT *txt, EMTILE *t, struct emui_tchunk *chunk);
int emtext_print(EMTEXT *txt, EMTILE *t);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent

