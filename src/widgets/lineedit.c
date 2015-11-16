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
#include "tiles.h"
#include "event.h"
#include "style.h"
#include "print.h"
#include "focus.h"

struct lineedit {
	int type;
	int in_edit;
	int mode;
	int pos;
	int maxlen;
	char *buf;
	char *editbuf;
	int txt_offset;
};

// -----------------------------------------------------------------------
void emui_lineedit_draw(EMTILE *t)
{
	struct lineedit *le = t->priv_data;
	char *cbuf;

	int style = t->style;

	if (le->in_edit) {
		cbuf = le->editbuf;
		if (t->content_invalid) {
			style = S_TEXT_EI;
		} else {
			style = S_TEXT_EN;
		}
	} else {
		cbuf = le->buf;
		if (emui_has_focus(t) ) {
			if (t->content_invalid) {
				style = S_TEXT_FI;
			} else {
				style = S_TEXT_FN;
			}
		}
	}

	int margin = 1;
	if (le->mode == M_OVR) margin = 0;

	// scroll the string so it's within the window
	if (le->pos >= t->i.w + le->txt_offset) {
		le->txt_offset = le->pos - t->i.w + margin;
	} else if (le->pos < le->txt_offset) {
		le->txt_offset = le->pos;
	}

	if (le->in_edit) {
		emuixyprt(t, 0, 0, style, "%-*s", t->i.w, cbuf + le->txt_offset);
		emuixy(t, le->pos - le->txt_offset, 0);
	} else {
		emuixyprt(t, 0, 0, style, "%-*s", t->i.w, cbuf);
	}
}

// -----------------------------------------------------------------------
void emui_lineedit_mode(EMTILE *t, int mode)
{
	struct lineedit *le = t->priv_data;
	le->mode = mode;
}

// -----------------------------------------------------------------------
static int le_handle_non_edit(EMTILE *t, struct emui_event *ev)
{
	switch (ev->sender) {
		case '\n':
			emui_lineedit_edit(t, 1);
			break;
		default:
			// event has not been handled
			return 1;
	}

	return 0;
}

// -----------------------------------------------------------------------
static int le_char_valid(int type, int ch, int pos)
{
	switch (type) {
		case TT_TEXT:
			if (isprint(ch)) return 1;
			break;
		case TT_INT:
			if (isdigit(ch) || ((ch == '-') && (pos == 0))) return 1;
			break;
		case TT_HEX:
			if (isxdigit(ch)) return 1;
			break;
		case TT_OCT:
			if ((ch >= 0) && (ch <= '7')) return 1;
			break;
		case TT_BIN:
			if ((ch >= 0) && (ch <= '1')) return 1;
			break;
		default:
			break;
	}
	return 0;
}

// -----------------------------------------------------------------------
static int le_handle_edit(EMTILE *t, struct emui_event *ev)
{
	struct lineedit *le = t->priv_data;

	switch (ev->sender) {
		case KEY_ENTER:
		case '\n':
			emui_lineedit_edit(t, 0);
			break;
		case 27: // ESC
			if (t->properties & P_AUTOEDIT) {
				return 1;
			}
			le->in_edit = 0;
			t->accept_updates = 1;
			curs_set(0);
			break;
		case KEY_LEFT:
			if (le->pos > 0) le->pos -= 1;
			break;
		case KEY_RIGHT:
			if (le->pos < strlen(le->editbuf)) le->pos += 1;
			break;
		case KEY_HOME:
			le->pos = 0;
			break;
		case KEY_END:
			le->pos = strlen(le->editbuf);
			if (le->mode == M_OVR) {
				le->pos--;
			}
			break;
		case KEY_BACKSPACE:
		case 127:
			if (le->pos > 0) {
				memmove(le->editbuf+le->pos-1, le->editbuf+le->pos, strlen(le->editbuf)-le->pos);
				le->pos -= 1;
				le->editbuf[strlen(le->editbuf)-1] = '\0';
			}
			break;
		case KEY_IC: // INSERT
			emui_lineedit_mode(t, le->mode ^ 1);
			if ((le->mode == M_OVR) && (le->pos == strlen(le->editbuf))) {
				le->pos--;
			}
			break;
		case KEY_DC: // DELETE
			if (le->pos < strlen(le->editbuf)) {
				memmove(le->editbuf+le->pos, le->editbuf+le->pos+1, strlen(le->editbuf)-le->pos-1);
				le->editbuf[strlen(le->editbuf)-1] = '\0';
			}
			break;
		case 9: // TAB
		case KEY_BTAB:
		case KEY_UP:
		case KEY_DOWN:
			// allow handling focus changes in autoedit mode
			if (t->properties & P_AUTOEDIT) {
				return 1;
			}
			break;
		default:
			if (!le_char_valid(le->type, ev->sender, le->pos)) break;

			if (le->mode == M_OVR) {
				if (le->pos <= le->maxlen) {
					if (le->pos == strlen(le->editbuf)) {
						le->editbuf[le->pos+1] = '\0';
					}
					le->editbuf[le->pos] = ev->sender;
					if (le->pos < le->maxlen - 1) {
						le->pos++;
					}
				}
			} else {
				if (strlen(le->editbuf) < le->maxlen) {
					memmove(le->editbuf+le->pos+1, le->editbuf+le->pos, strlen(le->editbuf)-le->pos);
					le->editbuf[le->pos++] = ev->sender;
				}
			}
			break;
	}

	// eat all unhandled events (ignore keypresses invalid for input type)
	return 0;
}

