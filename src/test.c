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
	struct emui_tile *split = emui_splitter(parent, AL_LEFT, 1, FIT_FILL, 30);

	// left side
	struct emui_tile *status_left = emui_window(split, 0, 0, 1, 1, "StatusL", P_NODECO);
	emui_tile_set_style(status_left, S_TEXT_INVERSED);
	struct emui_tile *misc = emui_label(status_left, 1, 0, 50, AL_LEFT, S_TEXT_INVERSED, "MIPS: 22.4  STOP  ALARM  CLOCK  IRQ  Q  MC  P");

	// right side
	struct emui_tile *status_right = emui_window(split, 0, 0, 1, 1, "StatusR", P_NODECO);
	emui_tile_set_style(status_right, S_TEXT_INVERSED);
	struct emui_tile *lfps = emui_label(status_right, 1, 0, 5, AL_RIGHT, S_TEXT_INVERSED, "FPS:");
	struct emui_tile *fps = emui_fpscounter(status_right, 7, 0, S_TEXT_INVERSED_BOLD);
	struct emui_tile *lframe = emui_label(status_right, 13, 0, 7, AL_RIGHT, S_TEXT_INVERSED, "Frame:");
	struct emui_tile *frame = emui_framecounter(status_right, 21, 0, S_TEXT_INVERSED_BOLD);

	return split;
}

// -----------------------------------------------------------------------
struct emui_tile * ui_create_ureg(struct emui_tile *parent)
{
	struct emui_tile *ureg = emui_window(parent, 0, 0, 55, 11, "Registers", P_NONE);
	emui_tile_set_focus_key(ureg, 'r');

	struct emui_tile *ureg_just = emui_justifier(ureg);
	struct emui_tile *ureg_r = emui_dummy_cont(ureg_just, 0, 0, 4, 11);
	struct emui_tile *ureg_hex = emui_dummy_cont(ureg_just, 4, 0, 4, 11);
	struct emui_tile *ureg_dec = emui_dummy_cont(ureg_just, 9, 0, 6, 11);
	struct emui_tile *ureg_oct = emui_dummy_cont(ureg_just, 16, 0, 6, 11);
	struct emui_tile *ureg_bin = emui_dummy_cont(ureg_just, 32, 0, 16, 11);
	struct emui_tile *ureg_ch = emui_dummy_cont(ureg_just, 40, 0, 2, 11);
	struct emui_tile *ureg_r40 = emui_dummy_cont(ureg_just, 42, 0, 3, 11);
	emui_label(ureg_r, 0, 0, 4, AL_LEFT, S_TEXT, "");
	emui_label(ureg_hex, 0, 0, 4, AL_LEFT, S_TEXT, "hex");
	emui_label(ureg_dec, 0, 0, 6, AL_LEFT, S_TEXT, "dec");
	emui_label(ureg_oct, 0, 0, 6, AL_LEFT, S_TEXT, "oct");
	emui_label(ureg_bin, 0, 0, 16, AL_LEFT, S_TEXT, "ZMVCLEGYX1234567");
	emui_label(ureg_ch, 0, 0, 2, AL_LEFT, S_TEXT, "ch");
	emui_label(ureg_r40, 0, 0, 3, AL_LEFT, S_TEXT, "R40");
	for (int i=1 ; i<=8 ; i++) {
		char buf[] = "R_:";
		buf[1] = '0' + i-1;
		emui_label(ureg_r, 0, i, 3, AL_RIGHT, S_TEXT, buf);
		emui_lineedit(ureg_hex, 0, i, 4, 4, TT_HEX, M_OVR);
		emui_lineedit(ureg_dec, 0, i, 6, 6, TT_INT, M_OVR);
		emui_lineedit(ureg_oct, 0, i, 6, 6, TT_OCT, M_OVR);
		emui_lineedit(ureg_bin, 0, i, 16, 16, TT_BIN, M_OVR);
		emui_lineedit(ureg_ch, 0, i, 2, 2, TT_TEXT, M_OVR);
		emui_lineedit(ureg_r40, 0, i, 3, 3, TT_R40, M_OVR);
	}

	return ureg;
}

