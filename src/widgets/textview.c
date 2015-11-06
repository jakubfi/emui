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

#include <stdlib.h>
#include <string.h>

#include "tile.h"
#include "tiles.h"
#include "event.h"
#include "style.h"
#include "print.h"

struct tchunk {
	int style;
	char *txt;
	struct tchunk *next, *prev;
	int lines;
};

struct textview {
	struct tchunk *beg, *end, *cur;
};

// -----------------------------------------------------------------------
void emui_textview_draw(struct emui_tile *t)
{
	struct textview *d = t->priv_data;
	struct tchunk *c = d->cur;

	int lines = 0;
	emuixy(t, 0, 0);

	while (c && lines < t->h) {
		lines += c->lines;
		emuiprt(t, c->style, "%s", c->txt);
		c = c->next;
	}
}

// -----------------------------------------------------------------------
int emui_textview_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	struct textview *d = t->priv_data;
	struct tchunk *c = d->cur;
	int lines = 0;

	if (ev->type == EV_KEY) {
		switch (ev->sender) {
			case KEY_UP:
				if (c) {
					c = c->prev;
					while (c && !c->lines) {
						c = c->prev;
					}
					if (c) {
						d->cur = c;
					}
				}
				return 0;
			case KEY_DOWN:
				if (c) {
					c = c->next;
					while (c && !c->lines) {
						c = c->next;
					}
					if (c) {
						d->cur = c;
					}
				}
				return 0;
			case KEY_HOME:
				d->cur = d->beg;
				return 0;
			case KEY_END:
				d->cur = d->end;
				return 0;
			case KEY_PPAGE:
				if (c) {
					while (c && (lines < t->h-1)) {
						lines += c->lines;
						c = c->prev;
					}
					if (c) {
						d->cur = c;
					} else {
						d->cur = d->beg;
					}
				}
				return 0;
			case KEY_NPAGE:
				if (c) {
					while (c && (lines < t->h-1)) {
						lines += c->lines;
						c = c->next;
					}
					if (c) {
						d->cur = c;
					} else {
						d->cur = d->end;
					}
				}
				return 0;
			default:
				break;
		}
	}

	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_textview_destroy_priv_data(struct emui_tile *t)
{
	emui_textview_clear(t);
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_textview_drv = {
	.draw = emui_textview_draw,
	.update_geometry = NULL,
	.event_handler = emui_textview_event_handler,
	.destroy_priv_data = emui_textview_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_textview(struct emui_tile *parent, int x, int y, int w, int h)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, -1, &emui_textview_drv, F_WIDGET, x, y, w, h, 0, 0, 0, 0, NULL, P_INTERACTIVE);

	t->priv_data = calloc(1, sizeof(struct textview));

	return t;
}

// -----------------------------------------------------------------------
static void _chunk_free(struct tchunk *c)
{
	if (!c) return;
	free(c->txt);
	free(c);
}

// -----------------------------------------------------------------------
static void _chunks_destroy(struct tchunk *c)
{
	struct tchunk *next;
	while (c) {
		next = c->next;
		_chunk_free(c);
		c = next;
	}
}

// -----------------------------------------------------------------------
void emui_textview_clear(struct emui_tile *t)
{
	struct textview *d = t->priv_data;
	_chunks_destroy(d->beg);
	d->beg = d->end = d->cur = NULL;
}

// -----------------------------------------------------------------------
static void _tv_append(struct textview *d, struct tchunk *chunk)
{
	if (d->end) {
		d->end->next = chunk;
	}
	chunk->prev = d->end;
	chunk->next = NULL;
	d->end = chunk;

	if (!d->beg) {
		d->beg = chunk;
		d->cur = chunk;
	}
}

// -----------------------------------------------------------------------
int emui_textview_append(struct emui_tile *t, int style, char *str)
{
	struct textview *d = t->priv_data;
	struct tchunk *chunk;
	char *nl;

	nl = strchr(str, '\n');
	while (nl) {
		int size = nl-str+1;
		chunk = malloc(sizeof(struct tchunk));
		chunk->style = style;
		chunk->lines = 1;
		chunk->txt = malloc(size+1);
		chunk->txt[size] = '\0';
		strncpy(chunk->txt, str, size);
		_tv_append(d, chunk);
		str = nl+1;
		nl = strchr(str, '\n');
	}

	if (*str) {
		chunk = malloc(sizeof(struct tchunk));
		chunk->style = style;
		chunk->lines = 0;
		chunk->txt = strdup(str);
		_tv_append(d, chunk);
	}

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
