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

#ifndef EMUI_EVENT_H
#define EMUI_EVENT_H

enum event_types {
	EV_QUIT,		// exit the main UI loop
	EV_KEY,			// key pressed
	EV_MOUSE,		// mouse event
	EV_ERROR,		// error
};

struct emui_event {
	int type;		// event type
	int sender;		// event sender (key, error)
	int x, y;
};

struct emui_event * emui_evq_get();
int emui_evq_prepend(struct emui_event *ev);
int emui_evq_append(struct emui_event *ev);
void emui_evq_clear();

#endif

// vim: tabstop=4 shiftwidth=4 autoindent
