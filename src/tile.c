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
#include <limits.h>

#include "dbg.h"
#include "tile.h"
#include "event.h"
#include "style.h"
#include "focus.h"
#include "print.h"

static void emtile_child_append(EMTILE *parent, EMTILE *t);

// -----------------------------------------------------------------------
static void emtile_fit_parent(EMTILE *t)
{
	// P_FLOAT property overrides everything - unhide, because it should be shown regardles of parent decisions
	if (t->properties & P_FLOAT) {
		EDBG(t, 2, "unhidding tile (is floating)");
		t->properties &= ~P_HIDDEN;
	// hide the child if parent is hidden, even if parent decides to show the child
	} else if (t->parent->properties & P_HIDDEN) {
		t->properties |= P_HIDDEN;
		EDBG(t, 2, "hiding tile (parent is hidden)");
	// respect parent's choice regarding children geometry and visibility
	} else if (t->properties & P_GEOM_FORCED) {
		EDBG(t, 2, "aborting fit of tile (geometry is forced by the parent)");
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

	// respect fill
	if (t->properties & P_HFILL) {
		t->e.w = t->pg->w - t->e.x;
	}
	if (t->properties & P_VFILL) {
		t->e.h = t->pg->h - t->e.y;
	}

	// respect maximization
	if (t->properties & P_HMAXIMIZE) {
		t->e.x = t->pg->x;
		t->e.w = t->pg->w;
	}
	if (t->properties & P_VMAXIMIZE) {
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
		EDBG(t, 2, "hidding tile (doesn't fit)");
	} else {
		if (t->parent->properties & P_HIDDEN) {
			EDBG(t, 2, "not unhidding tile (parent is hidden)");
		} else {
			EDBG(t, 2, "unhidding tile (fits)");
			t->properties &= ~P_HIDDEN;
		}
	}
	EDBG(t, 2, "post emtile_fit_parent() geometry: req: %i,%i,%i,%i, ext: %i,%i,%i,%i", t->r.x, t->r.y, t->r.w, t->r.h, t->e.x, t->e.y, t->e.w, t->e.h);
}

// -----------------------------------------------------------------------
static void emtile_fit_interior(EMTILE *t)
{
	// set internal tile geometry
	t->i = t->e;
	t->i.x += t->ml;
	t->i.y += t->mt;
	t->i.w -= t->ml + t->mr;
	t->i.h -= t->mt + t->mb;
	EDBG(t, 2, "post emtile_fit_interior() int geometry: %i,%i,%i,%i", t->i.x, t->i.y, t->i.w, t->i.h);
}

// -----------------------------------------------------------------------
void emtile_fit(EMTILE *t)
{
	if (t->parent) {
		EDBG(t, 1, "fitting tile");

		emtile_fit_parent(t);
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

	EDBG(t, 1, "doing tile specific geometry update");

	// do tile-specific geometry updates
	if (t->drv->update_children_geometry) {
		t->drv->update_children_geometry(t);
	}

	t->geometry_changed = 0;
}

// -----------------------------------------------------------------------
void emtile_draw(EMTILE *t)
{
	// tile is hidden or has no canvas, nothing to do
	if (t->properties & (P_HIDDEN | P_NOCANVAS)) {
		return;
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
}

// -----------------------------------------------------------------------
static int emtile_focus_keys(EMTILE *fg, int key)
{
	EMTILE *t = fg->fg_first;

	while (t) {
		if (t->key == key) {
			EDBG(t, 3, "Focus key %c matches", key);
			emui_focus(t);
			return E_HANDLED;
		}
		t = t->fg_next;
	}

	return E_UNHANDLED;
}

// -----------------------------------------------------------------------
static int _overlap(int b1, int e1, int b2, int e2)
{
#define MIN(a, b) ((a) < (b) ? a : b)
#define MAX(a, b) ((a) > (b) ? a : b)
	return MIN(e1, e2) - MAX(b1, b2);
#undef MIN
#undef MAX
}

// -----------------------------------------------------------------------
static int _distance(int x1, int y1, int x2, int y2)
{
	int distance = (x1-x2) * (x1-x2) + (y1-y2) * (y1-y2);
	return distance;
}

// -----------------------------------------------------------------------
EMTILE * emtile_get_physical_neighbour(EMTILE *fg, int dir, unsigned prop_match, unsigned prop_nomatch)
{
	EMTILE *t = emui_subfocus_get(fg);
	EMTILE *f = fg->fg_first;
	EMTILE *match = t;

	int dd; // directional distance (distance in the move direction)
	int ovrl, ovrl_max = 0; // overlap region
	int dist, dist_min = INT_MAX;

	while (f) {
		if ((f->properties & (prop_match | prop_nomatch)) == prop_match) {
			EDBG(f, 3, "considering phys. neigh.: %i,%i (%i,%i)", f->i.x, f->i.y, f->i.h, f->i.w);
			switch (dir) {
				case FC_ABOVE:
					dd = t->i.y - f->i.y - f->i.h;
					ovrl = _overlap(t->i.x, t->i.x + t->i.w, f->i.x, f->i.x + f->i.w);
					break;
				case FC_BELOW:
					dd = f->i.y - t->i.y - t->i.h;
					ovrl = _overlap(t->i.x, t->i.x + t->i.w, f->i.x, f->i.x + f->i.w);
					break;
				case FC_LEFT:
					dd = t->i.x - f->i.x - f->i.w;
					ovrl = _overlap(t->i.y, t->i.y + t->i.h, f->i.y, f->i.y + f->i.h);
					break;
				case FC_RIGHT:
					dd = f->i.x - t->i.x - t->i.w;
					ovrl = _overlap(t->i.y, t->i.y + t->i.h, f->i.y, f->i.y + f->i.h);
					break;
				default:
					return t; // unknown or incompatibile direction
			}

			dist = _distance(t->i.x+t->i.w/2, t->i.y+t->i.h/2, f->i.x+f->i.w/2, f->i.y+f->i.h/2);

			// tile has to be:
			//  * other than the current tile
			//  * further in the move direction
			//  * "overlapping" with current tile in axis perpendicural to movement
			if ((f != t) && (dd >= 0) && (ovrl > 0)) {
				// we search for the closest tile
				if (dist <= dist_min) {
					// we search for a tile that "overlaps" the most with the current one
					if (ovrl >= ovrl_max) {
						ovrl_max = ovrl/2; // /2 => less impact on decision
						dist_min = dist;
						match = f;
						EDBG(match, 3, "best so far");
					}
				}
			}
		}
		f = f->fg_next;
	}

	return match;
}

// -----------------------------------------------------------------------
EMTILE * emtile_get_list_neighbour(EMTILE *fg, int dir, unsigned prop_match, unsigned prop_nomatch)
{
	EMTILE *t = emui_subfocus_get(fg);
	EMTILE *next = t;

	// for cases when we start searching at t = t->fg->fg_first
	int first_item = 1;

	while (1) {
		switch (dir) {
			case FC_FIRST:
				next = fg->fg_first;
				dir = FC_NEXT;
				break;
			case FC_LAST:
				next = fg->fg_last;
				dir = FC_PREV;
				break;
			case FC_NEXT:
				next = next->fg_next;
				break;
			case FC_PREV:
				next = next->fg_prev;
				break;
			default:
				return t; // unknown or incompatibile direction
		}

		if (next) {
			EDBG(next, 3, "considering list neigh.");
			if ((next->properties & (prop_match | prop_nomatch)) == prop_match) {
				// got a tile that can be focused
				EDBG(next, 3, "using");
				return next;
			} else if (next == t) {
				if (first_item) {
					first_item = 0;
				} else {
					// we've looped over, nothing to do
					break;
				}
			}
		} else {
			// hit the boundary, start from the other end
			dir = (dir == FC_NEXT) ? FC_FIRST : FC_LAST;
		}
	}

	return t;
}

// -----------------------------------------------------------------------
static int emtile_neighbour_focus(EMTILE *fg, int key)
{
	EMTILE *focus = NULL;
	int ret = E_HANDLED;
	int param_nomatch = P_HIDDEN;

	if (fg->drv->scroll_handler) {
		param_nomatch = P_NONE;
	}

	switch (key) {
		case 9: // TAB
			focus = emtile_get_list_neighbour(fg, FC_NEXT, P_INTERACTIVE, param_nomatch);
			break;
		case KEY_BTAB:
			focus = emtile_get_list_neighbour(fg, FC_PREV, P_INTERACTIVE, param_nomatch);
			break;
		case KEY_UP:
			focus = emtile_get_physical_neighbour(fg, FC_ABOVE, P_INTERACTIVE, param_nomatch);
			break;
		case KEY_DOWN:
			focus = emtile_get_physical_neighbour(fg, FC_BELOW, P_INTERACTIVE, param_nomatch);
			break;
		case KEY_LEFT:
			focus = emtile_get_physical_neighbour(fg, FC_LEFT, P_INTERACTIVE, param_nomatch);
			break;
		case KEY_RIGHT:
			focus = emtile_get_physical_neighbour(fg, FC_RIGHT, P_INTERACTIVE, param_nomatch);
			break;
		default:
			ret = E_UNHANDLED;
			break;
	}

	EDBG(focus, 2, "after neighbour focus");

	// if current focus is hidden and focus group allow scrolling, do it.
	if (focus && (focus->properties & P_HIDDEN) && fg->drv->scroll_handler) {
		EDBG(focus, 2, "can scroll to show");
		fg->drv->scroll_handler(fg, focus);
	}

	emui_focus(focus);

	return ret;
}

// -----------------------------------------------------------------------
int emtile_event(EMTILE *t, struct emui_event *ev)
{
	// try running app key handler
	if ((ev->type == EV_KEY) && t->key_handler && (t->key_handler(t, ev->sender) == E_HANDLED)) {
		return E_HANDLED;
	}

	// run tile's own event handler
	if (t->drv->event_handler && (t->drv->event_handler(t, ev) == E_HANDLED)) {
		return E_HANDLED;
	}

	// try focus keys for focus groups
	if ((t->properties & P_FOCUS_GROUP) && (ev->type == EV_KEY)) {
		if (emtile_focus_keys(t, ev->sender) == E_HANDLED) {
			return E_HANDLED;
		}
		if (emtile_neighbour_focus(t, ev->sender) == E_HANDLED) {
			return E_HANDLED;
		}
	}

	return E_UNHANDLED;
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

	EDBG(t, 0, "Added tile");

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
	// changing geometry parent requires recalculating geometry
	emtile_geometry_changed(t);
}

// -----------------------------------------------------------------------
void emtile_set_float_parent(EMTILE *t, EMTILE *p)
{
	t->float_parent = p;
}

// -----------------------------------------------------------------------
void emtile_set_focus_key(EMTILE *t, int key)
{
	t->key = key;
}

// -----------------------------------------------------------------------
void emtile_set_update_handler(EMTILE *t, emui_int_f handler)
{
	t->update_handler = handler;
}

// -----------------------------------------------------------------------
void emtile_set_change_handler(EMTILE *t, emui_int_f handler)
{
	t->change_handler = handler;
}

// -----------------------------------------------------------------------
void emtile_set_key_handler(EMTILE *t, emui_int_f_int handler)
{
	t->key_handler = handler;
}

// -----------------------------------------------------------------------
int emtile_set_properties(EMTILE *t, unsigned properties)
{
	if (properties & ~P_APP_SETTABLE) {
		return -1;
	}

	t->properties |= properties;

	return E_OK;
}

// -----------------------------------------------------------------------
int emtile_clear_properties(EMTILE *t, unsigned properties)
{
	if (properties & ~P_APP_SETTABLE) {
		return -1;
	}

	t->properties &= ~properties;

	return E_OK;
}
// -----------------------------------------------------------------------
int emtile_set_name(EMTILE *t, char *name)
{
	free(t->name);
	t->name = strdup(name);
	if (!t->name) {
		return E_ALLOC;
	}

	return E_OK;
}

// -----------------------------------------------------------------------
void emtile_set_style(EMTILE *t, int style)
{
	t->style = style;
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
	EDBG(t, 0, "Tile marked for deletion");
	t->properties |= P_DELETED;
}

// -----------------------------------------------------------------------
void _emtile_really_delete(EMTILE *t)
{
	if (!t) return;

	EDBG(t, 0, "Physically deleting tile");

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
