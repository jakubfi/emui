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

#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <emcrk/r40.h>
#include <emdas.h>

#include "emui.h"

char *help = "\
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
 * arrows, pgup/down - move around\n\
 * ctrl-pgup/-pgdown - skip to previosu/next memory page\n\
 * < > - switch to prev/next memory segment\n\
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

#define MAX_MEM 1024*32

uint16_t treg[8];
struct emdas *emd;
uint16_t *mem[16];
uint16_t dasm_start;
int dasm_segment;
int dasm_follow;

// -----------------------------------------------------------------------
int mem_get(int nb, uint16_t addr, uint16_t *dest)
{
	if (addr < MAX_MEM) {
		*dest = mem[nb][addr];
		return 1;
	} else {
		return 0;
	}
}

// -----------------------------------------------------------------------
struct emui_tile * ui_create_status(struct emui_tile *parent)
{
	struct emui_tile *split = emui_splitter(parent, AL_LEFT, 1, FIT_FILL, 30);

	// left side
	struct emui_tile *status_left = emui_dummy_cont(split, 0, 0, 1, 1);
	struct emui_tile *misc = emui_label(status_left, 1, 0, 50, AL_LEFT, S_INV, "MIPS: 22.4  STOP  ALARM  CLOCK  IRQ  Q  MC  P");
	emui_tile_set_properties(misc, P_MAXIMIZED);

	// right side
	struct emui_tile *status_right = emui_dummy_cont(split, 0, 0, 1, 1);
	struct emui_tile *lfps = emui_label(status_right, 0, 0, 5, AL_LEFT, S_INV, "FPS:");
	struct emui_tile *fps = emui_fpscounter(status_right, 5, 0, S_INV_BOLD);
	struct emui_tile *lframe = emui_label(status_right, 11, 0, 9, AL_LEFT, S_INV, "  Frame: ");
	struct emui_tile *frame = emui_framecounter(status_right, 20, 0, S_INV_BOLD);

	return split;
}

// -----------------------------------------------------------------------
int reg_2char_update(struct emui_tile *t)
{
	char buf[3];

	buf[0] = treg[t->id] >> 8;
	buf[1] = treg[t->id] & 0xff;
	buf[2] = '\0';
	emui_lineedit_set_text(t, buf);

	return 0;
}
// -----------------------------------------------------------------------
int reg_2char_changed(struct emui_tile *t)
{
	char *txt;

	txt = emui_lineedit_get_text(t);
	treg[t->id] = *txt ? (*txt << 8) + *(txt+1) : 0;
	return 0;
}

// -----------------------------------------------------------------------
int reg_r40_update(struct emui_tile *t)
{
	char buf[4];
	uint16_t val;

	val = treg[t->id];
	emui_lineedit_set_text(t, r40_to_ascii(&val, 1, buf));

	return 0;
}

// -----------------------------------------------------------------------
int reg_r40_changed(struct emui_tile *t)
{
	uint16_t val;

	ascii_to_r40(emui_lineedit_get_text(t), NULL, &val);
	treg[t->id] = val;

	return 0;
}

// -----------------------------------------------------------------------
int reg_int_update(struct emui_tile *t)
{
	emui_lineedit_set_int(t, treg[t->id]);
	return 0;
}

// -----------------------------------------------------------------------
int reg_int_changed(struct emui_tile *t)
{
	int val;

	val = emui_lineedit_get_int(t);

	if ((val > USHRT_MAX) || (val < SHRT_MIN)) {
		return 1;
	} else {
		treg[t->id] = val;
		return 0;
	}
}

// -----------------------------------------------------------------------
struct emui_tile * ui_create_ureg(struct emui_tile *parent)
{
	struct emui_tile *ureg = emui_frame(parent, 0, 0, 55, 11, "Registers", P_NONE);
	emui_tile_set_focus_key(ureg, 'r');

