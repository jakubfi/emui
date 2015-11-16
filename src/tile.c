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
#include "focus.h"
#include "print.h"

static void emtile_child_append(EMTILE *parent, EMTILE *t);

// -----------------------------------------------------------------------
static void emtile_fit_parent(EMTILE *t)
{
	// (1) 'floating' property overrides everything
	if (t->properties & P_FLOAT) {
		t->properties &= ~P_HIDDEN;
	// (2) hide the child if parent is hidden
	} else if (t->parent->properties & P_HIDDEN) {
		t->properties |= P_HIDDEN;
		return;
	// (3) respect parent's choice regarding children geometry and visibility
	} else if (t->properties & P_GEOM_FORCED) {
		return;
	}

	// calculate own external geometry
	t->e = t->r;
	t->e.x += t->pg->x;
	t->e.y += t->pg->y;

	// respect centering
	if (t->properties & P_HCENTER) {
		t->e.w = t->r.w;
		t->e.x = (t->pg->w - t->r.w) / 2;
		if (t->e.x < 0) {
			t->e.x = 0;
		}
	}
	if (t->properties & P_VCENTER) {
		t->e.h = t->r.h;
		t->e.y = (t->pg->h - t->r.h) / 2;
		if (t->e.y < 0) {
			t->e.y = 0;
		}
	}

	// respect maximization
	if (t->properties & P_HMAXIMIZED) {
		t->e.x = t->pg->x;
		t->e.w = t->pg->w;
	}
	if (t->properties & P_VMAXIMIZED) {
		t->e.y = t->pg->y;
		t->e.h = t->pg->h;
	}

	// fit tile width to parent's width
	if (t->e.w + t->e.x > t->pg->w + t->pg->x) {
		t->e.w = t->pg->w - (t->e.x - t->pg->x);
	}
	// fit tile to parent's height
	if (t->e.h + t->e.y > t->pg->h + t->pg->y) {
		t->e.h = t->pg->h - (t->e.y - t->pg->y);
	}
	// hide tile if outside parent's area
	if ((t->e.x >= t->pg->x + t->pg->w) || (t->e.y >= t->pg->y + t->pg->h)) {
		t->properties |= P_HIDDEN;
	} else {
		t->properties &= ~P_HIDDEN;
	}
}

// -----------------------------------------------------------------------
static void emtile_fit_interior(EMTILE *t)
{
	// hide if no space for internal geometry
	if ((t->e.w <= t->ml+t->mr) || (t->e.h <= t->mt+t->mb)) {
		t->properties |= P_HIDDEN;
		return;
	} else {
		t->properties &= ~P_HIDDEN;
	}

	// set internal tile geometry
	t->i = t->e;
	t->i.x += t->ml;
	t->i.y += t->mt;
	t->i.w -= t->ml + t->mr;
	t->i.h -= t->mt + t->mb;
}

// -----------------------------------------------------------------------
void emtile_fit(EMTILE *t)
{
	if (t->parent) {
		// fit the tile within parent's geometry
		emtile_fit_parent(t);

		// if tile is still visible, set internal geometry
		if (!(t->properties & P_HIDDEN)) {
			emtile_fit_interior(t);

			// if tile is visible, prepare ncurses window
			if (!(t->properties & P_HIDDEN)) {
				// prepare ncurses window
				if (!(t->properties & P_NOCANVAS)) {
					if (!t->ncwin) {
						t->ncwin = newwin(t->e.h, t->e.w, t->e.y, t->e.x);
					} else {
						werase(t->ncwin);
						wresize(t->ncwin, t->e.h, t->e.w);
						mvwin(t->ncwin, t->e.y, t->e.x);
					}
				}
				// tile is inversed if parent is inversed
				if (t->parent->properties & P_INVERSE) {
					t->properties |= P_INVERSE;
				}
				emuifillbg(t, t->style);
			}
		}
	}

	// do tile-specific geometry updates
	if (t->drv->update_children_geometry) t->drv->update_children_geometry(t);

	t->geometry_changed = 0;
}

// -----------------------------------------------------------------------
int emtile_draw(EMTILE *t)
{
	// tile is hidden or has no canvas, nothing to do
	if (t->properties & (P_HIDDEN | P_NOCANVAS)) {
		return 1;
	}

	// if tile accepts content updates and app specified a handler,
	// then update content before the tile is drawn
	if (t->accept_updates && t->update_handler) {
		t->update_handler(t);
	}

	// draw the tile
	if (t->drv->draw) t->drv->draw(t);

	// update ncurses window, but don't output,
	// doupdate() is done in the main loop
	wnoutrefresh(t->ncwin);

	return 0;
}

// -----------------------------------------------------------------------
EMTILE * emtile(EMTILE *parent, struct emtile_drv *drv, int x, int y, int w, int h, int mt, int mb, int ml, int mr, char *name, int properties)
{
	EMTILE *t = calloc(1, sizeof(EMTILE));
	if (!t) return NULL;

	if (!(parent->properties & P_CONTAINER)) {
		return NULL;
	}

	if (name) {
		t->name = strdup(name);
		if (!t->name) {
			free(t);
			return NULL;
		}
	}

	t->properties = properties;
	t->drv = drv;
	t->geometry_changed = 1;
	t->accept_updates = 1;
	t->pg = &(parent->i);

	if (!t->drv->draw) {
		t->properties |= P_NOCANVAS;
	}

	t->r.x = x;
	t->r.y = y;
	t->r.h = h;
	t->r.w = w;

	t->mt = mt;
	t->mb = mb;
	t->ml = ml;
	t->mr = mr;

	emtile_child_append(parent, t);
	emui_focus_group_add(parent, t);
	emtile_fit(t);

	return t;
}