// -----------------------------------------------------------------------
int emui_lineedit_event_handler(EMTILE *t, struct emui_event *ev)
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
void emui_lineedit_destroy_priv_data(EMTILE *t)
{
	struct lineedit *le = t->priv_data;
	curs_set(0);
	free(le->buf);
	free(le->editbuf);
	free(le);
}

// -----------------------------------------------------------------------
int emui_lineedit_focus_handler(EMTILE *t, int focus)
{
	struct lineedit *le = t->priv_data;

	if (t->properties & P_AUTOEDIT) {
		if (focus & !(le->in_edit)) {
			emui_lineedit_edit(t, 1);
		} else if (le->in_edit) {
			emui_lineedit_edit(t, 0);
		}
	}
	return 0;
}

// -----------------------------------------------------------------------
struct emtile_drv emui_lineedit_drv = {
	.draw = emui_lineedit_draw,
	.update_children_geometry = NULL,
	.event_handler = emui_lineedit_event_handler,
	.focus_handler = emui_lineedit_focus_handler,
	.destroy_priv_data = emui_lineedit_destroy_priv_data,
};

// -----------------------------------------------------------------------
EMTILE * emui_lineedit(EMTILE *parent, int x, int y, int w, int maxlen, int type, int mode)
{
	EMTILE *t;

	t = emtile(parent, &emui_lineedit_drv, x, y, w, 1, 0, 0, 0, 0, NULL, P_INTERACTIVE);
	emtile_set_style(t, S_EDIT_NN);

	t->priv_data = calloc(1, sizeof(struct lineedit));

	struct lineedit *le = t->priv_data;

	le->maxlen = maxlen;
	le->type = type;
	le->buf = calloc(1, maxlen+1);
	le->mode = mode;

	return t;
}

// -----------------------------------------------------------------------
char * emui_lineedit_get_text(EMTILE *t)
{
	struct lineedit *le = t->priv_data;

	return le->buf;
}

// -----------------------------------------------------------------------
int emui_lineedit_set_text(EMTILE *t, char *text)
{
	struct lineedit *le = t->priv_data;

	free(le->buf);
	le->buf = calloc(1, le->maxlen + 1);
	strncpy(le->buf, text, le->maxlen);
	le->pos = 0;

	return 0;
}

// -----------------------------------------------------------------------
int emui_lineedit_get_int(EMTILE *t)
{
	struct lineedit *le = t->priv_data;
	switch (le->type) {
		case TT_HEX:
			return strtol(le->buf, NULL, 16);
			break;
		case TT_OCT:
			return strtol(le->buf, NULL, 8);
			break;
		case TT_BIN:
			return strtol(le->buf, NULL, 2);
			break;
		case TT_INT:
		case TT_TEXT:
		default:
			return strtol(le->buf, NULL, 10);
			break;

	}
	return 0;
}

// -----------------------------------------------------------------------
// convert an integer to formatted string with its binary representation
static void int2bin(char *o, int size, unsigned value)
{
	while (size > 0) {
		size--;
		*o = (value >> size) & 1 ? '1' : '0';
		o++;
	}
	*o = '\0';
}

// -----------------------------------------------------------------------
void emui_lineedit_set_int(EMTILE *t, int v)
{
	struct lineedit *le = t->priv_data;
	switch (le->type) {
		case TT_HEX:
			snprintf(le->buf, le->maxlen+1, "%x", v);
			break;
		case TT_OCT:
			snprintf(le->buf, le->maxlen+1, "%o", v);
			break;
		case TT_BIN:
			int2bin(le->buf, le->maxlen, v);
			break;
		case TT_INT:
		case TT_TEXT:
		default:
			snprintf(le->buf, le->maxlen+1, "%i", v);
			break;
	}
}

// -----------------------------------------------------------------------
void emui_lineedit_set_mode(EMTILE *t, int mode)
{
	struct lineedit *le = t->priv_data;

	le->mode = mode;
}

// -----------------------------------------------------------------------
void emui_lineedit_set_pos(EMTILE *t, unsigned pos)
{
	struct lineedit *le = t->priv_data;
	if (pos <= le->maxlen) {
		le->pos = pos;
	} else {
		le->pos = 0;
	}
}

// -----------------------------------------------------------------------
void emui_lineedit_edit(EMTILE *t, int state)
{
	struct lineedit *le = t->priv_data;
	le->in_edit = state;
	curs_set(state);

	if (state == 1) {
		t->accept_updates = 0;
		free(le->editbuf);
		le->editbuf = calloc(1, le->maxlen + 1);
		strcpy(le->editbuf, le->buf);
	} else {
		t->accept_updates = 1;
		free(le->buf);
		le->buf = calloc(1, le->maxlen + 1);
		strcpy(le->buf, le->editbuf);

		if (emtile_notify_change(t)) {
			emui_lineedit_edit(t, 1);
		} else {
			emui_lineedit_set_pos(t, 0);
		}
	}
}

// vim: tabstop=4 shiftwidth=4 autoindent
