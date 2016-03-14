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

#include <stdio.h>
#include <stdarg.h>

#include "tile.h"

#ifndef DEBUG_LOG
#define DEBUG_LOG "/tmp/emui.log"
#endif

static const char tabs[] = "                                                                               ";

// -----------------------------------------------------------------------
void __prt_flags(FILE *f, EMTILE *t)
{
	fprintf(f, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		t->properties & P_HMAXIMIZE ? "HM " : "",
		t->properties & P_VMAXIMIZE ? "VM " : "",
		t->properties & P_VCENTER ? "VC " : "",
		t->properties & P_HCENTER ? "HC " : "",
		t->properties & P_HFILL ? "HF " : "",
		t->properties & P_VFILL ? "VF " : "",
		t->properties & P_FLOAT ? "F " : "",
		t->properties & P_FOCUS_GROUP ? "FG " : "",
		t->properties & P_INVERSE ? "INV " : "",
		t->properties & P_AUTOEDIT ? "AE " : "",
		t->properties & P_HIDDEN ? "H " : "",
		t->properties & P_GEOM_FORCED ? "GF " : "",
		t->properties & P_INTERACTIVE ? "I " : "",
		t->properties & P_NOCANVAS ? "NCV " : "",
		t->properties & P_CONTAINER ? "CONT " : "",
		t->properties & P_DELETED ? "DEL " : ""
	);
}

// -----------------------------------------------------------------------
void __edbg(EMTILE *t, int level, char *format, ...)
{
	va_list vl;
	va_start(vl, format);
	FILE *f = fopen(DEBUG_LOG, "a");
	fprintf(f, "%.*s", level, tabs);
	if (t) {
		fprintf(f, "[%s:%i ", t->name, t->__dbg_id);
		__prt_flags(f, t);
		fprintf(f, "] ");
	}
	vfprintf(f, format, vl);
	fprintf(f, "\n");
	fclose(f);
	va_end(vl);
}

// vim: tabstop=4 shiftwidth=4 autoindent