	struct emui_tile *ureg_just = emui_justifier(ureg);
	struct emui_tile *ureg_r = emui_dummy_cont(ureg_just, 0, 0, 4, 11);
	struct emui_tile *ureg_hex = emui_dummy_cont(ureg_just, 4, 0, 4, 11);
	struct emui_tile *ureg_dec = emui_dummy_cont(ureg_just, 9, 0, 6, 11);
	struct emui_tile *ureg_oct = emui_dummy_cont(ureg_just, 16, 0, 6, 11);
	struct emui_tile *ureg_bin = emui_dummy_cont(ureg_just, 32, 0, 16, 11);
	struct emui_tile *ureg_ch = emui_dummy_cont(ureg_just, 40, 0, 2, 11);
	struct emui_tile *ureg_r40 = emui_dummy_cont(ureg_just, 42, 0, 3, 11);
	emui_label(ureg_r, 0, 0, 4, AL_LEFT, S_TEXT_NN, "");
	emui_label(ureg_hex, 0, 0, 4, AL_LEFT, S_TEXT_NN, "hex");
	emui_label(ureg_dec, 0, 0, 6, AL_LEFT, S_TEXT_NN, "dec");
	emui_label(ureg_oct, 0, 0, 6, AL_LEFT, S_TEXT_NN, "oct");
	emui_label(ureg_bin, 0, 0, 16, AL_LEFT, S_TEXT_NN, "ZMVCLEGYX1234567");
	emui_label(ureg_ch, 0, 0, 2, AL_LEFT, S_TEXT_NN, "ch");
	emui_label(ureg_r40, 0, 0, 3, AL_LEFT, S_TEXT_NN, "R40");

	char buf[] = "R_:";
	struct emui_tile *r;

	for (int i=0 ; i<8 ; i++) {
		buf[1] = '0' + i;
		emui_label(ureg_r, 0, i+1, 3, AL_RIGHT, S_TEXT_NN, buf);
		treg[i] = 0;

		r = emui_lineedit(ureg_hex, i, 0, i+1, 4, 4, TT_HEX, M_OVR);
		emui_tile_set_change_handler(r, reg_int_changed);
		emui_tile_set_update_handler(r, reg_int_update);
		emui_tile_set_focus_key(r, buf[1]);

		r = emui_lineedit(ureg_dec, i, 0, i+1, 6, 6, TT_INT, M_OVR);
		emui_tile_set_change_handler(r, reg_int_changed);
		emui_tile_set_update_handler(r, reg_int_update);

		r = emui_lineedit(ureg_oct, i, 0, i+1, 6, 6, TT_OCT, M_OVR);
		emui_tile_set_change_handler(r, reg_int_changed);
		emui_tile_set_update_handler(r, reg_int_update);

		r = emui_lineedit(ureg_bin, i, 0, i+1, 16, 16, TT_BIN, M_OVR);
		emui_tile_set_change_handler(r, reg_int_changed);
		emui_tile_set_update_handler(r, reg_int_update);

		r = emui_lineedit(ureg_ch, i, 0, i+1, 2, 2, TT_TEXT, M_OVR);
		emui_tile_set_change_handler(r, reg_2char_changed);
		emui_tile_set_update_handler(r, reg_2char_update);

		r = emui_lineedit(ureg_r40, i, 0, i+1, 3, 3, TT_TEXT, M_OVR);
		emui_tile_set_change_handler(r, reg_r40_changed);
		emui_tile_set_update_handler(r, reg_r40_update);
	}

	return ureg;
}

// -----------------------------------------------------------------------
struct emui_tile * ui_create_sreg(struct emui_tile *parent)
{
	struct emui_tile *sreg = emui_frame(parent, 0, 0, 55, 11, "Sys Registers", P_NONE);
	emui_label(sreg, 0, 0, 55, AL_LEFT, S_TEXT_NN, "    hex  opcode D A   B   C");
	emui_label(sreg, 0, 1, 55, AL_LEFT, S_TEXT_NN, "IR:");
	emui_lineedit(sreg, -1, 4, 1, 4, 4, TT_HEX, M_OVR);
	emui_lineedit(sreg, -1, 9, 1, 6, 6, TT_BIN, M_OVR);
	emui_lineedit(sreg, -1, 16, 1, 1, 1, TT_BIN, M_OVR);
	emui_lineedit(sreg, -1, 18, 1, 3, 3, TT_BIN, M_OVR);
	emui_lineedit(sreg, -1, 22, 1, 3, 3, TT_BIN, M_OVR);
	emui_lineedit(sreg, -1, 26, 1, 3, 3, TT_BIN, M_OVR);
	emui_label(sreg, 0, 2, 55, AL_LEFT, S_TEXT_NN, "    hex  PMCZs139fS Q s NB");
	emui_label(sreg, 0, 3, 55, AL_LEFT, S_TEXT_NN, "SR:");
	emui_lineedit(sreg, -1, 4, 3, 4, 4, TT_HEX, M_OVR);
	emui_lineedit(sreg, -1, 9, 3, 10, 10, TT_BIN, M_OVR);
	emui_lineedit(sreg, -1, 20, 3, 1, 1, TT_BIN, M_OVR);
	emui_lineedit(sreg, -1, 22, 3, 1, 1, TT_BIN, M_OVR);
	emui_lineedit(sreg, -1, 24, 3, 4, 4, TT_BIN, M_OVR);
	emui_label(sreg, 0, 4, 55, AL_LEFT, S_TEXT_NN, "KB:");
	emui_lineedit(sreg, -1, 4, 4, 4, 4, TT_HEX, M_OVR);
	emui_lineedit(sreg, -1, 9, 4, 16, 16, TT_BIN, M_OVR);

