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
#include <string.h>
#include <emcrk/r40.h>
#include <emdas.h>

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

#define MAX_MEM 1024*32

EMTILE *tabs;
EMTILE *help;

uint16_t treg[8];
struct emdas *emd;
uint16_t *mem[16];
uint16_t dasm_start;
int dasm_segment;
int dasm_follow;

enum app_styles {
	S_INSTRUCTION = S_FIRST_APP_STYLE,
	S_IC,
};

static struct emui_style_def app_scheme[] = {
	{ S_INSTRUCTION,	COLOR_BLACK,	COLOR_YELLOW,	A_BOLD },
	{ S_IC,				COLOR_BLUE,		COLOR_GREEN,	A_BOLD },
	{ -1, 0, 0, 0 }
};

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
int status_right_update(EMTILE *t)
{
	EMTEXT *txt = emui_label_get_emtext(t);
	emtext_clear(txt);
	emtext_append_str(txt, S_TEXT_NN, "FPS: ");
	emtext_append_str(txt, S_EDIT_NN, "%2.2f ", emui_get_current_fps());
	emtext_append_str(txt, S_TEXT_NN, " FRAME: ");
	emtext_append_str(txt, S_EDIT_NN, "%i", emui_get_current_frame());

	return E_UPDATED;
}

// -----------------------------------------------------------------------
EMTILE * ui_create_statusbar(EMTILE *parent)
{
	EMTILE *split = emui_splitter(parent, AL_LEFT, 1, FIT_FILL, 25);
	emtile_set_properties(split, P_INVERSE);

	// left side
	EMTILE *status_left = emui_label(split, 0, 0, 50, S_DEFAULT, "  MIPS: 22.4  STOP  ALARM  CLOCK  IRQ  Q  MC  P");

	// right side
	EMTILE *status_right = emui_label(split, 0, 0, 32, S_TEXT_NN, "??");
	emtile_set_update_handler(status_right, status_right_update);

	return split;
}

// -----------------------------------------------------------------------
int reg_2char_update(EMTILE *t)
{
	char buf[3];
	uint16_t *data = emtile_get_ptr(t);

	buf[0] = *data >> 8;
	buf[1] = *data & 0xff;
	buf[2] = '\0';
	emui_lineedit_set_text(t, buf);

	return E_UPDATED;
}
// -----------------------------------------------------------------------
int reg_2char_changed(EMTILE *t)
{
	char *txt;
	uint16_t *data = emtile_get_ptr(t);

	txt = emui_lineedit_get_text(t);
	*data = *txt ? (*txt << 8) + *(txt+1) : 0;

	return E_VALID;
}

// -----------------------------------------------------------------------
int reg_r40_update(EMTILE *t)
{
	char buf[4];
	uint16_t val;
	uint16_t *data = emtile_get_ptr(t);

	val = *data;
	emui_lineedit_set_text(t, r40_to_ascii(&val, 1, buf));

	return E_UPDATED;
}

// -----------------------------------------------------------------------
int reg_r40_changed(EMTILE *t)
{
	uint16_t val;
	uint16_t *data = emtile_get_ptr(t);

	ascii_to_r40(emui_lineedit_get_text(t), NULL, &val);
	*data = val;

	return E_VALID;
}

// -----------------------------------------------------------------------
int reg_int_update(EMTILE *t)
{
	uint16_t *data = emtile_get_ptr(t);
	emui_lineedit_set_int(t, *data);

	return E_UPDATED;
}

// -----------------------------------------------------------------------
int reg_int_changed(EMTILE *t)
{
	int val;

	val = emui_lineedit_get_int(t);
	uint16_t *data = emtile_get_ptr(t);

	if ((val > USHRT_MAX) || (val < SHRT_MIN)) {
		return E_INVALID;
	} else {
		*data = val;
		return E_VALID;
	}
}

// -----------------------------------------------------------------------
void float_focus(EMTILE *t, int focus)
{
	if (focus) {
		if (t->properties & P_HIDDEN) {
			emtile_set_geometry_parent(t, tabs, GEOM_INTERNAL);
			t->properties |= P_FLOAT;
		}
	} else {
		emtile_set_geometry_parent(t, t->parent, GEOM_INTERNAL);
		t->properties &= ~P_FLOAT;
	}
}

