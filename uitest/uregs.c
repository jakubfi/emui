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

#include "emui.h"

uint16_t treg[8];

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
EMTILE * ui_create_ureg(EMTILE *parent)
{
	EMTILE *ureg = emui_frame(parent, 0, 0, 55, 11, "Registers", P_CENTER);
	emtile_set_focus_key(ureg, 'r');

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
		emtile_set_properties(r, P_AUTOEDIT);

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

// vim: tabstop=4 shiftwidth=4 autoindent
