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
#include "style.h"
#include "event.h"
#include "focus.h"

static int emui_tile_debug_mode = 0;

// -----------------------------------------------------------------------
void emui_tile_update_geometry(struct emui_tile *t)
{
	struct emui_tile *parent = t->parent;

	if (!parent) {
		// nothing to do for the root tile (screen)
		return;
	}

	// set decoration window geometry
	if (t->properties & P_GEOM_FORCED) {
		if (t->properties & P_HIDDEN) {
			// just leave, don't even try to unhide it
			return;
		} else {
			// nothing to do here
			// dx, dy, dw, dh should be set by the parent already
		}
	} else if (t->properties & P_MAXIMIZED) {
		// tile is maximized - use all available parent's area
		t->dx = parent->x;
		t->dy = parent->y;
		t->dw = parent->w;
		t->dh = parent->h;
	} else {
		t->dx = parent->x + t->rx;
		t->dy = parent->y + t->ry;
		t->dw = t->rw;
		t->dh = t->rh;
	}

	// hide tile if outside parent's area
	if ((t->dx >= parent->x + parent->w) || (t->dy >= parent->y + parent->h)) {
		t->properties |= P_HIDDEN;
		// give up on hidden tile
		return;
	// unhide and fit otherwise
	} else {
		t->properties &= ~P_HIDDEN;
	}

	// fit tile width to parent's width
	if (t->dw + t->dx > parent->dw + parent->dx) {
		t->dw = parent->w - (t->dx - parent->x);
	}

	// fit tile to parent's height
	if (t->dh + t->dy > parent->dh + parent->dy) {
		t->dh = parent->h - (t->dy - parent->y);
	}

	// set contents window geometry
	t->x = t->dx + t->ml;
	t->y = t->dy + t->mt;
	t->w = t->dw - t->ml - t->mr;
	t->h = t->dh - t->mt - t->mb;

	// hide if no space for contents
	if ((t->dw <= t->ml+t->mr) || (t->dh <= t->mt+t->mb)) {
		t->properties |= P_HIDDEN;
		return;
	} else {
		t->properties &= ~P_HIDDEN;
	}

	// prepare decoration window
	if (t->properties & P_DECORATED) {
		if (!t->ncdeco) {
	        t->ncdeco = newwin(t->dh, t->dw, t->dy, t->dx);
		} else {
			wresize(t->ncdeco, t->dh, t->dw);
			mvwin(t->ncdeco, t->dy, t->dx);
		}
	}

	// prepare contents widow
	if (!t->ncwin) {
		t->ncwin = newwin(t->h, t->w, t->y, t->x);
	} else {
		wresize(t->ncwin, t->h, t->w);
		mvwin(t->ncwin, t->y, t->x);
	}
}

// -----------------------------------------------------------------------
static void emui_tile_debug(struct emui_tile *t)
{
	int x, y;
	WINDOW *win;
	char buf[256];
	attr_t attr_old;
	short colorpair_old;

	if (t->type == T_WIDGET) return;

	// format debug string
	buf[255] = '\0';
	snprintf(buf, 256, "d:%i,%i/%ix%i %i,%i/%ix%i%s%s%s%s%s%s%s",
		t->dx,
		t->dy,
		t->dw,
		t->dh,
		t->x,
		t->y,
		t->w,
		t->h,
		emui_has_focus(t) ? (emui_is_focused(t) ? "*" : "+") : " ",
		t->properties & P_MAXIMIZED ? "M" : "",
		t->properties & P_HIDDEN ? "H"  : "",
		t->properties & P_GEOM_FORCED ? "F" : "",
		t->properties & P_FOCUS_GROUP ? "G" : "",
		t->properties & P_INTERACTIVE ? "I" : "",
		t->properties & P_DECORATED ? "D" : ""
	);

	// find a possibly free space to print
	x = t->w - strlen(buf);
	if (x < 0) x = 0;
	// no decoration space
	if (!t->ncdeco) {
		y = t->h - 1;
		win = t->ncwin;
	// bottom decoration
	} else if (t->mb) {
		y = t->dh - 1;
		win = t->ncdeco;
	// top decoration
	} else if (t->mt) {
		y = 0;
		win = t->ncdeco;
	// only left or right decoration
	} else {
		y = t->h - 1;
		win = t->ncwin;
	}

	wattr_get(win, &attr_old, &colorpair_old, NULL);
	wattrset(win, emui_style_get(S_DEBUG));
	mvwprintw(win, y, x, "%s", buf);
	wattrset(win, colorpair_old | attr_old);

	if (t->drv->debug) t->drv->debug(t);
}

