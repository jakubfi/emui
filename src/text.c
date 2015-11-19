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

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "tile.h"
#include "style.h"
#include "print.h"
#include "text.h"

// -----------------------------------------------------------------------
EMTEXT * emtext()
{
	return calloc(1, sizeof(EMTEXT));
}

// -----------------------------------------------------------------------
static void _emtext_chunk_free(struct emui_tchunk *c)
{
	if (!c) return;
	free(c->txt);
	free(c);
}

// -----------------------------------------------------------------------
void emtext_delete_forward(EMTEXT *txt, struct emui_tchunk *c)
{
	if (!c) return;

	txt->last = c->prev;

	if (c == txt->first) {
		txt->first = NULL;
	}

	struct emui_tchunk *next;
	while (c) {
		if (c == txt->current) {
			txt->current = txt->last;
		}
		next = c->next;
		_emtext_chunk_free(c);
		c = next;
	}
}

// -----------------------------------------------------------------------
void emtext_delete_backward(EMTEXT *txt, struct emui_tchunk *c)
{
	txt->first = c->next;

	if (c == txt->last) {
		txt->last = NULL;
	}

	struct emui_tchunk *prev;
	while (c) {
		if (c == txt->current) {
			txt->current = txt->first;
		}
		prev = c->prev;
		_emtext_chunk_free(c);
		c = prev;
	}
}

// -----------------------------------------------------------------------
void emtext_clear(EMTEXT *txt)
{
	emtext_delete_forward(txt, txt->first);
}

// -----------------------------------------------------------------------
void emtext_delete(EMTEXT *txt)
{
	emtext_clear(txt);
	free(txt);
}

// -----------------------------------------------------------------------
void emtext_append_chunk(EMTEXT *txt, struct emui_tchunk *chunk)
{
	if (txt->last) {
		txt->last->next = chunk;
		chunk->lineno = txt->last->lineno + txt->last->lines;
	} else {
		chunk->lineno = 0;
	}

	chunk->prev = txt->last;
	chunk->next = NULL;
	txt->last = chunk;

	if (!txt->first) {
		txt->first = chunk;
		txt->current = chunk;
	}
}

// -----------------------------------------------------------------------
int emtext_append_str(EMTEXT *txt, int style, char *format, ...)
{
	struct emui_tchunk *chunk;
	char *nl;
	char *buf;

	va_list vl;
	va_start(vl, format);
	vasprintf(&buf, format, vl);
	va_end(vl);

	char *str = buf;
	nl = strchr(str, '\n');
	while (nl) {
		int size = nl-str+1;
		chunk = malloc(sizeof(struct emui_tchunk));
		chunk->style = style;
		chunk->lines = 1;
		chunk->txt = malloc(size+1);
		chunk->txt[size] = '\0';
		strncpy(chunk->txt, str, size);
		emtext_append_chunk(txt, chunk);
		str = nl+1;
		nl = strchr(str, '\n');
	}

	if (*str) {
		chunk = malloc(sizeof(struct emui_tchunk));
		chunk->style = style;
		chunk->lines = 0;
		chunk->txt = strdup(str);
		emtext_append_chunk(txt, chunk);
	}

	free(buf);
	return 0;
}

// -----------------------------------------------------------------------
void emtext_line_skip(EMTEXT *txt, int lines)
{
	while (lines != 0) {
		if (lines > 0) {
			if (txt->current->next) {
				lines -= txt->current->lines;
				txt->current = txt->current->next;
			} else {
				break;
			}
		} else {
			if (txt->current->prev) {
				lines += txt->current->lines;
				txt->current = txt->current->prev;
			} else {
				break;
			}
		}
	}
}

// -----------------------------------------------------------------------
void emtext_line_skipto(EMTEXT *txt, int line)
{
	if (line > 0) {
		emtext_line_skip(txt, line - txt->current->lineno);
	} else {
		emtext_line_skip(txt, txt->last->lineno + line - txt->current->lineno + 1);
	}
}

// -----------------------------------------------------------------------
int emtext_get_current_line(EMTEXT *txt)
{
	return txt->current->lineno;
}

// -----------------------------------------------------------------------
int emtext_get_lines(EMTEXT *txt)
{
	return txt->last->lineno;
}

// -----------------------------------------------------------------------
void emtext_line_first(EMTEXT *txt)
{
	txt->current = txt->first;
}

// -----------------------------------------------------------------------
void emtext_line_last(EMTEXT *txt)
{
	txt->current = txt->last;
}

// -----------------------------------------------------------------------
int emtext_print_from(EMTEXT *txt, EMTILE *t, struct emui_tchunk *chunk)
{
	int lines_printed = 0;

	emuixy(t, 0, 0);
	while (chunk && (lines_printed < t->i.h)) {
		lines_printed += chunk->lines;
		emuiprt(t, chunk->style, "%s", chunk->txt);
		chunk = chunk->next;
	}

	return lines_printed;
}

// -----------------------------------------------------------------------
int emtext_print(EMTEXT *txt, EMTILE *t)
{
	return emtext_print_from(txt, t, txt->current);
}

// vim: tabstop=4 shiftwidth=4 autoindent