// -----------------------------------------------------------------------
EMTILE * ui_create_ureg(EMTILE *parent)
{
	EMTILE *ureg = emui_frame(parent, 0, 0, 55, 11, "Registers", P_CENTER);
	emtile_set_focus_key(ureg, 'r');
	emtile_set_focus_handler(ureg, float_focus);

	EMTILE *ureg_just = emui_justifier(ureg);
	EMTILE *ureg_r = emui_dummy_cont(ureg_just, 0, 0, 4, 11);
	EMTILE *ureg_hex = emui_dummy_cont(ureg_just, 4, 0, 4, 11);
	EMTILE *ureg_dec = emui_dummy_cont(ureg_just, 9, 0, 6, 11);
	EMTILE *ureg_oct = emui_dummy_cont(ureg_just, 16, 0, 6, 11);
	EMTILE *ureg_bin = emui_dummy_cont(ureg_just, 32, 0, 16, 11);
	EMTILE *ureg_ch = emui_dummy_cont(ureg_just, 40, 0, 2, 11);
	EMTILE *ureg_r40 = emui_dummy_cont(ureg_just, 42, 0, 3, 11);
	emui_label(ureg_r, 0, 0, 4, S_TEXT_NN, "");
	emui_label(ureg_hex, 0, 0, 4, S_TEXT_NN, "hex");
	emui_label(ureg_dec, 0, 0, 6, S_TEXT_NN, "dec");
	emui_label(ureg_oct, 0, 0, 6, S_TEXT_NN, "oct");
	emui_label(ureg_bin, 0, 0, 16, S_TEXT_NN, "ZMVCLEGYX1234567");
	emui_label(ureg_ch, 0, 0, 2, S_TEXT_NN, "ch");
	emui_label(ureg_r40, 0, 0, 3, S_TEXT_NN, "R40");

	char buf[] = "R_:";
	EMTILE *r;

	for (int i=0 ; i<8 ; i++) {
		buf[1] = '0' + i;
		emui_label(ureg_r, 0, i+1, 3, S_TEXT_NN, buf);
		treg[i] = 0;

		r = emui_lineedit(ureg_hex, 0, i+1, 4, 4, TT_HEX, M_OVR);
		emtile_set_change_handler(r, reg_int_changed);
		emtile_set_update_handler(r, reg_int_update);
		emtile_set_ptr(r, treg+i);
		emtile_set_focus_key(r, buf[1]);

		r = emui_lineedit(ureg_dec, 0, i+1, 6, 6, TT_INT, M_OVR);
		emtile_set_change_handler(r, reg_int_changed);
		emtile_set_update_handler(r, reg_int_update);
		emtile_set_ptr(r, treg+i);

		r = emui_lineedit(ureg_oct, 0, i+1, 6, 6, TT_OCT, M_OVR);
		emtile_set_change_handler(r, reg_int_changed);
		emtile_set_update_handler(r, reg_int_update);
		emtile_set_ptr(r, treg+i);

		r = emui_lineedit(ureg_bin, 0, i+1, 16, 16, TT_BIN, M_OVR);
		emtile_set_change_handler(r, reg_int_changed);
		emtile_set_update_handler(r, reg_int_update);
		emtile_set_ptr(r, treg+i);

		r = emui_lineedit(ureg_ch, 0, i+1, 2, 2, TT_TEXT, M_OVR);
		emtile_set_change_handler(r, reg_2char_changed);
		emtile_set_update_handler(r, reg_2char_update);
		emtile_set_ptr(r, treg+i);

		r = emui_lineedit(ureg_r40, 0, i+1, 3, 3, TT_TEXT, M_OVR);
		emtile_set_change_handler(r, reg_r40_changed);
		emtile_set_update_handler(r, reg_r40_update);
		emtile_set_ptr(r, treg+i);
	}

	return ureg;
}

