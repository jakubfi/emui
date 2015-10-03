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

static int emui_tile_debug_mode = 0;

static struct emui_tile *focus;

// -----------------------------------------------------------------------
void emui_tile_update_geometry(struct emui_tile *t)
{
	if (!t->parent) {
		// nothing to do for the root tile (screen)
		return;
	}

	if (t->properties & P_GEOM_FORCED) {
		// nothing to do if tile geometry is controlled by the parent
		return;
	}

	if (t->properties & P_MAXIMIZED) {
		t->x = t->parent->x + t->parent->ml;
		t->y = t->parent->y + t->parent->mt;
		t->w = t->parent->w - (t->parent->ml + t->parent->mr);
		t->h = t->parent->h - (t->parent->mt + t->parent->mb);
	} else {
		t->x = t->rx + t->parent->x;
		t->y = t->ry + t->parent->y;
		t->w = t->rw;
		t->h = t->rh;
	}

	// draw decoration if it's there
	if (t->ncdeco) {
		wresize(t->ncdeco, t->h, t->w);
		mvwin(t->ncdeco, t->y, t->x);
		werase(t->ncdeco);
	}

	wresize(t->ncwin, t->h-(t->mt+t->mb), t->w-(t->ml+t->mr));
	mvwin(t->ncwin, t->y+t->mt, t->x+t->ml);
	werase(t->ncwin);
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
	snprintf(buf, 256, "%i,%i %ix%i (%i,%i %ix%i)%s%s%s%s%s%s",
		t->x,
		t->y,
		t->w,
		t->h,
		t->rx,
		t->ry,
		t->rw,
		t->rh,
		emui_tile_has_focus(t) ? (emui_tile_is_focused(t) ? "*" : "+") : "",
		t->properties & P_MAXIMIZED ? "MAX " : "",
		t->properties & P_HIDDEN ? "HID "  : "",
		t->properties & P_CHILD_CTRL ? "CTL " : "",
		t->properties & P_GEOM_FORCED ? "FORC " : "",
		t->properties & P_BORDERLESS ? "BLS " : ""
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
		y = t->h - 1;
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
void emui_tile_draw(struct emui_tile *t)
{
	// do tile-specific geometry updates
	t->drv->update_geometry(t);
	// draw the tile
	t->drv->draw(t);

	// DEBUG
	if (emui_tile_debug_mode) {
		emui_tile_debug(t);
	}

	// update ncurses windows, but don't refresh
	// (we'll do the update in emui_loop())
	if (t->ncdeco) {
		wnoutrefresh(t->ncdeco);
	}
	wnoutrefresh(t->ncwin);
}

// -----------------------------------------------------------------------
int emui_tile_handle_event(struct emui_tile *t, struct emui_event *ev)
{
	if (!t->drv->event_handler(t, ev)) {
		return 0;
	} else if (t->user_ev_handler) {
		t->user_ev_handler(t, ev);
	}

	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
static const int emui_tile_compatibile(struct emui_tile *parent, int child_type)
{
	static const int tile_compat[T_NUMTYPES][T_NUMTYPES] = {
	/*					child:								*/
	/* parent:			T_CONTAINER	T_WINDOW	T_WIDGET	*/
	/* T_CONTAINER */	{1,			1,			0 },
	/* T_WINDOW */		{1,			0,			1 },
	/* T_WIDGET */		{0,			0,			0 },
	};

	return tile_compat[parent->type][child_type];
}

// -----------------------------------------------------------------------
struct emui_tile * emui_tile_create(struct emui_tile *parent, struct emui_tile_drv *drv, int type, int x, int y, int h, int w, int mt, int mb, int ml, int mr, char *name, int properties)
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
	t->properties = properties & P_USER_SETTABLE;
	t->drv = drv;

	t->rx = x + parent->x + parent->ml;
	t->ry = y + parent->y + parent->mt;
	t->rh = h;
	t->rw = w;

	t->mt = mt;
	t->mb = mb;
	t->ml = ml;
	t->mr = mr;

	// create ncurses windows as requested by the user
	// emui_tile_update_geometry() will redo them anyway

	// if space for decoration is needed
	if ((t->mt) || (t->mb) || (t->ml) || (t->mr)) {
		t->ncdeco = newwin(t->rh, t->rw, t->ry, t->rx);
	}
	t->ncwin = newwin(t->rh-(t->mt+t->mb), t->rw-(t->ml+t->mr), t->ry+t->mt, t->rx+t->ml);

	emui_tile_child_append(parent, t);

	return t;
}

// -----------------------------------------------------------------------
void emui_tile_focus(struct emui_tile *t)
{
	// go down the tree if tile is not a leaf (focus needs to cover full tile path)
	while (t->child_h) {
		if (t->focus) {
			 t = t->focus;
		} else {
			t = t->child_h;
		}
	}

	focus = t;

	// set new focus path
	while (t->parent) {
		t->parent->focus = t;
		t = t->parent;
	}
}

// -----------------------------------------------------------------------
int emui_tile_is_focused(struct emui_tile *t)
{
	if (t == focus) return 1;
	else return 0;
}

// -----------------------------------------------------------------------
int emui_tile_has_focus(struct emui_tile *t)
{
	struct emui_tile *f = focus;
	while (f) {
		if (t == f) return 1;
		f = f->parent;
	}
	return 0;
}

// -----------------------------------------------------------------------
struct emui_tile * emui_tile_focus_get()
{
	return focus;
}

// -----------------------------------------------------------------------
int emui_tile_set_event_handler(struct emui_tile *t, emui_event_handler_f handler)
{
	t->user_ev_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
void emui_tile_child_append(struct emui_tile *parent, struct emui_tile *t)
{   
	t->parent = parent;
	t->prev = parent->child_t;
	if (parent->child_t) {
		parent->child_t->next = t;
	} else {
		parent->child_h = t;
	}
	parent->child_t = t;

	// if parent controls child geometry, set geometry to forced
	if (parent->properties & P_CHILD_CTRL) {
		t->properties |= P_GEOM_FORCED;
	}
}

// -----------------------------------------------------------------------
void emui_tile_child_remove(struct emui_tile *parent, struct emui_tile *t)
{
	struct emui_tile *ch = parent->child_h;
	while (ch) {
		if (ch == t) {
			if (ch->prev) {
				ch->prev->next = ch->next;
			}
			if (ch->next) {
				ch->next->prev = ch->prev;
			}
			if (ch == parent->child_h) {
				parent->child_h = ch->prev;
			}
			if (ch == parent->child_t) {
				parent->child_t = ch->next;
			}
			break;
		}
	}
}

// -----------------------------------------------------------------------
static void emui_tile_free(struct emui_tile *t)
{
	t->drv->destroy_priv_data(t);
	free(t->name);
	delwin(t->ncwin);
	if (t->ncdeco) {
		delwin(t->ncdeco);
	}
	free(t);
}

// -----------------------------------------------------------------------
static void emui_tile_destroy_children(struct emui_tile *t)
{
	struct emui_tile *ch = t->child_h;
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

	// destroy all child tiles
	emui_tile_destroy_children(t);

	// remove the tile from parent's child list
	if (t->parent) {
		emui_tile_child_remove(t->parent, t);
	}

	// remove the tile itself
	emui_tile_free(t);
}

// -----------------------------------------------------------------------
void emui_tile_debug_set(int i)
{
	emui_tile_debug_mode = i;
}

// vim: tabstop=4 shiftwidth=4 autoindent
