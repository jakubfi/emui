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
#include <stdlib.h>
#include <emdas.h>

#include "emui.h"

#define MAX_MEM 0x10000

enum app_styles {
	S_INSTRUCTION = S_FIRST_APP_STYLE,
	S_IC,
};

uint16_t *mem[16];
struct emdas *emd;
uint16_t dasm_start;
int dasm_segment;
int dasm_follow;

EMTILE * dialog_goto(EMTILE *parent, int *seg, uint16_t *addr);

// -----------------------------------------------------------------------
int mem_get(int nb, uint16_t addr, uint16_t *dest)
{
	if ((nb > 0) && (nb < 16)) {
		*dest = mem[nb][addr];
		return 1;
	} else {
		return 0;
	}
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
static int emdas_init()
{
	// initialize deassembler
	emd = emdas_create(EMD_ISET_MX16, mem_get);
	if (!emd) {
		return -1;
	}
	emdas_set_nl(emd, '\0');
	emdas_set_features(emd, EMD_FEAT_NONE);
	emdas_set_tabs(emd, 0, 0, 4, 4);

	// fake memory
	for (int seg=0 ; seg<16 ; seg++) {
		mem[seg] = malloc(sizeof(uint16_t) * MAX_MEM);
		for (int addr=0 ; addr<MAX_MEM ; addr++) {
			mem[seg][addr] = rand();
		}
	}

	for (int addr=0 ; addr<MAX_MEM ; addr++) {
		mem[1][addr] = addr;
	}

	return 0;
}

// -----------------------------------------------------------------------
void ui_destroy_dasm()
{
	emdas_destroy(emd);
	for (int seg=0 ; seg<16 ; seg++) {
		free(mem[seg]);
	}
}

// -----------------------------------------------------------------------
EMTILE * ui_create_dasm(EMTILE *parent)
{
	emdas_init();

	EMTILE *dasm = emui_frame(parent, 0, 0, 30, 20, "ASM", P_HCENTER|P_VMAXIMIZE);
	emtile_set_focus_key(dasm, 'a');
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

	return dasm;
}

// vim: tabstop=4 shiftwidth=4 autoindent