// -----------------------------------------------------------------------
int emui_tile_draw(struct emui_tile *t)
{
	// tile is hidden, nothing to do
	if (t->properties & P_HIDDEN) {
		return 1;
	}

	if (t->ncdeco) {
		werase(t->ncdeco);
	}
	if (t->ncwin) {
		werase(t->ncwin);
	}

	// draw the tile
	t->drv->draw(t);

	// DEBUG
	if (emui_tile_debug_mode) {
		emui_tile_debug(t);
	}

	// update ncurses windows, but don't refresh
	// (we'll do the update in emui_loop())
	if (t->properties & P_DECORATED) {
		wnoutrefresh(t->ncdeco);
	}
	wnoutrefresh(t->ncwin);

	return 0;
}

// -----------------------------------------------------------------------
static int emui_tile_handle_user_focus_keys(struct emui_tile *fg, int key)
{
	struct emui_tile *t = fg->fg_first;

	while (t) {
		edbg("%i == %i ?\n", t->key, key);
		if (t->key == key) {
			edbg("match\n");
			emui_focus(t);
			return 0;
		}
		t = t->fg_next;
	}

	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
static int emui_tile_handle_neighbour_focus(struct emui_tile *t, int key)
{
	switch (key) {
		case 9: // TAB
			emui_focus_list_neighbour(t, FC_NEXT);
			return 0;
		case KEY_BTAB:
			emui_focus_list_neighbour(t, FC_PREV);
			return 0;
		case KEY_UP:
			emui_focus_physical_neighbour(t, FC_UP);
			return 0;
		case KEY_DOWN:
			emui_focus_physical_neighbour(t, FC_DOWN);
			return 0;
		case KEY_LEFT:
			emui_focus_physical_neighbour(t, FC_LEFT);
			return 0;
		case KEY_RIGHT:
			emui_focus_physical_neighbour(t, FC_RIGHT);
			return 0;
		default:
			return 1;
	}
}

// -----------------------------------------------------------------------
int emui_tile_handle_event(struct emui_tile *t, struct emui_event *ev)
{
	// if tile has a driver event handler, run it first
	if (!t->drv->event_handler(t, ev)) {
		edbg("\"%s\" driver handled event %i %i\n", t->name, ev->type, ev->data.key);
		return 0;
	}

	// then run user event handler, so it may override default focus handling
	if (t->user_ev_handler && !t->user_ev_handler(t, ev)) {
		edbg("\"%s\" user event handler handled event %i %i\n", t->name, ev->type, ev->data.key);
		return 0;
	}

	// if tile is a focus group, search for user-set focus keys handled by it
	if (t->properties & P_FOCUS_GROUP) {
		if ((ev->type == EV_KEY) && !emui_tile_handle_user_focus_keys(t, ev->data.key)) {
			edbg("\"%s\" focus event handler handled event %i %i\n", t->name, ev->type, ev->data.key);
			return 0;
		}
	// if tile is a widget, handle neighbourhood focus change
	} else if (t->properties & P_INTERACTIVE) {
		if ((ev->type == EV_KEY) && !emui_tile_handle_neighbour_focus(t, ev->data.key)) {
			return 0;
		}
	}

	edbg("unhandled event %i %i\n", ev->type, ev->data.key);
	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
static const int emui_tile_compatibile(struct emui_tile *parent, int child_type)
{
	static const int tile_compat[T_NUMTYPES][T_NUMTYPES] = {
	/*					child:								*/
	/* parent:			T_CONTAINER	T_WINDOW	T_WIDGET	*/
	/* T_CONTAINER */	{1,			1,			1 },
	/* T_WINDOW */		{1,			0,			1 },
	/* T_WIDGET */		{0,			0,			0 },
	};

	return tile_compat[parent->type][child_type];
}

// -----------------------------------------------------------------------
struct emui_tile * emui_tile_create(struct emui_tile *parent, struct emui_tile_drv *drv, int type, int x, int y, int w, int h, int mt, int mb, int ml, int mr, char *name, int properties)
{
	if (!emui_tile_compatibile(parent, type)) {
		return NULL;
	}

	struct emui_tile *t = calloc(1, sizeof(struct emui_tile));
	if (!t) return NULL;

	if (name) {
		t->name = strdup(name);
		if (!t->name) {
			free(t);
			return NULL;
		}
	}

	t->type = type;
	t->properties = properties;
	t->drv = drv;

	t->rx = x;
	t->ry = y;
	t->rh = h;
	t->rw = w;

	t->mt = mt;
	t->mb = mb;
	t->ml = ml;
	t->mr = mr;

	if (mt || mb || ml || mr) {
		t->properties |= P_DECORATED;
	}

	emui_tile_child_append(parent, t);
	emui_tile_update_geometry(t);

	return t;
}

// -----------------------------------------------------------------------
int emui_tile_set_focus_key(struct emui_tile *t, int key)
{
	t->key = key;
	return 1;
}

// -----------------------------------------------------------------------
int emui_tile_set_event_handler(struct emui_tile *t, emui_event_handler_f handler)
{
	t->user_ev_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emui_tile_set_properties(struct emui_tile *t, unsigned properties)
{
	if (properties & ~P_USER_SETTABLE) {
		return -1;
	}

	t->properties |= properties;

	return 0;
}

// -----------------------------------------------------------------------
void emui_tile_child_append(struct emui_tile *parent, struct emui_tile *t)
{   
	// lint into children list
	t->parent = parent;
	t->prev = parent->ch_last;
	if (parent->ch_last) {
		parent->ch_last->next = t;
	} else {
		parent->ch_first = t;
	}
	parent->ch_last = t;

	// link into focus group list
	emui_focus_group_add(parent, t);
}

// -----------------------------------------------------------------------
void emui_tile_child_unlink(struct emui_tile *t)
{
	if (!t->parent) return;

	if (t->prev) {
		t->prev->next = t->next;
	}
	if (t->next) {
		t->next->prev = t->prev;
	}
	if (t == t->parent->ch_first) {
		t->parent->ch_first = t->next;
	}
	if (t == t->parent->ch_last) {
		t->parent->ch_last = t->prev;
	}
}

// -----------------------------------------------------------------------
static void emui_tile_free(struct emui_tile *t)
{
	t->drv->destroy_priv_data(t);
	free(t->name);

	// free ncurses windows
	delwin(t->ncwin);
	if (t->ncdeco) {
		delwin(t->ncdeco);
	}

	free(t);
}

// -----------------------------------------------------------------------
static void emui_tile_destroy_children(struct emui_tile *t)
{
	struct emui_tile *ch = t->ch_first;
	while (ch) {
		struct emui_tile *next_ch = ch->next;
		emui_tile_destroy_children(ch);
		emui_tile_free(ch);
		ch = next_ch;
	}
}

// -----------------------------------------------------------------------
void emui_tile_destroy(struct emui_tile *t)
{
	if (!t) return;

	// remove the tile from parent's child list
	emui_tile_child_unlink(t);

	// remove from focus group
	emui_focus_group_unlink(t);

	// destroy all child tiles
	emui_tile_destroy_children(t);

	// remove the tile itself
	emui_tile_free(t);
}

// -----------------------------------------------------------------------
void emui_tile_debug_set(int i)
{
	emui_tile_debug_mode = i;
}

// vim: tabstop=4 shiftwidth=4 autoindent
