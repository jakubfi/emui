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
	emui_label(split, 0, 0, 50, S_DEFAULT, "  MIPS: 22.4  STOP  ALARM  CLOCK  IRQ  Q  MC  P");

	// right side
	EMTILE *status_right = emui_label(split, 0, 0, 32, S_TEXT_NN, "??");
	emtile_set_update_handler(status_right, status_right_update);

	return split;
}

// vim: tabstop=4 shiftwidth=4 autoindent
