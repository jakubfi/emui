#include <ncurses.h>
#include <stdarg.h>

#include "tile.h"
#include "style.h"

// -----------------------------------------------------------------------
static int _tilestyle(EMTILE *t, int style)
{
	int w_inv = t->properties & P_INVERSE ? A_REVERSE : 0;
	return emui_style_get(style) ^ w_inv;
}

// -----------------------------------------------------------------------
static inline int __nc_vprint(EMTILE *t, int style, char *format, va_list vl)
{
	wattrset(t->ncwin, _tilestyle(t, style));
	return vwprintw(t->ncwin, format, vl);
}

// -----------------------------------------------------------------------
int vemuiprt(EMTILE *t, int style, char *format, va_list vl)
{
	return __nc_vprint(t, style, format, vl);
}

// -----------------------------------------------------------------------
int vemuixyprt(EMTILE *t, unsigned x, unsigned y, int style, char *format, va_list vl)
{
	wmove(t->ncwin, y, x);
	return __nc_vprint(t, style, format, vl);
}

// -----------------------------------------------------------------------
int emuiprt(EMTILE *t, int style, char *format, ...)
{
	int ret;
	va_list vl;

	va_start(vl, format);
	ret = __nc_vprint(t, style, format, vl);
	va_end(vl);

	return ret;
}

// -----------------------------------------------------------------------
int emuixyprt(EMTILE *t, unsigned x, unsigned y, int style, char *format, ...)
{
	int ret;
	va_list vl;

	va_start(vl, format);
	wmove(t->ncwin, y, x);
	ret = __nc_vprint(t, style, format, vl);
	va_end(vl);

	return ret;
}

// -----------------------------------------------------------------------
int emuixy(EMTILE *t, int x, int y)
{
	return wmove(t->ncwin, y, x);
}

// -----------------------------------------------------------------------
int emuibox(EMTILE *t, int style)
{
	wattrset(t->ncwin, _tilestyle(t, style));
	return box(t->ncwin, 0, 0);
}

// -----------------------------------------------------------------------
int emuifillbg(EMTILE *t, int style)
{
	wbkgd(t->ncwin, _tilestyle(t, style));
	return 1;
}

// -----------------------------------------------------------------------
int emuihline(EMTILE *t, int x, int y, int len, int style)
{
	wattrset(t->ncwin, _tilestyle(t, style));
	return mvwhline(t->ncwin, y, x, 0, len);
}

// -----------------------------------------------------------------------
int emuivline(EMTILE *t, int x, int y, int len, int style)
{
	wattrset(t->ncwin, _tilestyle(t, style));
	return mvwvline(t->ncwin, y, x, 0, len);
}

// vim: tabstop=4 shiftwidth=4 autoindent
