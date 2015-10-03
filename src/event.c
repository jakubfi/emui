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

#include "event.h"

struct event_elem {
	struct emui_event *ev;
	struct event_elem *next;
};

static struct event_elem *evq_head;
static struct event_elem *evq_tail;

// -----------------------------------------------------------------------
struct emui_event * emui_evq_get()
{
	struct emui_event *ev = NULL;
	struct event_elem *eve;

	if (evq_head) {
		ev = evq_head->ev;
		eve = evq_head;
		evq_head = evq_head->next;
		free(eve);
	}

	if (!evq_head) {
		evq_tail = NULL;
	}

	return ev;
}

// -----------------------------------------------------------------------
int emui_evq_prepend(struct emui_event *ev)
{
	struct event_elem *eve = malloc(sizeof(struct event_elem));

	if (!eve) return -1;

	eve->ev = ev;

	if (evq_head) {
		eve->next = evq_head;
	} else {
		eve->next = NULL;
		evq_tail = eve;
	}

	evq_head = eve;

	return 0;
}

// -----------------------------------------------------------------------
int emui_evq_append(struct emui_event *ev)
{
	struct event_elem *eve = malloc(sizeof(struct event_elem));

	if (!eve) return -1;

	eve->ev = ev;
	eve->next = NULL;

	if (evq_tail) {
		evq_tail->next = eve;
	} else {
		evq_head = eve;
	}

	evq_tail = eve;

	return 0;
}

// -----------------------------------------------------------------------
void emui_evq_clear()
{
	struct emui_event *ev;

	do {
		ev = emui_evq_get();
		free(ev);
	} while (ev);

}

// vim: tabstop=4 shiftwidth=4 autoindent