// -----------------------------------------------------------------------
EMTILE * ui_create_sreg(EMTILE *parent)
{
	EMTILE *sreg = emui_frame(parent, 0, 0, 55, 11, "Sys Registers", P_CENTER);
	emtile_set_focus_handler(sreg, float_focus);
	emui_label(sreg, 0, 0, 55, S_TEXT_NN, "    hex  opcode D A   B   C");
	emui_label(sreg, 0, 1, 55, S_TEXT_NN, "IR:");
	emui_lineedit(sreg, 4, 1, 4, 4, TT_HEX, M_OVR);
	emui_lineedit(sreg, 9, 1, 6, 6, TT_BIN, M_OVR);
	emui_lineedit(sreg, 16, 1, 1, 1, TT_BIN, M_OVR);
	emui_lineedit(sreg, 18, 1, 3, 3, TT_BIN, M_OVR);
	emui_lineedit(sreg, 22, 1, 3, 3, TT_BIN, M_OVR);
	emui_lineedit(sreg, 26, 1, 3, 3, TT_BIN, M_OVR);
	emui_label(sreg, 0, 2, 55, S_TEXT_NN, "    hex  PMCZs139fS Q s NB");
	emui_label(sreg, 0, 3, 55, S_TEXT_NN, "SR:");
	emui_lineedit(sreg, 4, 3, 4, 4, TT_HEX, M_OVR);
	emui_lineedit(sreg, 9, 3, 10, 10, TT_BIN, M_OVR);
	emui_lineedit(sreg, 20, 3, 1, 1, TT_BIN, M_OVR);
	emui_lineedit(sreg, 22, 3, 1, 1, TT_BIN, M_OVR);
	emui_lineedit(sreg, 24, 3, 4, 4, TT_BIN, M_OVR);
	emui_label(sreg, 0, 4, 55, S_TEXT_NN, "KB:");
	emui_lineedit(sreg, 4, 4, 4, 4, TT_HEX, M_OVR);
	emui_lineedit(sreg, 9, 4, 16, 16, TT_BIN, M_OVR);

	emtile_set_focus_key(sreg, 's');

	return sreg;
}

// -----------------------------------------------------------------------
static int dasm_update(EMTILE *t)
{
	char *buf = malloc(t->i.w + 1);
	char *dbuf;
	int pos = 0;
	uint16_t addr;
	int astyle;
	int istyle;
	EMTEXT *txt = emui_textview_get_emtext(t);

	emtext_clear(txt);

	while (pos < t->i.h) {
		addr = dasm_start + pos;
		emdas_dasm(emd, dasm_segment, addr);
		dbuf = emdas_get_buf(emd);

		astyle = S_DEFAULT;

		if (addr == 16) {
			istyle = S_IC;
			astyle = S_IC;
		} else if (*dbuf == '.') {
			istyle = S_EDIT_NN;
		} else if (*dbuf == ';') {
			istyle = S_TEXT_NI;
			dbuf += 2;
		} else {
			istyle = S_INSTRUCTION;
		}

		emtext_append_str(txt, astyle, "0x%04x: ", addr);
		emtext_append_str(txt, istyle, "%-*s", t->i.w-8, dbuf);

		pos++;
	}

	free(buf);

	return E_UPDATED;
}

// -----------------------------------------------------------------------
int dasmv_key_handler(EMTILE *t, int key)
{
	switch (key) {
	case KEY_UP:
		dasm_start--;
		return E_HANDLED;
	case KEY_DOWN:
		dasm_start++;
		return E_HANDLED;
	case KEY_PPAGE:
		dasm_start -= t->i.h;
		return E_HANDLED;
	case KEY_NPAGE:
		dasm_start += t->i.h;
		return E_HANDLED;
	case KEY_HOME:
		dasm_start = 0;
		return E_HANDLED;
	case KEY_END:
		dasm_start = 0x10000 - t->i.h;
		return E_HANDLED;
	case 550: // ctrl-page up
	case '<':
	case ',':
		dasm_start = (dasm_start - 0x1000) & 0xffff;
		return E_HANDLED;
	case 545: // ctrl-page down
	case '>':
	case '.':
		dasm_start = (dasm_start + 0x1000) & 0xffff;
		return E_HANDLED;
	case KEY_RIGHT:
		dasm_segment = (dasm_segment+1) & 0xf;
		return E_HANDLED;
	case KEY_LEFT:
		dasm_segment = (dasm_segment-1) & 0xf;
		return E_HANDLED;
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
		return E_HANDLED;
	case 'f':
		dasm_follow ^= 1;
		return E_HANDLED;
	case 'i':
		// TODO: current IC
		return E_HANDLED;

	}

	return E_UNHANDLED;
}

// -----------------------------------------------------------------------
struct goto_data {
	int *seg;
	uint16_t *addr;
	EMTILE *le;
};

// -----------------------------------------------------------------------
int str_to_segaddr(char *str, int *xseg, uint16_t *xaddr)
{
	char *buf = strdup(str);
	int segi, addri;
	char *endptr;
	char *seg = buf;
	char *addr = strchr(seg, ':');

	if (addr) {
		*addr = '\0';
		addr++;
	} else {
		addr = buf;
		seg = NULL;
	}

	if (addr && *addr) {
		addri = strtol(addr, &endptr, 0);
		if (*endptr != '\0') {
			free(buf);
			return 1;
		} else {
			*xaddr = addri;
		}
	}

	if (seg && *seg) {
		segi = strtol(seg, &endptr, 0);
		if (*endptr != '\0') {
			free(buf);
			return 1;
		} else {
			*xseg = segi;
		}
	}

	free(buf);

	return E_OK;
}

