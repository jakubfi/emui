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

// -----------------------------------------------------------------------
void emui_tile_update_external_geometry(struct emui_tile *t)
{
	// select which parent's geometry tile's gonna use:
	// screen, parent's external or parent's internal
	struct emui_geom pg;
	if (t->properties & P_FLOAT) {
		pg.x = 0;
		pg.y = 0;
		pg.h = LINES;
		pg.w = COLS;
	} else if (t->properties & P_IGNORE_MARGINS) {
		pg = t->parent->e;
	} else {
		pg = t->parent->i;
	}

	// (1) 'floating' property overrides everything
	if (t->properties & P_FLOAT) {
		// (1.1) still, respect P_MAXIMIZE
		if (t->properties & P_MAXIMIZED) {
			t->e = pg;
		// (1.2) or just set the geometry by centering the floating window
		} else {
			t->e.w = t->r.w;
			t->e.h = t->r.h;
			t->e.x = (pg.w - t->r.w) / 2;
			t->e.y = (pg.h - t->r.h) / 2;
			if (t->e.x < 0) {
				t->e.x = 0;
			}
			if (t->e.y < 0) {
				t->e.y = 0;
			}
		}
		t->properties &= ~P_HIDDEN;
	// (2) respect parent's choice to hide the child
	} else if (t->properties & P_GEOM_FORCED) {
		if (t->properties & P_HIDDEN) {
			return;
		// (2.5) but also hide the child if parent is hidden
		} else if (t->parent->properties & P_HIDDEN) {
			t->properties |= P_HIDDEN;
			return;
		}
	// (3) hide the child if parent is hidden
	} else if (t->parent->properties & P_HIDDEN) {
		t->properties |= P_HIDDEN;
		return;
	// (4) respect P_MAXIMIZE
	} else if (t->properties & P_MAXIMIZED) {
		t->e = pg;
		t->properties &= ~P_HIDDEN;
	// (5) calculate own external geometry
	} else {
		t->e = t->r;
		t->e.x += pg.x;
		t->e.y += pg.y;
	}

	// fit tile width to parent's width
	if (t->e.w + t->e.x > pg.w + pg.x) {
		t->e.w = pg.w - (t->e.x - pg.x);
	}
	// fit tile to parent's height
	if (t->e.h + t->e.y > pg.h + pg.y) {
		t->e.h = pg.h - (t->e.y - pg.y);
	}
	// hide tile if outside parent's area
	if ((t->e.x >= pg.x + pg.w) || (t->e.y >= pg.y + pg.h)) {
		t->properties |= P_HIDDEN;
	} else {
		t->properties &= ~P_HIDDEN;
	}
}

// -----------------------------------------------------------------------
void emui_tile_update_internal_geometry(struct emui_tile *t)
{
	// hide if no space for internal geometry
	if ((t->e.w <= t->ml+t->mr) || (t->e.h <= t->mt+t->mb)) {
		t->properties |= P_HIDDEN;
		return;
	} else {
		t->properties &= ~P_HIDDEN;
	}

	// set internal tile geometry
	t->i.x = t->e.x + t->ml;
	t->i.y = t->e.y + t->mt;
	t->i.w = t->e.w - t->ml - t->mr;
	t->i.h = t->e.h - t->mt - t->mb;
}

// -----------------------------------------------------------------------
void emui_tile_update_geometry(struct emui_tile *t)
{
	if (t->parent) {
		// set external geometry
		emui_tile_update_external_geometry(t);

		// if tile is still visible, set internal geometry
		if (!(t->properties & P_HIDDEN)) {
			emui_tile_update_internal_geometry(t);

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
				emuifillbg(t, t->style);
			}
		}
	}

	// do tile-specific geometry updates
	if (t->drv->update_children_geometry) t->drv->update_children_geometry(t);

	t->geometry_changed = 0;
}

// -----------------------------------------------------------------------
int emui_tile_draw(struct emui_tile *t)
{
	// tile is hidden or has no canvas, nothing to do
	if (t->properties & (P_HIDDEN | P_NOCANVAS)) {
		return 1;
	}

	// if tile accepts content updates and user specified a handler,
	// then update content before tile is drawn
	if (t->accept_updates && t->user_update_handler) {
		t->user_update_handler(t);
	}

	// draw the tile
	if (t->drv->draw) t->drv->draw(t);

	// update ncurses window, but don't output,
	// we will doupdate() in the main loop
	wnoutrefresh(t->ncwin);

	return 0;
}

