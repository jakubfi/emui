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
#include <ctype.h>

#include "tile.h"
#include "event.h"
#include "style.h"
#include "print.h"
#include "focus.h"

struct lineedit {
	int type;
	int in_edit;
	int inserting;
	int pos;
	int maxlen;
	char *buf;
};

// -----------------------------------------------------------------------
void emui_lineedit_draw(struct emui_tile *t)
{
	struct lineedit *le = t->priv_data;

	int style = S_TEXT_BOLD;
	if (le->in_edit) {
		style = S_TEXT_EDITED;
	} else if (emui_has_focus(t) ) {
		style = S_TEXT_FOCUSED;
	}

	emuixyprt(t, 0, 0, style, "%s", le->buf);
	wmove(t->ncwin, 0, le->pos);
}

// -----------------------------------------------------------------------
void emui_lineedit_debug(struct emui_tile *t)
{
}

// -----------------------------------------------------------------------
int emui_lineedit_update_geometry(struct emui_tile *t)
{
	return 0;
}

// -----------------------------------------------------------------------
static int le_handle_non_edit(struct emui_tile *t, struct emui_event *ev)
{
	struct lineedit *le = t->priv_data;

	switch (ev->data.key) {
		case 10:
			le->in_edit = 1;
			curs_set(1);
			break;
		default:
			// event has not been handled
			return 1;
	}

	return 0;
}

// -----------------------------------------------------------------------
static int le_handle_edit(struct emui_tile *t, struct emui_event *ev)
{
	struct lineedit *le = t->priv_data;

	switch (ev->data.key) {
		case KEY_ENTER:
		case '\n':
		case '\r':
			le->in_edit = 0;
			le->pos = 0;
			curs_set(0);
			break;
		case 27:
			le->in_edit = 0;
			le->pos = 0;
			curs_set(0);
			break;
		case KEY_LEFT:
			if (le->pos > 0) le->pos -= 1;
			break;
		case KEY_RIGHT:
			if (le->pos < strlen(le->buf)) le->pos += 1;
			break;
		case KEY_HOME:
			le->pos = 0;
			break;
		case KEY_END:
			le->pos = strlen(le->buf);
			break;
		case KEY_BACKSPACE:
		case 127:
			memmove(le->buf+le->pos-1, le->buf+le->pos, strlen(le->buf)-le->pos);
			le->pos -= 1;
			le->buf[strlen(le->buf)-1] = '\0';
			break;
		case KEY_DC:
			memmove(le->buf+le->pos, le->buf+le->pos+1, strlen(le->buf)-le->pos-1);
			le->buf[strlen(le->buf)-1] = '\0';
			break;
		default:
			if (isprint(ev->data.key)) {
				if (le->pos < le->maxlen) {
					memmove(le->buf+le->pos+1, le->buf+le->pos, strlen(le->buf)-le->pos);
					le->buf[le->pos++] = ev->data.key;
				}
			}
	}

	// eat all unhandled events
	return 0;
}

// -----------------------------------------------------------------------
int emui_lineedit_event_handler(struct emui_tile *t, struct emui_event *ev)
{
	struct lineedit *le = t->priv_data;

	if (ev->type == EV_KEY) {
		if (le->in_edit) {
			return le_handle_edit(t, ev);
		} else {
			return le_handle_non_edit(t, ev);
		}
	}

	// event has not been handled
	return 1;
}

// -----------------------------------------------------------------------
void emui_lineedit_destroy_priv_data(struct emui_tile *t)
{
	struct lineedit *le = t->priv_data;
	free(le->buf);
	free(le);
}

// -----------------------------------------------------------------------
struct emui_tile_drv emui_lineedit_drv = {
	.draw = emui_lineedit_draw,
	.debug = emui_lineedit_debug,
	.update_geometry = emui_lineedit_update_geometry,
	.event_handler = emui_lineedit_event_handler,
	.destroy_priv_data = emui_lineedit_destroy_priv_data,
};

// -----------------------------------------------------------------------
struct emui_tile * emui_lineedit_new(struct emui_tile *parent, int x, int y, int w, int maxlen, int type)
{
	struct emui_tile *t;

	t = emui_tile_create(parent, &emui_lineedit_drv, T_WIDGET, x, y, w, 1, 0, 0, 0, 0, NULL, P_INTERACTIVE);
	t->priv_data = calloc(1, sizeof(struct lineedit));

	struct lineedit *le = t->priv_data;

	le->maxlen = maxlen;
	le->type = type;
	le->buf = calloc(1, maxlen+1);

	return t;
}

// -----------------------------------------------------------------------
int emui_lineedit_set_text(struct emui_tile *t, char *text)
{
	struct lineedit *le = t->priv_data;

	free(le->buf);
	le->buf = strdup(text);
	le->pos = 0;

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