// -----------------------------------------------------------------------
int goto_changed(EMTILE *t)
{
	struct goto_data *dat = emtile_get_ptr(t);
	char *str = emui_lineedit_get_text(dat->le);

	if (str_to_segaddr(str, dat->seg, dat->addr) == E_OK) {
		free(dat);
		emtile_delete(t);
		return E_VALID;
	}

	return E_INVALID;
}

// -----------------------------------------------------------------------
int goto_key_handler(EMTILE *t, int key)
{
	switch (key) {
	case 27: // ESC
		emtile_delete(t);
		return E_HANDLED;
	}
	return E_UNHANDLED;
}

// -----------------------------------------------------------------------
EMTILE * dialog_goto(EMTILE *parent, int *seg, uint16_t *addr)
{
	struct goto_data *dat = malloc(sizeof(struct goto_data));
	dat->seg = seg;
	dat->addr = addr;

	EMTILE *dlg = emui_frame(parent, 0, 0, 24, 3, "GoTo", P_NONE | P_CENTER);
	emtile_set_geometry_parent(dlg, parent, GEOM_INTERNAL);

	emui_label(dlg, 1, 0, 9, S_DEFAULT, "Address: ");
	dat->le = emui_lineedit(dlg, 10, 0, 10, 10, TT_TEXT, M_INS);
	emtile_set_properties(dat->le, P_AUTOEDIT);

	emtile_set_ptr(dlg, dat);
	emtile_set_change_handler(dlg, goto_changed);
	emtile_set_key_handler(dlg, goto_key_handler);

	emui_focus(dat->le);

	return dlg;
}

// -----------------------------------------------------------------------
int dasm_key_handler(EMTILE *t, int key)
{
	switch (key) {
	case 'g':
		dialog_goto(t, &dasm_segment, &dasm_start);
		return E_HANDLED;
	}

	return E_UNHANDLED;
}

// -----------------------------------------------------------------------
static int dasm_status_update(EMTILE *t)
{
	EMTEXT *txt = emui_label_get_emtext(t);
	emtext_clear(txt);
	emtext_append_str(txt, S_TEXT_NN, "[ IC follow:%s seg:%-2i ]", dasm_follow ? "ON " : "OFF", dasm_segment);

	return E_UPDATED;
}