// -----------------------------------------------------------------------
struct emui_tile * emui_tile_create(struct emui_tile *parent, int id, struct emui_tile_drv *drv, int x, int y, int w, int h, int mt, int mb, int ml, int mr, char *name, int properties)
{
	struct emui_tile *t = calloc(1, sizeof(struct emui_tile));
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
	t->id = id;
	t->accept_updates = 1;
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

	emui_tile_child_append(parent, t);
	emui_focus_group_add(parent, t);
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
int emui_tile_set_update_handler(struct emui_tile *t, emui_handler_f handler)
{
	t->user_update_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emui_tile_set_change_handler(struct emui_tile *t, emui_handler_f handler)
{
	t->user_change_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emui_tile_set_focus_handler(struct emui_tile *t, emui_focus_handler_f handler)
{
	t->user_focus_handler = handler;
	return 0;
}

// -----------------------------------------------------------------------
int emui_tile_set_key_handler(struct emui_tile *t, emui_key_handler_f handler)
{
	t->user_key_handler = handler;
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
int emui_tile_clear_properties(struct emui_tile *t, unsigned properties)
{
	if (properties & ~P_USER_SETTABLE) {
		return -1;
	}

	t->properties &= ~properties;

	return 0;
}
// -----------------------------------------------------------------------
int emui_tile_set_name(struct emui_tile *t, char *name)
{
	free(t->name);
	t->name = strdup(name);

	return 0;
}

// -----------------------------------------------------------------------
int emui_tile_set_style(struct emui_tile *t, int style)
{
	t->style = style;
	return 0;
}

// -----------------------------------------------------------------------
void emui_tile_child_append(struct emui_tile *parent, struct emui_tile *t)
{
	t->parent = parent;
	t->prev = parent->ch_last;
	if (parent->ch_last) {
		parent->ch_last->next = t;
	} else {
		parent->ch_first = t;
	}
	parent->ch_last = t;
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
	if (t->drv->destroy_priv_data) t->drv->destroy_priv_data(t);
	free(t->name);

	// free ncurses windows
	delwin(t->ncwin);

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

	// remove from focus stack
	emui_focus_stack_delete_tile(t);

	// remove from focus path
	if (t->parent && (t->parent->focus == t)) {
		t->parent->focus = NULL;
	}

	// refocus (there may be a deleted tile on focus path)
	emui_focus_refocus();

	// delete the tile itself
	emui_tile_free(t);
}

// -----------------------------------------------------------------------
static void emui_tree_set_properties(struct emui_tile *t, int property)
{
	t->properties |= property;
	t = t->ch_first;
	while (t) {
		emui_tree_set_properties(t, property);
		t = t->next;
	}
}

// -----------------------------------------------------------------------
static void emui_tree_clear_properties(struct emui_tile *t, int property)
{
	t->properties &= ~property;
	t = t->ch_first;
	while (t) {
		emui_tree_clear_properties(t, property);
		t = t->next;
	}
}

// -----------------------------------------------------------------------
void emui_tile_inverse(struct emui_tile *t, int inv)
{
	if (inv) {
		emui_tree_set_properties(t, P_INVERSE);
	} else {
		emui_tree_clear_properties(t, P_INVERSE);
	}
}

// -----------------------------------------------------------------------
void emui_tile_set_id(struct emui_tile *t, int id)
{
	t->id = id;
}

// -----------------------------------------------------------------------
int emui_tile_get_id(struct emui_tile *t)
{
	return t->id;
}

// -----------------------------------------------------------------------
void emui_tile_set_margins(struct emui_tile *t, int mt, int mb, int ml, int mr)
{
	t->mr = mr;
	t->ml = ml;
	t->mt = mt;
	t->mb = mb;
}

// -----------------------------------------------------------------------
void emui_tile_geometry_changed(struct emui_tile *t)
{
	t->geometry_changed = 1;
	if (t->properties & P_GEOM_FORCED) {
		emui_tile_geometry_changed(t->parent);
	}
}

// -----------------------------------------------------------------------
int emui_tile_changed(struct emui_tile *t)
{
	if (t->user_change_handler && t->user_change_handler(t)) {
		t->content_invalid = 1;
	} else {
		t->content_invalid = 0;
	}

	return t->content_invalid;
}

// vim: tabstop=4 shiftwidth=4 autoindent
