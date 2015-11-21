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
static inline void geom(EMTILE *ch, int x, int y, int w, int h)
{
	if (!ch) return;

	// disable it, if doesn't fit
	if ((w <= 0) || (h <= 0)) {
		ch->properties |= P_HIDDEN;
	} else {
		ch->properties &= ~P_HIDDEN;
		ch->e.x = x;
		ch->e.y = y;
		ch->e.w = w;
		ch->e.h = h;
	}
}

// -----------------------------------------------------------------------
void emui_splitter_update_geometry(EMTILE *t)
{
	struct splitter *d = t->priv_data;

	EMTILE *ch1 = t->ch_first;
	EMTILE *ch2 = ch1 ? t->ch_first->ch_next : NULL;
	int space, size1, size2;

	// set geometry to forced for first two children
	if (ch1) {
		ch1->properties |= P_GEOM_FORCED;
		if (ch2) {
			ch2->properties |= P_GEOM_FORCED;
		}
	} else {
		// no children, nothing to do
		return;
	}

	// get the available space for align
	switch (d->edge) {
		case AL_TOP:
		case AL_BOTTOM:
			space = t->i.h;
			break;
		case AL_RIGHT:
		case AL_LEFT:
			space = t->i.w;
			break;
		default:
			// unknown/unhandled alignment
			return;
	}

	// fit children in the container's space
	fit(space, d->min1, d->max1, d->min2, &size1, &size2);

	// set children geometry
	switch (d->edge) {
		case AL_LEFT:
			geom(ch1, t->i.x,         t->i.y, size1, t->i.h);
			geom(ch2, t->i.x + size1, t->i.y, size2, t->i.h);
			break;
		case AL_RIGHT:
			geom(ch1, t->i.x + t->i.w - size1, t->i.y, size1, t->i.h);
			geom(ch2, t->i.x,                  t->i.y, size2, t->i.h);
			break;
		case AL_TOP:
			geom(ch1, t->i.x, t->i.y,         t->i.w, size1);
			geom(ch2, t->i.x, t->i.y + size1, t->i.w, size2);
			break;
		case AL_BOTTOM:
			geom(ch1, t->i.x, t->i.y + t->i.h - size1, t->i.w, size1);
			geom(ch2, t->i.x, t->i.y,                  t->i.w, size2);
			break;
	}
}

// -----------------------------------------------------------------------
void emui_splitter_destroy_priv_data(EMTILE *t)
{
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emtile_drv emui_splitter_drv = {
	.draw = NULL,
	.update_children_geometry = emui_splitter_update_geometry,
	.event_handler = NULL,
	.focus_handler = NULL,
	.destroy_priv_data = emui_splitter_destroy_priv_data,
};

// -----------------------------------------------------------------------
EMTILE * emui_splitter(EMTILE *parent, int edge, int min1, int max1, int min2)
{
	EMTILE *t;

	if ((edge != AL_TOP) && (edge != AL_BOTTOM) && (edge != AL_LEFT) && (edge != AL_RIGHT)) {
		return NULL;
	}

	t = emtile(parent, &emui_splitter_drv, 0, 0, parent->i.w, parent->i.h, 0, 0, 0, 0, "Splitter", P_CONTAINER | P_MAXIMIZED);

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
