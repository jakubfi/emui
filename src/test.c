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

// -----------------------------------------------------------------------
struct emui_tile * ui_create_status(struct emui_tile *parent)
{
	struct emui_tile *split = emui_splitter_new(parent, AL_LEFT, 1, FIT_FILL, 30);

	// left side
	struct emui_tile *status_left = emui_window_new(split, 0, 0, 1, 1, "StatusL", P_NODECO);
	emui_tile_set_style(status_left, S_TEXT_INVERSED);
	struct emui_tile *misc = emui_label_new(status_left, 1, 0, 30, AL_LEFT, S_TEXT_INVERSED, "MIPS: 22.4  ALARM  TIMER  ...");

	// right side
	struct emui_tile *status_right = emui_window_new(split, 0, 0, 1, 1, "StatusR", P_NODECO);
	emui_tile_set_style(status_right, S_TEXT_INVERSED);
	struct emui_tile *lfps = emui_label_new(status_right, 1, 0, 5, AL_RIGHT, S_TEXT_INVERSED, "FPS:");
	struct emui_tile *fps = emui_fpscounter_new(status_right, 7, 0, S_TEXT_INVERSED_BOLD);
	struct emui_tile *lframe = emui_label_new(status_right, 13, 0, 7, AL_RIGHT, S_TEXT_INVERSED, "Frame:");
	struct emui_tile *frame = emui_framecounter_new(status_right, 21, 0, S_TEXT_INVERSED_BOLD);

	return split;
}

// -----------------------------------------------------------------------
struct emui_tile * ui_create_debugger(struct emui_tile *parent)
{
	// ASM
	struct emui_tile *dasm_split = emui_splitter_new(parent, AL_LEFT, 30, 30, FIT_FILL);
	emui_tile_set_properties(dasm_split, P_FOCUS_GROUP);
	emui_tile_set_name(dasm_split, "Debugger");
	emui_tile_set_focus_key(dasm_split, KEY_F(12));
	struct emui_tile *dasm = emui_window_new(dasm_split, 0, 0, 30, 20, "ASM", P_NONE);
	emui_tile_set_focus_key(dasm, 'a');

	// registers
	struct emui_tile *reg_split = emui_splitter_new(dasm_split, AL_TOP, 11, 11, FIT_FILL);
	struct emui_tile *sreg_split = emui_splitter_new(reg_split, AL_LEFT, 55, FIT_DIV2, 55);
	struct emui_tile *ureg = emui_window_new(sreg_split, 0, 0, 55, 11, "Registers", P_NONE);
	emui_tile_set_focus_key(ureg, 'r');
	emui_label_new(ureg, 0, 1, 2, AL_RIGHT, S_TEXT, "0x");
	emui_label_new(ureg, 0, 2, 2, AL_RIGHT, S_TEXT, "0x");
	emui_label_new(ureg, 0, 3, 2, AL_RIGHT, S_TEXT, "0x");
	emui_label_new(ureg, 0, 4, 2, AL_RIGHT, S_TEXT, "0x");
	emui_label_new(ureg, 0, 5, 2, AL_RIGHT, S_TEXT, "0x");
	emui_label_new(ureg, 0, 6, 2, AL_RIGHT, S_TEXT, "0x");
	emui_label_new(ureg, 0, 7, 2, AL_RIGHT, S_TEXT, "0x");
	emui_label_new(ureg, 0, 8, 2, AL_RIGHT, S_TEXT, "0x");
	emui_lineedit_new(ureg, 2, 1, 4, 4, TT_TEXT);
	emui_lineedit_new(ureg, 2, 2, 4, 4, TT_TEXT);
	emui_lineedit_new(ureg, 2, 3, 4, 4, TT_TEXT);
	emui_lineedit_new(ureg, 2, 4, 4, 4, TT_TEXT);
	emui_lineedit_new(ureg, 2, 5, 4, 4, TT_TEXT);
	emui_lineedit_new(ureg, 2, 6, 4, 4, TT_TEXT);
	emui_lineedit_new(ureg, 2, 7, 4, 4, TT_TEXT);
	emui_lineedit_new(ureg, 2, 8, 4, 4, TT_TEXT);
	struct emui_tile *sreg = emui_window_new(sreg_split, 0, 0, 55, 11, "Sys Registers", P_NONE);

	emui_tile_set_focus_key(sreg, 's');
	struct emui_tile *test = emui_label_new(sreg, 0, 0, 30, AL_CENTER, S_TEXT_CHANGED, "Test");

	// memory
	struct emui_tile *mem_split = emui_splitter_new(reg_split, AL_TOP, 10, FIT_DIV2, 6);
	struct emui_tile *mem = emui_window_new(mem_split, 0, 0, 80, 20, "Memory", P_NONE);
	emui_tile_set_focus_key(mem, 'm');

	// eval
	struct emui_tile *eval_split = emui_splitter_new(mem_split, AL_BOTTOM, 3, 3, 10);
	struct emui_tile *eval = emui_window_new(eval_split, 0, 0, 80, 3, "Evaluator", P_NONE);
	emui_tile_set_focus_key(eval, 'e');

	// stack, watch, brk
	struct emui_tile *stack_split = emui_splitter_new(eval_split, AL_LEFT, 18, 18, 20);
	struct emui_tile *stack = emui_window_new(stack_split, 0, 0, 18, 10, "Stack", P_NONE);
	struct emui_tile *watch_split = emui_splitter_new(stack_split, AL_LEFT, 10, FIT_DIV2, 10);
	struct emui_tile *brk = emui_window_new(watch_split, 0, 0, 30, 10, "Watches", P_NONE);
	emui_tile_set_focus_key(brk, 'w');
	struct emui_tile *watch = emui_window_new(watch_split, 0, 0, 30, 10, "Breakpoints", P_NONE);
	emui_tile_set_focus_key(watch, 'b');

	// memory

	return dasm_split;
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	emui_tile_debug_set(1);

	struct emui_tile *layout = emui_init(30);

	// status
	struct emui_tile *status_split = emui_splitter_new(layout, AL_BOTTOM, 1, 1, FIT_FILL);
	struct emui_tile *status = ui_create_status(status_split);

	// tabs
	struct emui_tile *tabs = emui_tabs_new(status_split);

	// terminal windows
	struct emui_tile *term1 = emui_window_new(tabs, 0, 0, 80, 25, "Term 1", P_MAXIMIZED);
	emui_tile_set_focus_key(term1, KEY_F(1));
	struct emui_tile *term2 = emui_window_new(tabs, 0, 0, 80, 25, "Term 2", P_MAXIMIZED);
	emui_tile_set_focus_key(term2, KEY_F(2));
	struct emui_tile *term3 = emui_window_new(tabs, 0, 0, 80, 25, "Term 3", P_MAXIMIZED);
	emui_tile_set_focus_key(term3, KEY_F(3));
	struct emui_tile *term4 = emui_window_new(tabs, 0, 0, 80, 25, "Term 4", P_MAXIMIZED);
	emui_tile_set_focus_key(term4, KEY_F(4));

	// device manager
	struct emui_tile *devmgr = emui_window_new(tabs, 0, 0, 80, 25, "DevManager", P_MAXIMIZED);
	emui_tile_set_focus_key(devmgr, KEY_F(11));

	// debugger
	struct emui_tile *debugger = ui_create_debugger(tabs);

	emui_focus(term1);

	emui_loop();
	emui_destroy();

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
