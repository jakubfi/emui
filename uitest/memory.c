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
EMTILE * ui_create_memory(EMTILE *parent)
{
	EMTILE *mem = emui_frame(parent, 0, 0, 80, 20, "Memory", P_MAXIMIZE);
	emtile_set_focus_key(mem, 'm');

	// memory status
	EMTILE *mem_status_split = emui_splitter(mem, AL_BOTTOM, 1, 1, 0);
	emtile_set_geometry_parent(mem_status_split, mem_status_split->parent, GEOM_EXTERNAL);

	EMTILE *mem_status_cont = emui_splitter(mem_status_split, AL_RIGHT, 24, 24, 0);
	emtile_set_margins(mem_status_cont, 0, 0, 2, 2);
	emui_label(mem_status_cont, 0, 0, 24, S_DEFAULT, "[ disp:HEX, cols:FIX16 ]");

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

	return mem;
}

// vim: tabstop=4 shiftwidth=4 autoindent
