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

char *help_text = "                 _____  ______ ______\n\
.-----.--------.|  |  ||      |      |\n\
|  -__|        ||__    |  --  |  --  |\n\
|_____|__|__|__|   |__||______|______|\n\
\n\
UI shortcuts:\n\
 * m, r, s, a, w, b - switch window\n\
 * h,? - help\n\
 * l - configure logging\n\
 * ctrl-q - quit\n\
\n\
CPU control:\n\
 * ctrl-r - run/stop\n\
 * shift-del - reset\n\
 * space - step (cycle)\n\
 * ctrl-t - switch clock on/off\n\
\n\
Registers and system registers windows:\n\
 * arrows, tab - move around\n\
 * 0-7 - select user register\n\
 * ENTER - edit register contents\n\
\n\
Memory window:\n\
 * up/down, pgup/down - scroll\n\
 * ctrl-pgup/pgdown, </> - skip to prev/next memory page\n\
 * left/right - switch to prev/next memory segment\n\
 * 0-9 - select memory segment 0-9\n\
 * i - move to current IC location\n\
 * g - go to memory location\n\
 * d - decode memory under cursor\n\
 * f - find\n\
 * o - open (load) image file\n\
 * c - change display between hex/ASCII/R40\n\
 * ENTER - edit memory under cursor\n\
 * ESC - exit edit\n\
 * . - show current memory layout\n\
\n\
Assembler window:\n\
 * up/down/pgup/pgdn/home/end - scroll contents\n\
 * ctrl-pgup/-pgdown - skip to previous/next memory page\n\
 * < > - switch to previous/next memory segment\n\
 * 0-9 - select memory segment 0-9\n\
 * i - move to current IC location\n\
 * g - go to memory location\n\
 * f - follow IC on/off\n\
\n\
Watches window:\n\
 * up/down/pgup/pgdn/home/end - move around\n\
 * INS - add watch\n\
 * DEL - delete watch\n\
 * ENTER - edit watch\n\
\n\
Breakpoints window:\n\
 * up/down/pgup/pgdn/home/end - move around\n\
 * INS - add breakpoint\n\
 * DEL - delete breakpoint\n\
 * ENTER - edit breakpoint\n\
\n\
Logging window:\n\
 * up/down/pgup/pgdn/home/end - move around\n\
 * 0-9 - change logging level\n\
 * +/- - change logging level\n\
 * space - diasble/enable logging\n\
\n\
Status bar indicators:\n\
 * STOP/WAIT/RUN\n\
 * ALARM\n\
 * CLOCK\n\
 * IRQ\n\
 * Q\n\
 * MC\n\
 * P\n\
";

EMTILE *help;

// -----------------------------------------------------------------------
int help_key_handler(EMTILE *t, int key)
{
	emtile_delete(help);
	return E_HANDLED;
}

// -----------------------------------------------------------------------
void help_dlg(EMTILE *parent, EMTILE *geom_parent)
{
	EMTILE *help_tv;

	help = emui_frame(parent, 1, 1, 60, 20, "Help", P_VMAXIMIZE | P_HCENTER);
	emtile_set_geometry_parent(help, geom_parent, GEOM_INTERNAL);
	emtile_set_key_handler(help, help_key_handler);
	help_tv = emui_textview(help, 0, 0, 10, 10);
	emtile_set_properties(help_tv, P_MAXIMIZE);
	emtext_append_str(emui_textview_get_emtext(help_tv), S_DEFAULT, "%s", help_text);

	emui_focus(help);
}

// vim: tabstop=4 shiftwidth=4 autoindent
