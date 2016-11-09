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
EMTILE * ui_create_sreg(EMTILE *parent)
{
	EMTILE *sreg = emui_frame(parent, 0, 0, 55, 11, "Sys Registers", P_CENTER);
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

// vim: tabstop=4 shiftwidth=4 autoindent