	emui_tile_set_focus_key(sreg, 's');

	return sreg;
}

// -----------------------------------------------------------------------
static int dasm_update(struct emui_tile *t)
{
	char *buf = malloc(t->i.w + 1);
	char *dbuf;
	int pos = 0;
	uint16_t addr;
	int astyle;
	int istyle;

	emui_textview_clear(t);

	while (pos < t->i.h) {
		addr = dasm_start + pos;
		emdas_dasm(emd, dasm_segment, addr);
		dbuf = emdas_get_buf(emd);

		astyle = S_DEFAULT;

		if (addr == 16) {
			istyle = S_TEXT_FN;
			astyle = S_TEXT_FN;
		} else if (*dbuf == '.') {
			istyle = S_EDIT_NN;
		} else if (*dbuf == ';') {
			istyle = S_TEXT_NI;
			dbuf += 2;
		} else {
			istyle = S_YELLOW;
		}

		sprintf(buf, "0x%04x: ", addr);
		emui_textview_append(t, astyle, buf);
		sprintf(buf, "%-*s", t->i.w-8, dbuf);
		emui_textview_append(t, istyle, buf);

		pos++;
	}
	free(buf);
	return 0;
}

// -----------------------------------------------------------------------
int dasm_key_handler(struct emui_tile *t, int key)
{
	switch (key) {
	case KEY_UP:
		dasm_start--;
		return 0;
	case KEY_DOWN:
		dasm_start++;
		return 0;
	case KEY_PPAGE:
		dasm_start -= t->i.h;
		return 0;
	case KEY_NPAGE:
		dasm_start += t->i.h;
		return 0;
	case KEY_HOME:
		dasm_start = 0;
		return 0;
	case KEY_END:
		dasm_start = 0x10000 - t->i.h;
		return 0;
	case 550: // ctrl-page up
		dasm_start = (dasm_start & 0xf000) - 0x1000;
		return 0;
	case 545: // ctrl-page down
		dasm_start = (dasm_start & 0xf000) + 0x1000;
		return 0;
	case '>':
	case '.':
		dasm_segment = (dasm_segment+1) & 0xf;
		return 0;
	case '<':
	case ',':
		dasm_segment = (dasm_segment-1) & 0xf;
		return 0;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		dasm_segment = key - '0';
		return 0;
	case 'f':
		dasm_follow ^= 1;
		return 0;
	case 'g':
		return 0;
	case 'i':
		return 0;

	}

	return 1;
}

// -----------------------------------------------------------------------
struct emui_tile * ui_create_debugger(struct emui_tile *parent)
{
	// ASM
	struct emui_tile *dasm_split = emui_splitter(parent, AL_LEFT, 30, 30, FIT_FILL);
	emui_tile_set_properties(dasm_split, P_FOCUS_GROUP);
	emui_tile_set_name(dasm_split, "Debugger");
	emui_tile_set_focus_key(dasm_split, KEY_F(12));
	struct emui_tile *dasm = emui_frame(dasm_split, 0, 0, 30, 20, "ASM", P_NONE);
	emui_tile_set_focus_key(dasm, 'a');
	struct emui_tile *asmv = emui_textview(dasm, 0, 0, 30, 20);
	emui_tile_set_properties(asmv, P_MAXIMIZED);
	emui_tile_set_key_handler(asmv, dasm_key_handler);
	emui_tile_set_update_handler(asmv, dasm_update);

	// registers
	struct emui_tile *reg_split = emui_splitter(dasm_split, AL_TOP, 11, 11, FIT_FILL);
	struct emui_tile *sreg_split = emui_splitter(reg_split, AL_LEFT, 10, FIT_DIV2, 55);
	struct emui_tile *ureg = ui_create_ureg(sreg_split);
	struct emui_tile *sreg = ui_create_sreg(sreg_split);

	// memory
	struct emui_tile *mem_split = emui_splitter(reg_split, AL_TOP, 10, FIT_DIV2, 6);
	struct emui_tile *mem = emui_frame(mem_split, 0, 0, 80, 20, "Memory", P_NONE);
	emui_tile_set_focus_key(mem, 'm');

