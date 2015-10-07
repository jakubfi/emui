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
#include <sys/time.h>

#include "emui.h"
#include "tile.h"
#include "event.h"
#include "style.h"
#include "print.h"

struct fpscounter {
	int style;
	struct timeval t;
	double fps;
	int modulo;
};

// -----------------------------------------------------------------------
void emui_fpscounter_draw(struct emui_tile *t)
{
	struct fpscounter *d = t->priv_data;

	if (emui_get_frame() % d->modulo == 0) {
		time_t o_sec = d->t.tv_sec;
		suseconds_t o_usec = d->t.tv_usec;

		gettimeofday(&(d->t), NULL);
		double frame_time = (d->t.tv_sec - o_sec) * 1000000.0 + (d->t.tv_usec - o_usec);
		d->fps = d->modulo * 1000000.0 / frame_time;
	}

	emuixyprt(t, 0, 0, d->style, "%.02f", d->fps);
}

// -----------------------------------------------------------------------
void emui_fpscounter_debug(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
int emui_fpscounter_update_geometry(struct emui_tile *t)
{
	return 0;
}

// -----------------------------------------------------------------------
int emui_fpscounter_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_fpscounter_destroy_priv_data(struct emui_tile *t)
{
	free(t->priv_data);
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_fpscounter_drv = {
	.draw = emui_fpscounter_draw,
	.debug = emui_fpscounter_debug,
	.update_geometry = emui_fpscounter_update_geometry,
	.event_handler = emui_fpscounter_event_handler,
	.destroy_priv_data = emui_fpscounter_destroy_priv_data,
};

// -----------------------------------------------------------------------
void emui_fpscounter_set_style(struct emui_tile *t, int style)
{
	struct fpscounter *d = t->priv_data;
	d->style = style;
}

// -----------------------------------------------------------------------
struct emui_tile * emui_fpscounter_new(struct emui_tile *parent, int x, int y, int style)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, &emui_fpscounter_drv, T_WIDGET, x, y, 6, 1, 0, 0, 0, 0, NULL, P_NONE);

	t->priv_data = calloc(1, sizeof(struct fpscounter));

	struct fpscounter *d = t->priv_data;

	d->style = style;

	unsigned fps = emui_get_fps();
	if (fps >= 4) {
		d->modulo = fps / 4;
	} else if (fps >= 2) {
		d->modulo = 2;
	} else {
		d->modulo = 1;
	}

	return t;
}

// vim: tabstop=4 shiftwidth=4 autoindent
