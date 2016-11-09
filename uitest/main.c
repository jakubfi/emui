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

EMTILE *tabs;

enum app_styles {
	S_INSTRUCTION = S_FIRST_APP_STYLE,
	S_IC,
};

static struct emui_style_def app_scheme[] = {
	{ S_INSTRUCTION,	COLOR_BLACK,	COLOR_YELLOW,	A_BOLD },
	{ S_IC,				COLOR_BLUE,		COLOR_GREEN,	A_BOLD },
	{ -1, 0, 0, 0 }
};

EMTILE * ui_create_statusbar(EMTILE *parent);
EMTILE * ui_create_debugger(EMTILE *parent);
void ui_destroy_dasm();
void help_dlg(EMTILE *parent, EMTILE *geom_parent);

// -----------------------------------------------------------------------
int top_key_handler(EMTILE *t, int key)
{
	switch (key) {
	case 'h':
	case '?':
	case 'H':
		help_dlg(t, tabs);
		return E_HANDLED;
	}

	return E_UNHANDLED;
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	EMTILE *layout = emui_init(30);

	emui_scheme_set(app_scheme);
	emtile_set_key_handler(layout, top_key_handler);

	// status
	EMTILE *status_split = emui_splitter(layout, AL_BOTTOM, 1, 1, FIT_FILL);
	ui_create_statusbar(status_split);

	// tabs
	tabs = emui_tabs(status_split);

	// terminal windows
	EMTILE *term1 = emui_frame(tabs, 0, 0, 80, 25, "Term 1", P_MAXIMIZE);
	emtile_set_focus_key(term1, KEY_F(1));
	EMTILE *term2 = emui_frame(tabs, 0, 0, 80, 25, "Term 2", P_MAXIMIZE);
	emtile_set_focus_key(term2, KEY_F(2));
	EMTILE *term3 = emui_frame(tabs, 0, 0, 80, 25, "Term 3", P_MAXIMIZE);
	emtile_set_focus_key(term3, KEY_F(3));
	EMTILE *term4 = emui_frame(tabs, 0, 0, 80, 25, "Term 4", P_MAXIMIZE);
	emtile_set_focus_key(term4, KEY_F(4));

	// device manager
	EMTILE *devmgr = emui_frame(tabs, 0, 0, 80, 25, "DevManager", P_MAXIMIZE);
	emtile_set_focus_key(devmgr, KEY_F(11));

	// debugger
	EMTILE *debugger = ui_create_debugger(tabs);

	emui_focus(debugger);
	emui_loop();

	ui_destroy_dasm();
	emui_destroy();

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
