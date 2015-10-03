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

#include "emui.h"

struct emui_tile *layout;
struct emui_tile *win0, *win1, *win2, *win3, *win4;
struct emui_tile *wid1, *wid2, *wid3, *wid4, *wid5;

// -----------------------------------------------------------------------
int ev_handler(struct emui_tile *t, struct emui_event *ev)
{
	if (ev->type == EV_KEY) {
		switch (ev->data.key) {
			case KEY_F(5):
				emui_tile_focus(win1);
				return 0;
			case KEY_F(6):
				emui_tile_focus(win2);
				return 0;
			case KEY_F(7):
				emui_tile_focus(win3);
				return 0;
			case KEY_F(8):
				emui_tile_focus(win4);
				return 0;
			default:
				return 1;
		}
	}
	return 1;
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{

	//emui_tile_debug_set(1);

	layout = emui_init(60);
	win0 = emui_tabs_new(layout);

	win1 = emui_window_new(win0, 1, 1, 30, 20, "The Window 1", P_NONE);
	wid1 = emui_lineedit_new(win1, 0, 0, 20, 30, TT_TEXT, P_NONE);
	emui_lineedit_settext(wid1, "IU");

	win2 = emui_window_new(win0, 32, 3, 30, 20, "The Window 2", P_NONE);
	wid2 = emui_lineedit_new(win2, 0, 0, 20, 30, TT_TEXT, P_NONE);
	emui_lineedit_settext(wid2, "Nasumi");
	wid2 = emui_lineedit_new(win2, 0, 2, 20, 30, TT_TEXT, P_NONE);
	emui_lineedit_settext(wid2, "Test content");

	win3 = emui_window_new(win0, 62, 4, 30, 20, "The Window 3", P_NONE);
	wid3 = emui_lineedit_new(win3, 0, 0, 20, 30, TT_TEXT, P_NONE);
	emui_lineedit_settext(wid3, "Sample txt");

	win4 = emui_window_new(win0, 7, 5, 30, 10, "The Window 4", P_NONE);
	wid4 = emui_lineedit_new(win4, 0, 3, 20, 30, TT_TEXT, P_NONE);
	emui_lineedit_settext(wid4, "Another");
	wid4 = emui_framecounter_new(win4, 0, 0);
	wid4 = emui_fpscounter_new(win4, 0, 1);


	emui_tile_set_event_handler(win0, ev_handler);


	emui_tile_focus(win2);

	emui_loop();
	emui_destroy();

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
