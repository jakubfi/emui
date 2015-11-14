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

#include "tile.h"
#include "focus.h"

struct focus_item {
	EMTILE *t;
	struct focus_item *prev;
};

struct focus_item *focus_stack;
static EMTILE *focus;

// -----------------------------------------------------------------------
static void _focus_stack_put(EMTILE *t)
{
	struct focus_item *fi = focus_stack;
	struct focus_item *nfi = NULL;
	struct focus_item *next = NULL;

	// check if tile is already on stack
	while (fi) {
		if (fi->t == t) {
			nfi = fi;
			if (next) {
				next->prev = fi->prev;
			}
			break;
		}
		next = fi;
		fi = fi->prev;
	}

	// if not, create new focus item
	if (!nfi) {
		nfi = malloc(sizeof(struct focus_item));
		nfi->t = t;
	}

	// put on stack
	if (nfi != focus_stack) {
		nfi->prev = focus_stack;
		focus_stack = nfi;
	}
}

// -----------------------------------------------------------------------
void emui_focus_stack_delete_tile(EMTILE *t)
{
	struct focus_item *fi = focus_stack;
	struct focus_item *next = NULL;

	while (fi) {
		if (fi->t == t) {
			if (next) {
				next->prev = fi->prev;
			} else {
				focus_stack = fi->prev;
			}
			free(fi);
			return;
		}
		next = fi;
		fi = fi->prev;
	}
}

// -----------------------------------------------------------------------
void emui_focus_stack_drop()
{
	struct focus_item *fi = focus_stack;
	struct focus_item *fi_prev = NULL;
	while (fi){
		fi_prev = fi->prev;
		free(fi);
		fi = fi_prev;
	}
}

// -----------------------------------------------------------------------
static int _overlap(int b1, int e1, int b2, int e2)
{
#define MIN(a, b) ((a) < (b) ? a : b)
#define MAX(a, b) ((a) > (b) ? a : b)
	return MIN(e1, e2) - MAX(b1, b2);
}

// -----------------------------------------------------------------------
static int _distance(int x1, int y1, int x2, int y2)
{
	int distance = (x1-x2) * (x1-x2) + (y1-y2) * (y1-y2);
	return distance;
}

// -----------------------------------------------------------------------
static void _focus_up(EMTILE *t)
{
	// set new focus path
	focus = t;
	while (t && t->parent) {
		t->parent->focus = t;
		t = t->parent;
	}
}

// -----------------------------------------------------------------------
int emui_focus_physical_neighbour(EMTILE *t, int dir)
{
	EMTILE *f = t->fg->fg_first;
	EMTILE *match = t;

	int dd; // directional distance (distance in the move direction)
	int ovrl, ovrl_max = 0; // overlap region
	int dist, dist_min = INT_MAX;

	while (f) {
		if (IS_INTERACTIVE(f)) {

			dist = _distance(t->i.x+t->i.w/2, t->i.y+t->i.h/2, f->i.x+f->i.w/2, f->i.y+f->i.h/2);

			switch (dir) {
				case FC_UP:
					dd = t->i.y - f->i.y - f->i.h;
					ovrl = _overlap(t->i.x, t->i.x + t->i.w, f->i.x, f->i.x + f->i.w);
					break;
				case FC_DOWN:
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
					return 0; // unknown or incompatibile direction
			}

			// tile has to be:
			//  * other than the current tile
			//  * further in the move direction
			//  * "overlapping" with current tile in axis perpendicural to movement
			if ((f != t) && (dd >= 0) && (ovrl > 0)) {
				// we search for the closest tile
				if (dist <= dist_min) {
					// we search for a tile that "overlaps" the most with the current one
					if (ovrl >= ovrl_max) {
						ovrl_max = ovrl/2; // /2 = less impact on decision
						dist_min = dist;
						match = f;
					}
				}
			}
		}
		f = f->fg_next;
	}

	if (match != t) _focus_up(match);

	return 0;
}

