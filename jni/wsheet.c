/*****************************************************************************
 *    Copyright (C) 2011 Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of Writer.
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License
 *    (<http://www.gnu.org/licenses/lgpl.html>) for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.	If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include "config.h"

#include <stdio.h>
#include <malloc.h>
#ifdef CONFIG_DUALCORE
#	include <pthread.h>
#endif /* CONFIG_DUALCORE */


#include "common.h"
#include "list.h"
#include "nmp.h"
#include "div.h"
#include "wsheet.h"
#include "g2d.h"
#include "d2d.h"

/*
 * Flag to check that this wsheet static values are initialized or not
 */
static bool _initialized = false;

#ifdef CONFIG_MEMPOOL
struct mp* g_wsheet_nmp = NULL; /* node memory pool */
#endif /* CONFIG_MEMPOOL */

static void
_init(void) {
	wassert(!_initialized);
	_initialized = true;

	nmp_create(256 * 1024);
}

static void _deinit(void) __attribute__((unused));
static void
_deinit(void) {
	if (!_initialized)
		return;
	_initialized = false;

	nmp_destroy();
}


static inline struct node*
_node(struct list_link* link) {
	return container_of(link, struct node, lk);
}

/********************************
 * Functions for struct wsheet
 ********************************/

/*
 * get division where line should be assigned to
 */
static inline struct div*
_wsheet_div(struct wsheet* wsh, struct line* ln) {
	/*
	 * NOTE !!
	 * division should be calculated based on minimum value!!
	 * This algorithm is tightly coupled with "split" algorithm.
	 * We split from smaller value to larger value
	 *   and we include point on the line...
	 * That's why we should consider only smaller value!
	 */
	int32_t c, r;
	c = ln->x0 / wsh->divW;
	if (ln->y0 < ln->y1)
		r = ln->y0 / wsh->divH;
	else
		r = ln->y1 / wsh->divH;
	return &wsh->divs[r][c];
}

static inline bool
_wsheet_splitX(struct wsheet* wsh,
	       int32_t* out_intersecty,
	       struct line* ln, int32_t x) {
	return splitX(out_intersecty, ln, x, 0, wsh->divH * wsh->rowN);
}


static inline bool
_wsheet_splitY(struct wsheet* wsh,
	       int32_t* out_intersectx,
	       struct line* ln, int32_t y) {
	return splitY(out_intersectx, ln, y, 0, wsh->divW * wsh->colN);
}





/*
 * find lines in the given rectangle division group
 * (bi and ri are NOT included)
 */
static inline void
_wsheet_find_lines_(struct wsheet* wsh, struct list_link* out,
		    int32_t ti, int32_t bi, int32_t li, int32_t ri) {
	int32_t i, j;
	/* fully included division - we don't need to check. */
	for (i = ti; i < bi; i++)
		for (j = li; j < ri; j++)
			wlist_add_list(out, &wsh->divs[i][j].lns);
}

#if defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM)

struct _wsheet_find_lines_arg {
	struct wsheet*     wsh;
	struct list_link*  out;
	int32_t            ti, bi, li, ri;
};

static void*
_wsheet_find_lines_worker(void* arg) {
	struct _wsheet_find_lines_arg* a = arg;
	_wsheet_find_lines_(a->wsh, a->out,
			    a->ti, a->bi, a->li, a->ri);
	return NULL;
}

#endif /* defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM) */



/*
 * NOTE!!!:
 *   Lines that is included in the *rectangle will be returned.
 *   *rectange includes left/top line, but excludes right/bottom line.
 * returned line object should not be modified!!
 * out_keep : all list link should be preserved.
 * out_new : all list link should be deleted.
 */
void
wsheet_find_lines(struct wsheet* wsh, struct list_link* out,
		  int32_t l, int32_t t, int32_t r, int32_t b) {

#define __horizontal_strip()						\
	do {								\
		/* horizontal strip */					\
		for (i = li; i <= ri; i++) {				\
			/* top strip */					\
			div_find_lines(&wsh->divs[ti][i], out, l, t, r, b); \
			if (bi > ti) {					\
				/* bottom strip */			\
				div_find_lines(&wsh->divs[bi][i], out,	\
					       l, t, r, b);		\
			}						\
		}							\
	} while (0)

#define __vertical_strip()						\
	do {								\
		/* vertical strip except for 4-vertex-position in rect. */ \
		for (i = ti + 1; i < bi; i++) {				\
			/* left strip */				\
			div_find_lines(&wsh->divs[i][li], out, l, t, r, b); \
			if (ri > li) {					\
				/* right strip */			\
				div_find_lines(&wsh->divs[i][ri], out,	\
					       l, t, r, b);		\
			}						\
		}							\
	} while (0)


	int32_t i;
	/* left index */
	int32_t li = l / wsh->divW;
	/* top index */
	int32_t ti = t / wsh->divH;
	/* right index (exclude right line); */
	int32_t ri = (0 == r % wsh->divW)? r / wsh->divW - 1: r / wsh->divW;
	/* bottom index (exclude bottom line); */
	int32_t bi = (0 == b % wsh->divH)? b / wsh->divH - 1: b / wsh->divH;

	if (r <= l || b <= t)
		return; /* empty rectangle.. nothing to do... */


#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_check_start(DBG_PERF_FIND_LINE);
#endif /* CONFIG_DBG_STATISTICS */

#if defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM)
	{ /* just scope */
		pthread_t                     thd;
		void*                         ret;
		struct _wsheet_find_lines_arg arg;
		struct list_link              lns;

		list_init_link(&lns);

		arg.wsh = wsh;
		arg.out = &lns;
		arg.ti  = ti + 1;
		arg.bi  = bi;
		arg.li  = li + 1;
		arg.ri  = ri;

		if (pthread_create(&thd, NULL,
				   &_wsheet_find_lines_worker, &arg))
			wassert(0);

		/* Not fully included division. - we need to check */

		__horizontal_strip();
		__vertical_strip();

		if (pthread_join(thd, &ret))
			wassert(0);

		_list_absorb_list(out, &lns);

	} /* just scope */
#else /* defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM) */

	/* Not fully included division. - we need to check */

	__horizontal_strip();
	__vertical_strip();
	_wsheet_find_lines_(wsh, out, ti + 1, bi, li + 1, ri);


#endif /* defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM) */

#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_check_end(DBG_PERF_FIND_LINE);
#endif /* CONFIG_DBG_STATISTICS */


#undef __vertical_strip
#undef __horizontal_strip
}

#ifdef CONFIG_DUALCORE
struct _draw_arg {
	struct list_link* head;
	struct list_link* lk;
	int32_t*          pixels;
	int32_t           w, h, ox, oy;
	float             zf;
};

static void
_draw_(struct list_link* head,
       struct list_link* lk,
       int32_t* pixels,
       int32_t w, int32_t h, int32_t ox, int32_t oy,
       float zf) {
	struct line* ln;

	while (lk != head) {
		ln = _node(lk)->ln;
		draw_line(pixels,
			  w, h,
			  _rbg16to32(ln->color),
			  _round_off(zf * (float)(ln->thick)),
			  _round_off(zf * (float)(ln->x0 - ox)),
			  _round_off(zf * (float)(ln->y0 - oy)),
			  _round_off(zf * (float)(ln->x1 - ox)),
			  _round_off(zf * (float)(ln->y1 - oy)));

		if (lk->_next != head)
			/* use next's next! (ex. even/odd node for dualcore) */
			lk = lk->_next->_next;
		else
			break;
	}
}

static void*
_draw_worker(void* arg) {
	struct _draw_arg* a = arg;
	_draw_(a->head, a->lk, a->pixels, a->w, a->h, a->ox, a->oy, a->zf);
	return NULL;
}

#endif /* CONFIG_DUALCORE */

/* ======================
 * Interface functions
 * ======================*/

/*
 * For debugging system.
 */
void wsys_deinit(void) __attribute__((unused));
void
wsys_deinit(void) {
	_deinit();
}

struct wsheet*
wsheet_create(void) {
	struct wsheet* sh = wmalloc(sizeof(*sh));

	if (!_initialized)
		_init();

	return sh;
}

void
wsheet_init(struct wsheet* wsh, int32_t divW, int32_t divH, int32_t colN, int32_t rowN) {
	int32_t i, j;

	wsh->divW = divW;
	wsh->divH = divH;
	wsh->colN = colN;
	wsh->rowN = rowN;
	wsh->divs = wmalloc(sizeof(*wsh->divs) * rowN);
	for (i = 0; i < rowN; i++) {
		wsh->divs[i] = wmalloc(sizeof(*wsh->divs[i]) * colN);
		for (j = 0; j < colN; j++) {
			div_init(&wsh->divs[i][j],
				 j * divW,
				 i * divH,
				 (j + 1) * divW,
				 (i + 1) * divH);
		}
	}
}

void
wsheet_destroy(struct wsheet* wsh) {
	int32_t i,j;
	for (i = 0; i < wsh->rowN; i++) {
		for (j = 0; j < wsh->colN; j++)
			div_clean(&wsh->divs[i][j]);
		wfree(wsh->divs[i]);
	}
	wfree(wsh->divs);
	wfree(wsh);
}

void
wsheet_cutout(struct wsheet* wsh, int32_t l, int32_t t, int32_t r, int32_t b) {
	struct list_link   lns;
	struct line*       oln; /* original line */
	struct line*       ln;
	struct line*       tmpl;
	int32_t            intersect;
	struct node*       n;

	list_init_link(&lns);

	wsheet_find_lines(wsh, &lns, l, t, r, b);

	list_foreach_item(n, &lns, struct node, lk) {
		ln = oln = n->ln;

		/* check for all for edge. */
		if (splitX(&intersect, ln, l, t, b)) {
			/* NOTE: X-sorted */
			tmpl = wmalloc(sizeof(*tmpl));
			line_set(tmpl, ln->x0, ln->y0, l, intersect);
			tmpl->thick = oln->thick;
			tmpl->color = oln->color;
			div_add(oln->div, tmpl);
			line_set(ln, l, intersect, ln->x1, ln->y1);
		}
		if (splitX(&intersect, ln, r, t, b)) {
			/* NOTE X-sorted */
			tmpl = wmalloc(sizeof(*tmpl));
			line_set(tmpl, ln->x1, ln->y1, r, intersect);
			tmpl->thick = oln->thick;
			tmpl->color = oln->color;
			div_add(oln->div, tmpl);
			line_set(ln, r, intersect, ln->x0, ln->y0);
		}
		if (splitY(&intersect, ln, t, l, r)) {
			tmpl = wmalloc(sizeof(*tmpl));
			tmpl->thick = oln->thick;
			tmpl->color = oln->color;
			if (ln->y0 < ln->y1) {
				line_set(tmpl, ln->x0, ln->y0, intersect, t);
				line_set(ln, ln->x1, ln->y1, intersect, t);
			} else {
				line_set(tmpl, ln->x1, ln->y1, intersect, t);
				line_set(ln, ln->x0, ln->y0, intersect, t);
			}
			div_add(oln->div, tmpl);
		}
		if (splitY(&intersect, ln, b, l, r)) {
			tmpl = wmalloc(sizeof(*tmpl));
			tmpl->thick = oln->thick;
			tmpl->color = oln->color;
			if (ln->y0 < ln->y1) {
				line_set(tmpl, ln->x1, ln->y1, intersect, b);
				line_set(ln, ln->x0, ln->y0, intersect, b);
			} else {
				line_set(tmpl, ln->x0, ln->y0, intersect, b);
				line_set(ln, ln->x1, ln->y1, intersect, b);
			}
			div_add(oln->div, tmpl);
		}

		/* free link */
		list_del(oln->divlk);

		nmp_free(_node(oln->divlk));
		wfree(oln);
	}

	wlist_clean(&lns);
}

void
wsheet_add(struct wsheet* wsh,
	   int32_t x0, int32_t y0,
	   int32_t x1, int32_t y1,
	   uint8_t thick,
	   uint16_t color) {

	struct list_link   list;
	struct list_link   splits;
	struct node*       n;
	struct line*       l;
	struct line*       tmpl;
	int32_t	           v, intersect, i;
	int32_t            vmin, vmax; /* value min/max */

	list_init_link(&list);
	list_init_link(&splits);

	/*
	 * NOTE :
	 *     check it here??? ==> should be checked in java!!
	 *     x0, y0 ... value should not be larger than LIMIT
	 *	 of unsigned short!!!
	 */
	l = wmalloc(sizeof(*l));
	line_set(l, x0, y0, x1, y1);
	l->thick = thick;
	l->color = color;

	/* check horizontal - row - split */
	if (y0 < y1) {
		vmin = y0 / wsh->divH;
		vmax = y1 / wsh->divH;
	} else {
		vmin = y1 / wsh->divH;
		vmax = y0 / wsh->divH;
	}

	for (i=vmin; i <= vmax; i++) {
		v = (i + 1) * wsh->divH;
		if (!_wsheet_splitY(wsh, &intersect, l, v)) {
			wlist_add_line(&list, l);
			break;
		}

		/* keep doing with remained one! */
		tmpl = wmalloc(sizeof(*tmpl));
		if (l->y0 < l->y1) {
			line_set(tmpl, l->x0, l->y0, intersect, v);
			line_set(l, intersect, v, l->x1, l->y1);
		} else {
			line_set(tmpl, l->x1, l->y1, intersect, v);
			line_set(l, intersect, v, l->x0, l->y0);
		}
		tmpl->thick = thick;
		tmpl->color = color;
		wlist_add_line(&list, tmpl);
	}

	/* check vertical - column - split */
	list_foreach_item(n, &list, struct node, lk) {
		l = n->ln;

		vmin = l->x0 / wsh->divW;
		vmax = l->x1 / wsh->divW;

		for (i = vmin; i <= vmax; i++) {
			v = (i + 1) * wsh->divW;
			if (!_wsheet_splitX(wsh, &intersect, l, v)) {
				wlist_add_line(&splits, l);
				break;
			}

			tmpl = wmalloc(sizeof(*tmpl));
			tmpl->thick = thick;
			tmpl->color = color;
			line_set(tmpl, l->x0, l->y0, v, intersect);
			wlist_add_line(&splits, tmpl);
			line_set(l, v, intersect, l->x1, l->y1);
		}
	}
	wlist_clean(&list);

	/* assign to division */
	list_foreach_item(n, &splits, struct node, lk) {
		div_add(_wsheet_div(wsh, n->ln), n->ln);
	}
	wlist_clean(&splits);
}

void
wsheet_draw(struct wsheet* wsh,
	    int32_t* pixels,
	    int32_t w, int32_t h,
	    int32_t ox, int32_t oy,
	    int32_t l, int32_t t, int32_t r, int32_t b,
	    float zf) {
	struct list_link   lns;

#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_check_start(DBG_PERF_DRAW_LINE);
#endif /* CONFIG_DBG_STATISTICS */

	list_init_link(&lns);

	wsheet_find_lines(wsh, &lns, l, t, r, b);


#ifdef CONFIG_DUALCORE

	{ /* just scope */
		pthread_t             thd;
		void*                 ret;
		struct _draw_arg      arg;

		arg.head   = &lns;
		arg.lk     = lns._next;
		arg.pixels = pixels;
		arg.w      = w;
		arg.h      = h;
		arg.ox     = ox;
		arg.oy     = oy;
		arg.zf     = zf;

		if (pthread_create(&thd, NULL, &_draw_worker, &arg))
			wassert(0);

		_draw_(&lns, lns._next->_next, pixels, w, h, ox, oy, zf);

		if (pthread_join(thd, &ret))
			wassert(0);

	} /* just scope */

#else /* CONFIG_DUALCORE */

	{ /* just scope */
		struct node*       n;
		struct line*       ln;

		list_foreach_item(n, &lns, struct node, lk) {
			ln = n->ln;
			draw_line(pixels,
				  w, h,
				  _rbg16to32(ln->color),
				  _round_off(zf * (float)(ln->thick)),
				  _round_off(zf * (float)(ln->x0 - ox)),
				  _round_off(zf * (float)(ln->y0 - oy)),
				  _round_off(zf * (float)(ln->x1 - ox)),
				  _round_off(zf * (float)(ln->y1 - oy)));
		}
	} /* just scope */

#endif /* CONFIG_DUALCORE */

	wlist_clean(&lns);

#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_check_end(DBG_PERF_DRAW_LINE);
#endif /* CONFIG_DBG_STATISTICS */

}
