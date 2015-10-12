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
#include <ncurses.h>

#include "tile.h"
#include "event.h"

struct splitter {
	int edge;
	int min1;
	int max1;
	int min2;
};

// -----------------------------------------------------------------------
void emui_splitter_draw(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
void emui_splitter_debug(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
static inline void fit(int space, int min1, int max1, int min2, int *size1, int *size2)
{
	// translate "special" requirements to actual lengths
	if (max1 < 0) {
		max1 = space / -max1;
	}

	// BEST: can we fit both with size1=max1?
	if (space >= max1 + min2) {
		*size1 = max1;
		*size2 = space - *size1;
	// OK: can we fit with both with size1 at least min1?
	} else if (space >= min1 + min2) {
		*size2 = min2;
		*size1 = space - *size2;
	// BAD: can we at least fit ch1?
	} else if (space >= min1) {
		*size1 = space;
		*size2 = 0;
	// REALLY BAD: can we at least fit ch2?
	} else if (space >= min2) {
		*size2 = space;
		*size1 = 0;
	// FATAL: cannot fit anything
	} else {
		*size1 = 0;
		*size2 = 0;
	}
}

// -----------------------------------------------------------------------
static inline void geom(struct emui_tile *ch, int x, int y, int w, int h)
{
	if (!ch) return;

	// disable it, if doesn't fit
	if ((w <= 0) || (h <= 0)) {
		emui_tile_hide(ch);
	} else {
		emui_tile_unhide(ch);
		ch->dx = x;
		ch->dy = y;
		ch->dw = w;
		ch->dh = h;
	}
}

// -----------------------------------------------------------------------
int emui_splitter_update_geometry(struct emui_tile *t)
{
	struct splitter *d = t->priv_data;

	struct emui_tile *ch1 = t->ch_first;
	struct emui_tile *ch2 = ch1 ? t->ch_first->next : NULL;
	int space, size1, size2;

	// set geometry to forced for first two children
	if (ch1) {
		ch1->properties |= P_GEOM_FORCED;
		if (ch2) {
			ch2->properties |= P_GEOM_FORCED;
		}
	} else {
		// no children, nothing to do
		return 0;
	}

	// get the available space for align
	switch (d->edge) {
		case AL_TOP:
		case AL_BOTTOM:
			space = t->h;
			break;
		case AL_RIGHT:
		case AL_LEFT:
			space = t->w;
			break;
		default:
			// unknown/unhandled alignment
			return 0;
	}

	// fit children in the container's space
	fit(space, d->min1, d->max1, d->min2, &size1, &size2);

	// set children geometry
	switch (d->edge) {
		case AL_LEFT:
			geom(ch1, t->x,         t->y, size1, t->h);
			geom(ch2, t->x + size1, t->y, size2, t->h);
			break;
		case AL_RIGHT:
			geom(ch1, t->x + t->w - size1, t->y, size1, t->h);
			geom(ch2, t->x,                t->y, size2, t->h);
			break;
		case AL_TOP:
			geom(ch1, t->x, t->y,         t->w, size1);
			geom(ch2, t->x, t->y + size1, t->w, size2);
			break;
		case AL_BOTTOM:
			geom(ch1, t->x, t->y + t->h - size1, t->w, size1);
			geom(ch2, t->x, t->y,                t->w, size2);
			break;
	}

	return 0;
}

// -----------------------------------------------------------------------
int emui_splitter_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_splitter_destroy_priv_data(struct emui_tile *t)
{
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_splitter_drv = {
	.draw = emui_splitter_draw,
	.debug = emui_splitter_debug,
	.update_geometry = emui_splitter_update_geometry,
	.event_handler = emui_splitter_event_handler,
	.destroy_priv_data = emui_splitter_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_splitter(struct emui_tile *parent, int edge, int min1, int max1, int min2)
{
	struct emui_tile *t;

	if ((edge != AL_TOP) && (edge != AL_BOTTOM) && (edge != AL_LEFT) && (edge != AL_RIGHT)) {
		return NULL;
	}

	t = emui_tile_create(parent, &emui_splitter_drv, F_CONTAINER, 0, 0, parent->w, parent->h, 0, 0, 0, 0, "Splitter", P_MAXIMIZED);

	if (!t) return NULL;

	t->priv_data = calloc(1, sizeof(struct splitter));

	struct splitter *d = t->priv_data;

	d->edge = edge;
	d->min1 = min1;
	d->max1 = max1;
	d->min2 = min2;

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