// -----------------------------------------------------------------------
void emtile_set_geometry_parent(EMTILE *t, EMTILE *pg, int geom_type)
{
	if (geom_type == GEOM_EXTERNAL) {
		t->pg = &(pg->e);
	} else {
		t->pg = &(pg->i);
	}
}

// -----------------------------------------------------------------------
int emtile_set_focus_key(EMTILE *t, int key)
{
	t->key = key;
	return 1;
}

// -----------------------------------------------------------------------
int emtile_set_update_handler(EMTILE *t, emui_int_f handler)
{
	t->update_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emtile_set_change_handler(EMTILE *t, emui_int_f handler)
{
	t->change_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emtile_set_focus_handler(EMTILE *t, emui_int_f_int handler)
{
	t->focus_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emtile_set_key_handler(EMTILE *t, emui_int_f_int handler)
{
	t->key_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emtile_set_properties(EMTILE *t, unsigned properties)
{
	if (properties & ~P_APP_SETTABLE) {
		return -1;
	}

	t->properties |= properties;

	return 0;
}

// -----------------------------------------------------------------------
int emtile_clear_properties(EMTILE *t, unsigned properties)
{
	if (properties & ~P_APP_SETTABLE) {
		return -1;
	}

	t->properties &= ~properties;

	return 0;
}
// -----------------------------------------------------------------------
int emtile_set_name(EMTILE *t, char *name)
{
	free(t->name);
	t->name = strdup(name);

	return 0;
}

// -----------------------------------------------------------------------
int emtile_set_style(EMTILE *t, int style)
{
	t->style = style;
	return 0;
}

// -----------------------------------------------------------------------
static void emtile_child_append(EMTILE *parent, EMTILE *t)
{
	t->parent = parent;
	t->ch_prev = parent->ch_last;
	if (parent->ch_last) {
		parent->ch_last->ch_next = t;
	} else {
		parent->ch_first = t;
	}
	parent->ch_last = t;
}

// -----------------------------------------------------------------------
static void emtile_child_unlink(EMTILE *t)
{
	if (!t->parent) return;

	if (t->ch_prev) {
		t->ch_prev->ch_next = t->ch_next;
	}
	if (t->ch_next) {
		t->ch_next->ch_prev = t->ch_prev;
	}
	if (t == t->parent->ch_first) {
		t->parent->ch_first = t->ch_next;
	}
	if (t == t->parent->ch_last) {
		t->parent->ch_last = t->ch_prev;
	}
}

// -----------------------------------------------------------------------
void emtile_delete(EMTILE *t)
{
	t->properties |= P_DELETED;
}

// -----------------------------------------------------------------------
void _emtile_really_delete(EMTILE *t)
{
	if (!t) return;

	// delete all children first
	EMTILE *next_ch;
	EMTILE *ch = t->ch_first;
	while (ch) {
		next_ch = ch->ch_next;
		_emtile_really_delete(ch);
		ch = next_ch;
	}

	// remove from focus path
	if (t->parent && (t->parent->focus == t)) {
		t->parent->focus = NULL;
	}

	// remove from focus group
	emui_focus_group_unlink(t);

	// remove from focus stack
	emui_focus_stack_delete_tile(t);

	// remove the tile from parent's child list
	emtile_child_unlink(t);

	// delete the tile itself
	delwin(t->ncwin);
	free(t->name);
	if (t->drv->destroy_priv_data) t->drv->destroy_priv_data(t);
	free(t);

	// refocus (there may be a deleted tile on focus path)
	emui_focus_refocus();
}

// -----------------------------------------------------------------------
void emtile_set_ptr(EMTILE *t, void *ptr)
{
	t->ptr = ptr;
}

// -----------------------------------------------------------------------
void * emtile_get_ptr(EMTILE *t)
{
	return t->ptr;
}

// -----------------------------------------------------------------------
void emtile_set_margins(EMTILE *t, int mt, int mb, int ml, int mr)
{
	t->mr = mr;
	t->ml = ml;
	t->mt = mt;
	t->mb = mb;
}

// -----------------------------------------------------------------------
void emtile_geometry_changed(EMTILE *t)
{
	t->geometry_changed = 1;
	if (t->properties & P_GEOM_FORCED) {
		emtile_geometry_changed(t->parent);
	}
}

// -----------------------------------------------------------------------
int emtile_notify_change(EMTILE *t)
{
	t->content_invalid = 0;

	EMTILE *notified_tile = t;
	while (notified_tile) {
		if (notified_tile->change_handler) {
			t->content_invalid = notified_tile->change_handler(notified_tile);
			break;
		}
		notified_tile = notified_tile->parent;
	}

	return t->content_invalid;
}

// vim: tabstop=4 shiftwidth=4 autoindent