// -----------------------------------------------------------------------
EMTILE * ui_create_debugger(EMTILE *parent)
{
	// ASM
	EMTILE *dasm_split = emui_splitter(parent, AL_LEFT, 30, 30, FIT_FILL);
	emtile_set_properties(dasm_split, P_FOCUS_GROUP);
	emtile_set_name(dasm_split, "Debugger");
	emtile_set_focus_key(dasm_split, KEY_F(12));
	EMTILE *dasm = emui_frame(dasm_split, 0, 0, 30, 20, "ASM", P_HCENTER|P_VMAXIMIZE);
	emtile_set_focus_key(dasm, 'a');
	emtile_set_focus_handler(dasm, float_focus);
	EMTILE *asmv = emui_textview(dasm, 0, 0, 30, 20);
	emtile_set_properties(asmv, P_MAXIMIZE);
	emtile_set_key_handler(dasm, dasm_key_handler);
	emtile_set_key_handler(asmv, dasmv_key_handler);
	emtile_set_update_handler(asmv, dasm_update);

	// asm status
	EMTILE *dasm_status_split = emui_splitter(dasm, AL_BOTTOM, 1, 1, 0);
	emtile_set_geometry_parent(dasm_status_split, dasm_status_split->parent, GEOM_EXTERNAL);
	EMTILE *dasm_status_cont = emui_splitter(dasm_status_split, AL_RIGHT, 24, 24, 0);
	emtile_set_margins(dasm_status_cont, 0, 0, 2, 2);
	EMTILE *dasm_status = emui_label(dasm_status_cont, 0, 0, 24, S_DEFAULT, "");
	emtile_set_update_handler(dasm_status, dasm_status_update);

	// registers
	EMTILE *reg_split = emui_splitter(dasm_split, AL_TOP, 11, 11, FIT_FILL);
	EMTILE *sreg_split = emui_splitter(reg_split, AL_LEFT, 10, FIT_DIV2, 55);
	EMTILE *ureg = ui_create_ureg(sreg_split);
	EMTILE *sreg = ui_create_sreg(sreg_split);

	// memory
	EMTILE *mem_split = emui_splitter(reg_split, AL_TOP, 10, FIT_DIV2, 6);
	EMTILE *mem = emui_frame(mem_split, 0, 0, 80, 20, "Memory", P_MAXIMIZE);
	emtile_set_focus_key(mem, 'm');
	emtile_set_focus_handler(mem, float_focus);

	// memory status
	EMTILE *mem_status_split = emui_splitter(mem, AL_BOTTOM, 1, 1, 0);
	emtile_set_geometry_parent(mem_status_split, mem_status_split->parent, GEOM_EXTERNAL);

	EMTILE *mem_status_cont = emui_splitter(mem_status_split, AL_RIGHT, 24, 24, 0);
	emtile_set_margins(mem_status_cont, 0, 0, 2, 2);
	EMTILE *mem_status = emui_label(mem_status_cont, 0, 0, 24, S_DEFAULT, "[ disp:HEX, cols:FIX16 ]");

	emui_label(mem, 1, 0, 6, S_DEFAULT, "seg 2");

	char buf[5];
	for (int i=0 ; i<32 ; i++) {
		sprintf(buf, "%x", i);
		emui_label(mem, 9+i*5, 0, 4, S_DEFAULT, buf);
	}
	emui_line(mem, AL_HORIZONTAL, 0, 1, 1000);
	emui_line(mem, AL_VERTICAL, 8, 0, 1000);

	for (int i=2 ; i<50 ; i++) {
		emui_label(mem, 0, i, 7, S_DEFAULT, "addr");
	}

	EMTILE *memv_cont = emui_dummy_cont(mem, 9, 2, 1000, 1000);
	EMTILE *memv = emui_grid(memv_cont, -1, -1, 4, 1, 1);

	for (int i=0 ; i<1000 ; i++) {
		sprintf(buf, "%x", i);
		EMTILE *l = emui_lineedit(memv, 0, 0, 4, 4, TT_HEX, M_OVR);
		emui_lineedit_set_text(l, buf);
	}

	// eval
	EMTILE *eval_split = emui_splitter(mem_split, AL_BOTTOM, 3, 3, 10);
	EMTILE *eval = emui_frame(eval_split, 0, 0, 80, 3, "Evaluator", P_VCENTER|P_HMAXIMIZE);
	emtile_set_focus_key(eval, 'e');
	emtile_set_focus_handler(eval, float_focus);

	// stack, watch, brk
	EMTILE *stack_split = emui_splitter(eval_split, AL_LEFT, 18, 18, 20);
	EMTILE *stack = emui_frame(stack_split, 0, 0, 18, 10, "Stack", P_CENTER);
	EMTILE *watch_split = emui_splitter(stack_split, AL_LEFT, 10, FIT_DIV2, 10);
	EMTILE *brk = emui_frame(watch_split, 0, 0, 25, 10, "Breakpoints", P_CENTER);
	emtile_set_focus_key(brk, 'b');
	emtile_set_focus_handler(brk, float_focus);
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
	emtile_set_focus_handler(watch, float_focus);
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

// -----------------------------------------------------------------------
int help_key_handler(EMTILE *t, int key)
{
	emtile_delete(help);
	return E_HANDLED;
}

// -----------------------------------------------------------------------
int top_key_handler(EMTILE *t, int key)
{
	EMTILE *help_tv;

	switch (key) {
	case 'h':
	case '?':
	case 'H':
		help = emui_frame(t, 1, 1, 60, 20, "Help", P_VMAXIMIZE | P_HCENTER);
		emtile_set_geometry_parent(help, tabs, GEOM_INTERNAL);
		emtile_set_key_handler(help, help_key_handler);
		help_tv = emui_textview(help, 0, 0, 10, 10);
		emtile_set_properties(help_tv, P_MAXIMIZE);
		emtext_append_str(emui_textview_get_emtext(help_tv), S_DEFAULT, "%s", help_text);

		emui_focus(help);
		return E_HANDLED;
	}

	return E_UNHANDLED;
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{

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
	emui_destroy();
	emdas_destroy(emd);

	return 0;
}

// vim: tabstop=4 shiftwidth=4 autoindent
