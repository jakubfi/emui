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

EMTILE * ui_create_ureg(EMTILE *parent);
EMTILE * ui_create_sreg(EMTILE *parent);
EMTILE * ui_create_dasm(EMTILE *parent);
EMTILE * ui_create_memory(EMTILE *parent);
void help_dlg(EMTILE *parent, EMTILE *geom_parent);

// -----------------------------------------------------------------------
EMTILE * ui_create_debugger(EMTILE *parent)
{
	// Debugger
	EMTILE *dasm_split = emui_splitter(parent, AL_LEFT, 30, 30, FIT_FILL);
	emtile_set_properties(dasm_split, P_FOCUS_GROUP);
	emtile_set_name(dasm_split, "Debugger");
	emtile_set_focus_key(dasm_split, KEY_F(12));

	// ASM
	ui_create_dasm(dasm_split);

	// registers
	EMTILE *reg_split = emui_splitter(dasm_split, AL_TOP, 11, 11, FIT_FILL);
	EMTILE *sreg_split = emui_splitter(reg_split, AL_LEFT, 10, FIT_DIV2, 55);
	ui_create_ureg(sreg_split);
	ui_create_sreg(sreg_split);

	// memory
	EMTILE *mem_split = emui_splitter(reg_split, AL_TOP, 10, FIT_DIV2, 6);
	ui_create_memory(mem_split);

	// eval
	EMTILE *eval_split = emui_splitter(mem_split, AL_BOTTOM, 3, 3, 10);
	EMTILE *eval = emui_frame(eval_split, 0, 0, 80, 3, "Evaluator", P_VCENTER|P_HMAXIMIZE);
	emtile_set_focus_key(eval, 'e');

	// stack, watch, brk
	EMTILE *stack_split = emui_splitter(eval_split, AL_LEFT, 18, 18, 20);
	emui_frame(stack_split, 0, 0, 18, 10, "Stack", P_CENTER);
	EMTILE *watch_split = emui_splitter(stack_split, AL_LEFT, 10, FIT_DIV2, 10);
	EMTILE *brk = emui_frame(watch_split, 0, 0, 25, 10, "Breakpoints", P_CENTER);
	emtile_set_focus_key(brk, 'b');
	EMTILE *brklist = emui_list(brk);
	for (int i=0 ; i<20 ; i++) {
		EMTILE *brkc = emui_dummy_cont(brklist, 0, i, 30, 1);
		emtile_set_properties(brkc, P_HMAXIMIZE);
		char str[32];
		sprintf(str, "%3i:", i);
		emui_label(brkc, 0, 0, 4, S_DEFAULT, str);
		emui_lineedit(brkc, 4, 0, 100, 100, TT_TEXT, M_INS);

	}
	EMTILE *watch = emui_frame(watch_split, 0, 0, 30, 10, "Watches", P_CENTER);
	emtile_set_focus_key(watch, 'w');
	EMTILE *watchlist = emui_list(watch);
	for (int i=0 ; i<20 ; i++) {
		EMTILE *wc = emui_dummy_cont(watchlist, 0, i, 30, 1);
		emtile_set_properties(wc, P_HMAXIMIZE);
		EMTILE *ws = emui_splitter(wc, AL_RIGHT, 6, 6, FIT_FILL);
		emui_label(ws, 0, 0, 6, 1, "int");
		EMTILE *ws2 = emui_splitter(ws, AL_RIGHT, 6, 6, FIT_FILL);
		emui_label(ws2, 0, 0, 6, 1, "hex");
		char str[32];
		sprintf(str, "watch_%i", i);
		EMTILE *l = emui_lineedit(ws2, 4, 0, 100, 100, TT_TEXT, M_INS);
		emui_lineedit_set_text(l, str);
	}

	// memory

	return dasm_split;
}

// vim: tabstop=4 shiftwidth=4 autoindent