// -----------------------------------------------------------------------
int emui_focus_list_neighbour(EMTILE *t, int dir)
{
	EMTILE *next = t;
	// for cases when we start searching at t = t->fg->fg_first
	int first_item = 1;

	while (1) {
		switch (dir) {
			case FC_BEG: next = t->fg->fg_first; dir = FC_NEXT; break;
			case FC_END: next = t->fg->fg_last; dir = FC_PREV; break;
			case FC_NEXT: next = next->fg_next; break;
			case FC_PREV: next = next->fg_prev; break;
			default: return 0; // unknown or incompatibile direction
		}

		if (next) {
			if (IS_INTERACTIVE(next)) {
				// got a tile that can be focused
				_focus_up(next);
				break;
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
			dir = (dir == FC_NEXT) ? FC_BEG : FC_END;
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
static EMTILE * _focus_down(EMTILE *t)
{
	// try to follow old focus first
	if (t->focus) {
		return _focus_down(t->focus);
	// then go for a first focusable tile
	} else if (IS_INTERACTIVE(t)) {
		return t;
	// then if this is a focus group...
	} else if (t->properties & P_FOCUS_GROUP) {
		// ...that has members, start searching the focus group
		if (t->fg_first) {
			return _focus_down(t->fg_first);
		// ... with no members, stop here
		} else {
			return t;
		}
	// continue searching the focus group
	} else if (t->fg_next) {
		return _focus_down(t->fg_next);
	// lastly, return anything
	} else {
		return t;
	}
}

// -----------------------------------------------------------------------
void emui_focus(EMTILE *t)
{
	if (!t) return;

	EMTILE *last_focus = focus_stack ? focus_stack->t : NULL;

	// search for a focus path down to the (possibly) interactive tile
	EMTILE *f = _focus_down(t);
	// fill the focus path to the root
	_focus_up(f);
	_focus_stack_put(t);

	// call focus handlers
	if (last_focus && last_focus->focus_handler) {
		last_focus->focus_handler(last_focus, 0);
	}
	if (t->focus_handler) {
		t->focus_handler(t, 1);
	}

	// getting focus may change tile's geometry (P_FLOAT)
	emtile_geometry_changed(t);
	if (last_focus) {
		emtile_geometry_changed(last_focus);
	}
}

// -----------------------------------------------------------------------
void emui_focus_refocus()
{
	if (focus_stack) {
		emui_focus(focus_stack->t);
	}
}

// -----------------------------------------------------------------------
int emui_is_focused(EMTILE *t)
{
	if (t == focus) return 1;
	else return 0;
}

// -----------------------------------------------------------------------
int emui_has_focus(EMTILE *t)
{
	EMTILE *f = focus;
	while (f) {
		if (t == f) return 1;
		f = f->parent;
	}
	return 0;
}

// -----------------------------------------------------------------------
EMTILE * emui_focus_get()
{
	return focus;
}

// -----------------------------------------------------------------------
int emui_focus_group_add(EMTILE *parent, EMTILE *t)
{
	EMTILE *fg = NULL;

	// if parent is a focus group
	if (parent->properties & P_FOCUS_GROUP) {
		fg = parent;
	// if parent is not a focus group, but has focus group set
	} else if (parent->fg) {
		fg = parent->fg;
	}

	// there is a focus group to be in
	if (fg) {
		t->fg = fg;
		t->fg_prev = fg->fg_last;
		if (fg->fg_last) {
			fg->fg_last->fg_next = t;
		} else {
			fg->fg_first = t;
		}
		fg->fg_last = t;
	}

	return 0;
}

// -----------------------------------------------------------------------
void emui_focus_group_unlink(EMTILE *t)
{
	if (!t) return;

	EMTILE *fg = t->fg;

	if (!fg) return;

	if (t->fg_prev) {
		t->fg_prev->fg_next = t->fg_next;
	}
	if (t->fg_next) {
		t->fg_next->fg_prev = t->fg_prev;
	}
	if (t == fg->fg_first) {
		fg->fg_first = t->fg_next;
	}
	if (t == fg->fg_last) {
		fg->fg_last = t->fg_prev;
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
