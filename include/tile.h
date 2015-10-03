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

enum emui_tile_types {
	T_CONTAINER,
	T_WINDOW,
	T_WIDGET,
	
	T_NUMTYPES
};

enum emui_tile_properties {
	P_NONE			= 0,
	P_MAXIMIZED		= 1 << 0,
	P_HIDDEN		= 1 << 1,
	P_CHILD_CTRL	= 1 << 2,
	P_GEOM_FORCED	= 1 << 3,
	P_BORDERLESS	= 1 << 4,
	P_FOCUS_GROUP	= 1 << 5,
};

#define P_USER_SETTABLE (P_MAXIMIZED | P_HIDDEN | P_BORDERLESS | P_FOCUS_GROUP )

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

	// geometry
	unsigned rx, ry, rw, rh;	// requested tile geometry
	unsigned x, y, w, h;		// actual tile geometry
	unsigned mt, mb, ml, mr;	// margins for decoration

	// ncurses data
	WINDOW *ncdeco;				// decoration ncurses window
	WINDOW *ncwin;				// contents ncurses window

	// UI hierarchical structure
	struct emui_tile *parent;	// parent tile
	struct emui_tile *child_h;	// child tiles list head
	struct emui_tile *child_t;	// child tiles list tail
	struct emui_tile *next;		// next tile in child list
	struct emui_tile *prev;		// previous tile in child list
	struct emui_tile *focus;		// next tile in focus chain

	// tile-specific data and methods
	void *priv_data;
	struct emui_tile_drv *drv;

	// user data and methods
	emui_event_handler_f user_ev_handler;
};

struct emui_tile * emui_tile_create(struct emui_tile *parent, struct emui_tile_drv *drv, int type, int x, int y, int h, int w, int mt, int mb, int ml, int mr, char *name, int properties);
void emui_tile_destroy(struct emui_tile *t);
void emui_tile_debug_set(int i);

void emui_tile_child_append(struct emui_tile *parent, struct emui_tile *t);
void emui_tile_child_remove(struct emui_tile *parent, struct emui_tile *t);
void emui_tile_focus(struct emui_tile *t);
int emui_tile_has_focus(struct emui_tile *t);
int emui_tile_is_focused(struct emui_tile *t);

struct emui_tile * emui_tile_focus_get();

void emui_tile_update_geometry(struct emui_tile *t);
void emui_tile_draw(struct emui_tile *t);
int emui_tile_handle_event(struct emui_tile *t, struct emui_event *ev);
int emui_tile_set_event_handler(struct emui_tile *t, emui_event_handler_f handler);

// tiles

struct emui_tile * emui_screen_new();
struct emui_tile * emui_window_new(struct emui_tile *parent, int x, int y, int w, int h, char *name, int properties);
struct emui_tile * emui_tabs_new(struct emui_tile *parent);
struct emui_tile * emui_framecounter_new(struct emui_tile *parent, int x, int y);
struct emui_tile * emui_fpscounter_new(struct emui_tile *parent, int x, int y);
struct emui_tile * emui_lineedit_new(struct emui_tile *parent, int x, int y, int w, int maxlen, int type, int properties);
int emui_lineedit_settext(struct emui_tile *t, char *text);

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