// -----------------------------------------------------------------------
struct emui_tile * ui_create_sreg(struct emui_tile *parent)
{
	struct emui_tile *sreg = emui_window(parent, 0, 0, 55, 11, "Sys Registers", P_NONE);
	emui_label(sreg, 0, 0, 55, AL_LEFT, S_TEXT, "    hex  opcode D A   B   C");
	emui_label(sreg, 0, 1, 55, AL_LEFT, S_TEXT, "IR:");
	emui_lineedit(sreg, 4, 1, 4, 4, TT_HEX, M_OVR);
	emui_lineedit(sreg, 9, 1, 6, 6, TT_BIN, M_OVR);
	emui_lineedit(sreg, 16, 1, 1, 1, TT_BIN, M_OVR);
	emui_lineedit(sreg, 18, 1, 3, 3, TT_BIN, M_OVR);
	emui_lineedit(sreg, 22, 1, 3, 3, TT_BIN, M_OVR);
	emui_lineedit(sreg, 26, 1, 3, 3, TT_BIN, M_OVR);
	emui_label(sreg, 0, 2, 55, AL_LEFT, S_TEXT, "    hex  PMCZs139fS Q s NB");
	emui_label(sreg, 0, 3, 55, AL_LEFT, S_TEXT, "SR:");
	emui_lineedit(sreg, 4, 3, 4, 4, TT_HEX, M_OVR);
	emui_lineedit(sreg, 9, 3, 10, 10, TT_BIN, M_OVR);
	emui_lineedit(sreg, 20, 3, 1, 1, TT_BIN, M_OVR);
	emui_lineedit(sreg, 22, 3, 1, 1, TT_BIN, M_OVR);
	emui_lineedit(sreg, 24, 3, 4, 4, TT_BIN, M_OVR);
	emui_label(sreg, 0, 4, 55, AL_LEFT, S_TEXT, "KB:");
	emui_lineedit(sreg, 4, 4, 4, 4, TT_HEX, M_OVR);
	emui_lineedit(sreg, 9, 4, 8, 8, TT_BIN, M_OVR);
	emui_lineedit(sreg, 18, 4, 8, 8, TT_BIN, M_OVR);

	emui_tile_set_focus_key(sreg, 's');

	return sreg;
}

// -----------------------------------------------------------------------
struct emui_tile * ui_create_debugger(struct emui_tile *parent)
{
	// ASM
	struct emui_tile *dasm_split = emui_splitter(parent, AL_LEFT, 30, 30, FIT_FILL);
	emui_tile_set_properties(dasm_split, P_FOCUS_GROUP);
	emui_tile_set_name(dasm_split, "Debugger");
	emui_tile_set_focus_key(dasm_split, KEY_F(12));
	struct emui_tile *dasm = emui_window(dasm_split, 0, 0, 30, 20, "ASM", P_NONE);
	emui_tile_set_focus_key(dasm, 'a');

	// registers
	struct emui_tile *reg_split = emui_splitter(dasm_split, AL_TOP, 11, 11, FIT_FILL);
	struct emui_tile *sreg_split = emui_splitter(reg_split, AL_LEFT, 10, FIT_DIV2, 55);
	struct emui_tile *ureg = ui_create_ureg(sreg_split);
	struct emui_tile *sreg = ui_create_sreg(sreg_split);

	// memory
	struct emui_tile *mem_split = emui_splitter(reg_split, AL_TOP, 10, FIT_DIV2, 6);
	struct emui_tile *mem = emui_window(mem_split, 0, 0, 80, 20, "Memory", P_NONE);
	emui_tile_set_focus_key(mem, 'm');

	// eval
	struct emui_tile *eval_split = emui_splitter(mem_split, AL_BOTTOM, 3, 3, 10);
	struct emui_tile *eval = emui_window(eval_split, 0, 0, 80, 3, "Evaluator", P_NONE);
	emui_tile_set_focus_key(eval, 'e');

	// stack, watch, brk
	struct emui_tile *stack_split = emui_splitter(eval_split, AL_LEFT, 18, 18, 20);
	struct emui_tile *stack = emui_window(stack_split, 0, 0, 18, 10, "Stack", P_NONE);
	struct emui_tile *watch_split = emui_splitter(stack_split, AL_LEFT, 10, FIT_DIV2, 10);
	struct emui_tile *brk = emui_window(watch_split, 0, 0, 30, 10, "Watches", P_NONE);
	emui_tile_set_focus_key(brk, 'w');
	struct emui_tile *watch = emui_window(watch_split, 0, 0, 30, 10, "Breakpoints", P_NONE);
	emui_tile_set_focus_key(watch, 'b');

	// memory

	return dasm_split;
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	//emui_tile_debug_set(1);

	struct emui_tile *layout = emui_init(30);

	// status
	struct emui_tile *status_split = emui_splitter(layout, AL_BOTTOM, 1, 1, FIT_FILL);
	struct emui_tile *status = ui_create_status(status_split);

	// tabs
	struct emui_tile *tabs = emui_tabs(status_split);

	// terminal windows
	struct emui_tile *term1 = emui_window(tabs, 0, 0, 80, 25, "Term 1", P_MAXIMIZED);
	emui_tile_set_focus_key(term1, KEY_F(1));
	struct emui_tile *term2 = emui_window(tabs, 0, 0, 80, 25, "Term 2", P_MAXIMIZED);
	emui_tile_set_focus_key(term2, KEY_F(2));
	struct emui_tile *term3 = emui_window(tabs, 0, 0, 80, 25, "Term 3", P_MAXIMIZED);
	emui_tile_set_focus_key(term3, KEY_F(3));
	struct emui_tile *term4 = emui_window(tabs, 0, 0, 80, 25, "Term 4", P_MAXIMIZED);
	emui_tile_set_focus_key(term4, KEY_F(4));

	// device manager
	struct emui_tile *devmgr = emui_window(tabs, 0, 0, 80, 25, "DevManager", P_MAXIMIZED);
	emui_tile_set_focus_key(devmgr, KEY_F(11));

	// debugger
	struct emui_tile *debugger = ui_create_debugger(tabs);

	emui_focus(term1);

	emui_loop();
	emui_destroy();

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
