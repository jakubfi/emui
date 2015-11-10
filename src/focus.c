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

static struct emui_tile *focus;

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
static void _focus_up(struct emui_tile *t)
{
	// set new focus path
	focus = t;
	while (t && t->parent) {
		t->parent->focus = t;
		t = t->parent;
	}
}

// -----------------------------------------------------------------------
int emui_focus_physical_neighbour(struct emui_tile *t, int dir)
{
	struct emui_tile *f = t->fg->fg_first;
	struct emui_tile *match = t;

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
int emui_focus_list_neighbour(struct emui_tile *t, int dir)
{
	struct emui_tile *next = t;
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
struct emui_tile * _focus_down(struct emui_tile *t)
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
void emui_focus(struct emui_tile *t)
{
	if (!t) return;
	// search for a focus path down to the (possibly) interactive tile
	struct emui_tile *f = _focus_down(t);
	// fill the focus path to the root
	_focus_up(f);
	emui_tile_geometry_changed(t);
}

// -----------------------------------------------------------------------
int emui_is_focused(struct emui_tile *t)
{
	if (t == focus) return 1;
	else return 0;
}

// -----------------------------------------------------------------------
int emui_has_focus(struct emui_tile *t)
{
	struct emui_tile *f = focus;
	while (f) {
		if (t == f) return 1;
		f = f->parent;
	}
	return 0;
}

// -----------------------------------------------------------------------
struct emui_tile * emui_focus_get()
{
	return focus;
}

// -----------------------------------------------------------------------
int emui_focus_group_add(struct emui_tile *parent, struct emui_tile *t)
{
	struct emui_tile *fg = NULL;;

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
void emui_focus_group_unlink(struct emui_tile *t)
{
	if (!t) return;

	struct emui_tile *fg = t->fg;

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