	emui_label(mem, 1, 0, 6, AL_LEFT, S_DEFAULT, "seg 2");

	char buf[5];
	for (int i=0 ; i<32 ; i++) {
		sprintf(buf, "%x", i);
		emui_label(mem, 9+i*5, 0, 4, AL_LEFT, S_DEFAULT, buf);
	}
	struct emui_tile *mem_hline = emui_line(mem, AL_HORIZONTAL, 0, 1, 1000);
	struct emui_tile *mem_vline = emui_line(mem, AL_VERTICAL, 8, 0, 1000);

	for (int i=2 ; i<50 ; i++) {
		emui_label(mem, 0, i, 7, AL_LEFT, S_DEFAULT, "addr");
	}

	struct emui_tile *memv_cont = emui_dummy_cont(mem, 9, 2, 1000, 1000);
	struct emui_tile *memv = emui_grid(memv_cont, -1, -1, 4, 1, 1);

	for (int i=0 ; i<1000 ; i++) {
		sprintf(buf, "%x", i);
		struct emui_tile *l = emui_lineedit(memv, -1, 0, 0, 4, 4, TT_HEX, M_OVR);
		emui_lineedit_set_text(l, buf);
	}

	// eval
	struct emui_tile *eval_split = emui_splitter(mem_split, AL_BOTTOM, 3, 3, 10);
	struct emui_tile *eval = emui_frame(eval_split, 0, 0, 80, 3, "Evaluator", P_NONE);
	emui_tile_set_focus_key(eval, 'e');

	// stack, watch, brk
	struct emui_tile *stack_split = emui_splitter(eval_split, AL_LEFT, 18, 18, 20);
	struct emui_tile *stack = emui_frame(stack_split, 0, 0, 18, 10, "Stack", P_NONE);
	struct emui_tile *watch_split = emui_splitter(stack_split, AL_LEFT, 10, FIT_DIV2, 10);
	struct emui_tile *brk = emui_frame(watch_split, 0, 0, 30, 10, "Watches", P_NONE);
	emui_tile_set_focus_key(brk, 'w');
	struct emui_tile *watch = emui_frame(watch_split, 0, 0, 30, 10, "Breakpoints", P_NONE);
	emui_tile_set_focus_key(watch, 'b');

	// memory

	return dasm_split;
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	for (int seg=0 ; seg<16 ; seg++) {

	}

	for (int seg=0 ; seg<16 ; seg++) {
		mem[seg] = malloc(sizeof(uint16_t) * MAX_MEM);
		for (int addr=0 ; addr<MAX_MEM ; addr++) {
			mem[seg][addr] = rand();
		}
	}

	for (int addr=0 ; addr<MAX_MEM ; addr++) {
		mem[1][addr] = addr;
	}

	// initialize deassembler
	emd = emdas_create(EMD_ISET_MX16, mem_get);
	if (!emd) {
		exit(1);
	}
	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_NONE);
	emdas_set_tabs(emd, 0, 0, 4, 4);

	struct emui_tile *layout = emui_init(30);

	// status
	struct emui_tile *status_split = emui_splitter(layout, AL_BOTTOM, 1, 1, FIT_FILL);
	struct emui_tile *status = ui_create_status(status_split);

	// tabs
	struct emui_tile *tabs = emui_tabs(status_split);

	// terminal windows
	struct emui_tile *term1 = emui_frame(tabs, 0, 0, 80, 25, "Term 1", P_MAXIMIZED);
	emui_tile_set_focus_key(term1, KEY_F(1));
	struct emui_tile *term2 = emui_frame(tabs, 0, 0, 80, 25, "Term 2", P_MAXIMIZED);
	emui_tile_set_focus_key(term2, KEY_F(2));
	struct emui_tile *term3 = emui_frame(tabs, 0, 0, 80, 25, "Term 3", P_MAXIMIZED);
	emui_tile_set_focus_key(term3, KEY_F(3));
	struct emui_tile *term4 = emui_frame(tabs, 0, 0, 80, 25, "Term 4", P_MAXIMIZED);
	emui_tile_set_focus_key(term4, KEY_F(4));

	// device manager
	struct emui_tile *devmgr = emui_frame(tabs, 0, 0, 80, 25, "DevManager", P_MAXIMIZED);
	emui_tile_set_focus_key(devmgr, KEY_F(11));

	// debugger
	struct emui_tile *debugger = ui_create_debugger(tabs);

	emui_focus(debugger);

	emui_loop();
	emui_destroy();
	emdas_destroy(emd);

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent


