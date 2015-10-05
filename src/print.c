#include <ncurses.h>
#include <stdarg.h>

#include "tile.h"
#include "style.h"

// -----------------------------------------------------------------------
static int __nc_vprint(WINDOW *win, int style, char *format, va_list vl)
{
	int ret;
	attr_t attr_old;
	short colorpair_old;

	wattr_get(win, &attr_old, &colorpair_old, NULL);
	wattrset(win, emui_style_get(style));
	ret = vwprintw(win, format, vl);
	wattrset(win, colorpair_old | attr_old);

	return ret;
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
	int ret;
	attr_t attr_old;
	short colorpair_old;
	WINDOW *win = t->ncdeco;
			    
	wattr_get(win, &attr_old, &colorpair_old, NULL);
	wattrset(win, emui_style_get(style));
	ret = box(win, 0, 0);
	wattrset(win, colorpair_old | attr_old);
								    
	return ret;
}

// -----------------------------------------------------------------------
int emuifillbg(struct emui_tile *t, int style)
{
	int ret;
	attr_t attr_old;
	short colorpair_old;

	WINDOW *win = t->ncwin;

	wattr_get(win, &attr_old, &colorpair_old, NULL);
	wattrset(win, emui_style_get(style));
	for (int y=0 ; y<t->h ; y++) {
		ret = whline(win, ' ', t->w);
	}
	wattrset(win, colorpair_old | attr_old);

	return ret;
}

// vim: tabstop=4 shiftwidth=4 autoindent
