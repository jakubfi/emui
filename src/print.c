#include <ncurses.h>
#include <stdarg.h>

#include "tile.h"
#include "style.h"

// -----------------------------------------------------------------------
static inline int __nc_vprint(WINDOW *win, int style, char *format, va_list vl)
{
	wattrset(win, emui_style_get(style));
	return vwprintw(win, format, vl);
}

// -----------------------------------------------------------------------
int vemuiprt(struct emui_tile *t, int style, char *format, va_list vl)
{
	return __nc_vprint(t->ncwin, style, format, vl);
}

// -----------------------------------------------------------------------
int vemuidprt(struct emui_tile *t, int style, char *format, va_list vl)
{
	return __nc_vprint(t->ncdeco, style, format, vl);
}

// -----------------------------------------------------------------------
int vemuixyprt(struct emui_tile *t, unsigned x, unsigned y, int style, char *format, va_list vl)
{
	wmove(t->ncwin, y, x);
	return __nc_vprint(t->ncwin, style, format, vl);
}

// -----------------------------------------------------------------------
int vemuixydprt(struct emui_tile *t, unsigned x, unsigned y, int style, char *format, va_list vl)
{
	wmove(t->ncdeco, y, x);
	return __nc_vprint(t->ncdeco, style, format, vl);
}

// -----------------------------------------------------------------------
int emuiprt(struct emui_tile *t, int style, char *format, ...)
{
	int ret;
	va_list vl;

	va_start(vl, format);
	ret = __nc_vprint(t->ncwin, style, format, vl);
	va_end(vl);

	return ret;
}

// -----------------------------------------------------------------------
int emuidprt(struct emui_tile *t, int style, char *format, ...)
{
	int ret;
	va_list vl;

	va_start(vl, format);
	ret = __nc_vprint(t->ncdeco, style, format, vl);
	va_end(vl);

	return ret;
}

// -----------------------------------------------------------------------
int emuixyprt(struct emui_tile *t, unsigned x, unsigned y, int style, char *format, ...)
{
	int ret;
	va_list vl;

	va_start(vl, format);
	wmove(t->ncwin, y, x);
	ret = __nc_vprint(t->ncwin, style, format, vl);
	va_end(vl);

	return ret;
}

// -----------------------------------------------------------------------
int emuixydprt(struct emui_tile *t, unsigned x, unsigned y, int style, char *format, ...)
{
	int ret;
	va_list vl;

	va_start(vl, format);
	wmove(t->ncdeco, y, x);
	ret = __nc_vprint(t->ncdeco, style, format, vl);
	va_end(vl);

	return ret;
}

// -----------------------------------------------------------------------
int emuixy(struct emui_tile *t, int x, int y)
{
	return wmove(t->ncwin, y, x);
}

// -----------------------------------------------------------------------
int emuixyd(struct emui_tile *t, int x, int y)
{
	return wmove(t->ncdeco, y, x);
}

// -----------------------------------------------------------------------
int emuidbox(struct emui_tile *t, int style)
{
	wattrset(t->ncdeco, emui_style_get(style));
	return box(t->ncdeco, 0, 0);
}

// -----------------------------------------------------------------------
int emuifillbg(struct emui_tile *t, int style)
{
	wbkgd(t->ncwin, emui_style_get(style));
	return 1;
}

// -----------------------------------------------------------------------
int emuihline(struct emui_tile *t, int x, int y, int len, int style)
{
	wattrset(t->ncwin, emui_style_get(style));
	return mvwhline(t->ncwin, y, x, 0, len);
}

// -----------------------------------------------------------------------
int emuivline(struct emui_tile *t, int x, int y, int len, int style)
{
	wattrset(t->ncwin, emui_style_get(style));
	return mvwvline(t->ncwin, y, x, 0, len);
}

// vim: tabstop=4 shiftwidth=4 autoindent
