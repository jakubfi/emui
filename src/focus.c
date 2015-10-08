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
static int distance(int x1, int y1, int x2, int y2)
{
	int distance = (x1-x2) * (x1-x2) + (y1-y2) * (y1-y2);
	return distance;
}

// -----------------------------------------------------------------------
int emui_focus_physical_neighbour(struct emui_tile *t, int dir)
{
	struct emui_tile *f = t->fg->fg_first;
	struct emui_tile *match = t;
	int min_dist = INT_MAX;

	while (f) {
		if (f->properties & P_INTERACTIVE) {
			int dist = distance(t->x, t->y, f->x, f->y);
			int cl, cg;

			// TODO: find more ergonomic policy for switching up/down/left/right

			switch (dir) {
				case FC_UP: cl = f->y; cg = t->y; break;
				case FC_DOWN: cl = t->y; cg = f->y; break;
				case FC_LEFT: cl = f->x; cg = t->x; break;
				case FC_RIGHT: cl = t->x; cg = f->x; break;
				default: return 0; // unknown or incompatibile direction
			}

			if (cl < cg) {
				if (dist < min_dist) {
					min_dist = dist;
					match = f;
				}
			}
		}
		f = f->fg_next;
	}

	if (match != t) emui_focus(match);

	return 0;
}

// -----------------------------------------------------------------------
int emui_focus_list_neighbour(struct emui_tile *t, int dir)
{
	struct emui_tile *next = t;

	while (1) {
		switch (dir) {
			case FC_BEG: next = t->fg->fg_first; dir = FC_NEXT; break;
			case FC_END: next = t->fg->fg_last; dir = FC_PREV; break;
			case FC_NEXT: next = next->fg_next; break;
			case FC_PREV: next = next->fg_prev; break;
			default: return 0; // unknown or incompatibile direction
		}

		if (next) {
			if (next->properties & P_INTERACTIVE) {
				// got a tile that can be focused
				emui_focus(next);
				break;
			} else if (next == t) {
				// we've looped over, nothing to do
				break;
			}
		} else {
			// hit the boundary, start from the other and
			dir = (dir == FC_NEXT) ? FC_BEG : FC_END;
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
void emui_focus(struct emui_tile *t)
{
	// go down the tree if tile is not a leaf (focus needs to cover full tile path)
	while (t->ch_first) {
		if (t->focus) {
			 t = t->focus;
		} else {
			t = t->ch_first;
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
