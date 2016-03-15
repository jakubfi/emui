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

#include "event.h"

enum emui_text_types {
	TT_TEXT,
	TT_INT,
	TT_HEX,
	TT_OCT,
	TT_BIN,
};

enum emui_edit_modes {
	M_INS,
	M_OVR,
};

enum emui_fit_types {
	FIT_DIV5 = -5,
	FIT_DIV4 = -4,
	FIT_DIV3 = -3,
	FIT_DIV2 = -2,
	FIT_FILL = -1,
};

enum emui_align_types {
	AL_LEFT,
	AL_RIGHT,
	AL_TOP,
	AL_BOTTOM,
	AL_CENTER,
	AL_MIDDLE,
	AL_HORIZONTAL,
	AL_VERTICAL,
};

enum emui_geometries {
	GEOM_INTERNAL,
	GEOM_EXTERNAL
};

enum emui_return_codes {
	E_OK = 0,
	E_ALLOC,
};

enum emui_handler_results {
	E_HANDLED = 0,
	E_UNHANDLED = 1,
};

enum emui_content_state {
	E_VALID = 0,
	E_INVALID = 1,
};

enum emui_update_results {
	E_UPDATED = 0,
	E_UNCHANGED = 1,
};

enum emui_tile_properties {
	P_NONE			= 0,
	// app-settable
	P_HMAXIMIZE		= 1 << 0,	// tile is maximized horizontally
	P_VMAXIMIZE		= 1 << 1,	// tile is maximized vertically
	P_MAXIMIZE		= P_HMAXIMIZE | P_VMAXIMIZE, // tile is maximized
	P_VCENTER		= 1 << 2,	// tile is centered vertically
	P_HCENTER		= 1 << 3,	// tile is centered horizontally
	P_CENTER		= P_HCENTER | P_VCENTER, // tile is centered
	P_HFILL			= 1 << 4,	// tile fills the parent horizontaly (to the right edge)
	P_VFILL			= 1 << 5,	// file fills the parent vertically (to the bottom edge)
	P_FLOAT			= 1 << 6,	// floating tile (detached from parent's geometry)
	P_FOCUS_GROUP	= 1 << 7,	// tile is a root of focus group
	P_INVERSE		= 1 << 8,	// tile is drawn in inversed colors
	P_AUTOEDIT		= 1 << 9,	// enter edit mode when focused
	// internal
	P_HIDDEN		= 1 << 16,	// tile is hidden due to geometry constraints
	P_GEOM_FORCED	= 1 << 17,	// tile geometry is forced by the parent
	P_INTERACTIVE	= 1 << 18,	// user can interact with the tile (thus it can be focused)
	P_NOCANVAS		= 1 << 19,	// tile has no canvas to draw on
	P_CONTAINER		= 1 << 20,	// other tiles can be placed within this tile
	P_DELETED		= 1 << 21,	// delete the tile with next emui_draw()
};

#define P_APP_SETTABLE 0xffff

struct emui_event;
struct emui_tile;
typedef struct emui_tile EMTILE;

typedef void (*emui_void_f)(EMTILE *t);
typedef int (*emui_int_f)(EMTILE *t);
typedef int (*emui_int_f_ev)(EMTILE *t, struct emui_event *ev);
typedef int (*emui_int_f_int)(EMTILE *t, int arg);
typedef void (*emui_void_f_int)(EMTILE *t, int arg);
typedef void (*emui_void_f_emtile)(EMTILE *t, EMTILE *f);

struct emtile_drv {
	emui_void_f draw;
	emui_void_f update_children_geometry;
	emui_int_f_ev event_handler;
	emui_void_f_int focus_handler;
	emui_void_f destroy_priv_data;
	emui_void_f_emtile scroll_handler;
};

struct emui_geom {
	int x, y, w, h;
};

struct emui_tile {
	// general
	int __dbg_id;				// for debugging purposes
	char *name;					// tile name
	unsigned properties;		// tile properties
	int key;					// shortcut key
	int style;					// default style
	int geometry_changed;		// geometry has changed (and needs to be updated)
	int accept_updates;			// tile currently accepts content updates (contents are not being edited)
	int content_invalid;		// tile contents are invalid after last edit

	// geometry
	struct emui_geom *pg;		// geometry used for calculating tile geometry (parent->i by default)
	unsigned mt, mb, ml, mr;	// tile-requested interior margins
	struct emui_geom r;			// app-requested tile geometry (parent relative)
	struct emui_geom e;			// actual external tile area
	struct emui_geom i;			// actual internal tile area (d* minus m*)

	// ncurses data
	WINDOW *ncwin;				// ncurses window

	// UI hierarchical structure
	EMTILE *parent;		// parent tile
	EMTILE *ch_first;	// children list start
	EMTILE *ch_last;	// children list end
	EMTILE *ch_next;	// next tile in child list
	EMTILE *ch_prev;	// previous tile in child list
	EMTILE *focus;		// next tile in focus chain

	// focus group structure
	EMTILE *fg;			// tile's focus group
	EMTILE *fg_first;	// focus group start
	EMTILE *fg_last;	// focus group end
	EMTILE *fg_next;	// next tile in focus group list
	EMTILE *fg_prev;	// previous tile in focus group list

	// tile-specific data and methods
	void *priv_data;
	struct emtile_drv *drv;

	// app-provided data and handlers
	void *ptr;
	emui_int_f update_handler;
	emui_int_f change_handler;
	emui_void_f_int focus_handler;
	emui_int_f_int key_handler;
};

EMTILE * emtile(EMTILE *parent, struct emtile_drv *drv, int x, int y, int w, int h, int mt, int mb, int ml, int mr, char *name, int properties);
void emtile_delete(EMTILE *t);
void _emtile_really_delete(EMTILE *t);

void emtile_fit(EMTILE *t);
void emtile_draw(EMTILE *t);
int emtile_event(EMTILE *t, struct emui_event *ev);

void emtile_set_update_handler(EMTILE *t, emui_int_f handler);
void emtile_set_change_handler(EMTILE *t, emui_int_f handler);
void emtile_set_focus_handler(EMTILE *t, emui_void_f_int handler);
void emtile_set_key_handler(EMTILE *t, emui_int_f_int handler);

void emtile_set_focus_key(EMTILE *t, int key);
int emtile_set_properties(EMTILE *t, unsigned properties);
int emtile_clear_properties(EMTILE *t, unsigned properties);
int emtile_set_name(EMTILE *t, char *name);
void emtile_set_style(EMTILE *t, int style);
void emtile_set_ptr(EMTILE *t, void *ptr);
void * emtile_get_ptr(EMTILE *t);
void emtile_set_margins(EMTILE *t, int mt, int mb, int ml, int mr);
void emtile_set_geometry_parent(EMTILE *t, EMTILE *pg, int geom_type);

void emtile_geometry_changed(EMTILE *t);
int emtile_notify_change(EMTILE *t);

EMTILE *emtile_get_list_neighbour(EMTILE *t, int dir, unsigned prop_match, unsigned prop_nomatch);
EMTILE *emtile_get_physical_neighbour(EMTILE *t, int dir, unsigned prop_match, unsigned prop_nomatch);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
