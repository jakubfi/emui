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
#include <string.h>
#include <stdlib.h>

#include "emui.h"

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

	EMTILE *dlg = emui_frame(parent, 0, 0, 24, 3, "GoTo", P_CENTER);
	emtile_set_geometry_parent(dlg, parent, GEOM_INTERNAL);

	emui_label(dlg, 1, 0, 9, S_DEFAULT, "Address: ");
	dat->le = emui_lineedit(dlg, 10, 0, 10, 10, TT_TEXT, M_INS);
	emtile_set_properties(dat->le, P_AUTOEDIT);

	emtile_set_ptr(dlg, dat);
	emtile_set_change_handler(dlg, goto_changed);
	emtile_set_key_handler(dlg, goto_key_handler);

	emui_focus(dat->le);

	// NOTE: dlg is unused by the caller
	return dlg;
}

// vim: tabstop=4 shiftwidth=4 autoindent
