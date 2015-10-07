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

#ifndef EMUI_TILE_H
#define EMUI_TILE_H

#include <ncurses.h>

enum emui_text_types {
	TT_TEXT,
	TT_DEC,
	TT_HEX,
	TT_OCT,
	TT_BIN,
	TT_FLOAT,
};

enum emui_fit_types {
	FIT_DIV5 = -5,
	FIT_DIV4 = -4,
	FIT_DIV3 = -3,
	FIT_DIV2 = -2,
	FIT_FILL = -1,
};

enum emui_tile_types {
	T_CONTAINER,
	T_WINDOW,
	T_WIDGET,
	
	T_NUMTYPES
};

enum emui_align_types {
	AL_LEFT,
	AL_RIGHT,
	AL_TOP,
	AL_BOTTOM,
	AL_CENTER,
	AL_MIDDLE,
};

enum emui_tile_properties {
	P_NONE			= 0,
	// user-settable
	P_MAXIMIZED		= 1 << 0,	// tile is maximized within parent's geometry
	// internal
	P_HIDDEN		= 1 << 1,	// tile is hidden due to geometry constraints
	P_DECORATED		= 1 << 2,	// tile has decoration
	P_GEOM_FORCED	= 1 << 3,	// tile geometry is forced by the parent
	P_unused_3		= 1 << 4,
	P_FOCUS_GROUP	= 1 << 5,	// tile is a root of focus group
	P_INTERACTIVE	= 1 << 6,	// user can interact with the tile (thus it can be focused)
	P_unused_2		= 1 << 7,
};

#define P_USER_SETTABLE ( P_MAXIMIZED )

struct emui_tile;
struct emui_event;

typedef void (*emui_draw_f)(struct emui_tile *t);
typedef int (*emui_update_geometry_f)(struct emui_tile *t);
typedef int (*emui_event_handler_f)(struct emui_tile *t, struct emui_event *ev);
typedef void (*emui_destroy_priv_data_f)(struct emui_tile *t);

struct emui_tile_drv {
	emui_draw_f draw;
	emui_draw_f debug;
	emui_update_geometry_f update_geometry;
	emui_event_handler_f event_handler;
	emui_destroy_priv_data_f destroy_priv_data;
};

struct emui_tile {
	// general
	unsigned type;				// tile type
	char *name;					// tile name
	unsigned properties;		// tile properties
	int key;					// shortcut key

	// geometry
	unsigned rx, ry, rw, rh;	// user-requested tile geometry including decoration (relative to parent's work area)
	unsigned mt, mb, ml, mr;	// user-requested margins for decoration
	unsigned dx, dy, dw, dh;	// actual tile area including decoration
	unsigned x, y, w, h;		// actual tile work area geometry (d* minus m*)

	// ncurses data
	WINDOW *ncdeco;				// decoration ncurses window
	WINDOW *ncwin;				// contents ncurses window

	// UI hierarchical structure
	struct emui_tile *parent;	// parent tile
	struct emui_tile *ch_first;	// children list start
	struct emui_tile *ch_last;	// children list end
	struct emui_tile *next;		// next tile in child list
	struct emui_tile *prev;		// previous tile in child list
	struct emui_tile *focus;	// next tile in focus chain

	// focus group structure
	struct emui_tile *fg;		// tile's focus group
	struct emui_tile *fg_first;	// focus group start
	struct emui_tile *fg_last;	// focus group end
	struct emui_tile *fg_next;	// next tile in focus group list
	struct emui_tile *fg_prev;	// previous tile in focus group list

	// tile-specific data and methods
	void *priv_data;
	struct emui_tile_drv *drv;

	// user data and methods
	emui_event_handler_f user_ev_handler;
};

struct emui_tile * emui_tile_create(struct emui_tile *parent, struct emui_tile_drv *drv, int type, int x, int y, int w, int h, int mt, int mb, int ml, int mr, char *name, int properties);
void emui_tile_destroy(struct emui_tile *t);
void emui_tile_debug_set(int i);

void emui_tile_child_append(struct emui_tile *parent, struct emui_tile *t);
void emui_tile_child_remove(struct emui_tile *parent, struct emui_tile *t);

void emui_tile_update_geometry(struct emui_tile *t);
int emui_tile_draw(struct emui_tile *t);
int emui_tile_handle_event(struct emui_tile *t, struct emui_event *ev);
int emui_tile_set_event_handler(struct emui_tile *t, emui_event_handler_f handler);
int emui_tile_set_focus_key(struct emui_tile *t, int key);
int emui_tile_set_properties(struct emui_tile *t, unsigned properties);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
