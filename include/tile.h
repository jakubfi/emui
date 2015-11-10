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

enum emui_tile_properties {
	P_NONE			= 0,
	// user-settable
	P_MAXIMIZED		= 1 << 0,	// tile is maximized within parent's geometry
	P_FOCUS_GROUP	= 1 << 1,	// tile is a root of focus group
	P_IGNORE_MARGINS= 1 << 2,	// tile will ignore parent's margins
	P_FLOAT			= 1 << 3,	// floating tile (detached from parent's geometry)
	P_INVERSE		= 1 << 4,	// tile is drawn in inversed colors
	// internal
	P_HIDDEN		= 1 << 16,	// tile is hidden due to geometry constraints
	P_GEOM_FORCED	= 1 << 17,	// tile geometry is forced by the parent
	P_INTERACTIVE	= 1 << 18,	// user can interact with the tile (thus it can be focused)
	P_NOCANVAS		= 1 << 19,	// tile has no canvas to draw on
	P_CONTAINER		= 1 << 20,	// other tiles can be placed within this tile
};

#define P_USER_SETTABLE 0xffff

#define IS_INTERACTIVE(t) (((t)->properties & (P_INTERACTIVE | P_HIDDEN)) == P_INTERACTIVE)

struct emui_tile;
struct emui_event;

// provided by the tile driver
typedef void (*emui_draw_f)(struct emui_tile *t);
typedef int (*emui_update_geometry_f)(struct emui_tile *t);
typedef void (*emui_destroy_priv_data_f)(struct emui_tile *t);
typedef int (*emui_event_handler_f)(struct emui_tile *t, struct emui_event *ev);

// user-provided handlers
typedef int (*emui_handler_f)(struct emui_tile *t);
typedef int (*emui_key_handler_f)(struct emui_tile *t, int key);

struct emui_tile_drv {
	emui_draw_f draw;
	emui_update_geometry_f update_children_geometry;
	emui_event_handler_f event_handler;
	emui_destroy_priv_data_f destroy_priv_data;
};

struct emui_geom {
	int x, y, w, h;
};

struct emui_tile {
	// general
	unsigned type;				// tile type (lineedit, label, window, ...)
	int id;						// tile id (user may set it to whatever is needed by the application)
	char *name;					// tile name
	unsigned properties;		// tile properties
	int key;					// shortcut key
	int style;					// default style
	int geometry_changed;		// geometry has changed (and needs to be updated)
	int accept_updates;			// tile accepts content updates (contents are not being edited)
	int content_invalid;		// tile contents are invalid after last edit

	// geometry
	unsigned mt, mb, ml, mr;	// tile-requested interior margins
	struct emui_geom r;			// user-requested tile geometry (relative to parent's internal work area)
	struct emui_geom e;			// actual external tile area
	struct emui_geom i;			// actual internal tile work area (d* minus m*)

	// ncurses data
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

	// user-provided handlers
	emui_handler_f user_update_handler;
	emui_handler_f user_change_handler;
	emui_key_handler_f user_key_handler;
};

struct emui_tile * emui_tile_create(struct emui_tile *parent, int id, struct emui_tile_drv *drv, int x, int y, int w, int h, int mt, int mb, int ml, int mr, char *name, int properties);
void emui_tile_destroy(struct emui_tile *t);
void emui_tile_child_append(struct emui_tile *parent, struct emui_tile *t);

void emui_tile_update_geometry(struct emui_tile *t);
int emui_tile_draw(struct emui_tile *t);

int emui_tile_set_update_handler(struct emui_tile *t, emui_handler_f handler);
int emui_tile_set_change_handler(struct emui_tile *t, emui_handler_f handler);
int emui_tile_set_key_handler(struct emui_tile *t, emui_key_handler_f handler);
int emui_tile_set_focus_key(struct emui_tile *t, int key);
int emui_tile_set_properties(struct emui_tile *t, unsigned properties);
int emui_tile_clear_properties(struct emui_tile *t, unsigned properties);
int emui_tile_set_name(struct emui_tile *t, char *name);
int emui_tile_set_style(struct emui_tile *t, int style);
void emui_tile_set_id(struct emui_tile *t, int id);
int emui_tile_get_id(struct emui_tile *t);
void emui_tile_set_margins(struct emui_tile *t, int mt, int mb, int ml, int mr);

void emui_tile_hide(struct emui_tile *t);
void emui_tile_unhide(struct emui_tile *t);
void emui_tile_inverse(struct emui_tile *t, int inv);
void emui_tile_geometry_changed(struct emui_tile *t);
int emui_tile_changed(struct emui_tile *t);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
