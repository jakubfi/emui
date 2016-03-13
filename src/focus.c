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
		nfi->prev = NULL;
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
void emui_focus(EMTILE *t)
{
	if (!t) return;

	EMTILE *last_focus = focus_stack ? focus_stack->t : NULL;

	// search for a focus path down to the (possibly) interactive tile
	EMTILE *f = _focus_down(t);
	// fill the focus path to the root
	_focus_up(f);
	if (!IS_INTERACTIVE(t)) {
		_focus_stack_put(t);
	}

	// call focus handlers
	if (last_focus && last_focus->focus_handler) {
		last_focus->focus_handler(last_focus, 0);
	}
	if (last_focus && last_focus->drv->focus_handler) {
		last_focus->drv->focus_handler(last_focus, 0);
	}
	if (t->focus_handler) {
		t->focus_handler(t, 1);
	}
	if (t->drv->focus_handler) {
		t->drv->focus_handler(t, 1);
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
