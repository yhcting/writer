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

#include <stdint.h>
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

/*
 * Utility functions
 */

/*
 * get division index.
 * -1 means invalid. (index should be >= 0)
 */
static inline int32_t
_divI(int32_t v, uint32_t divV) {
	int32_t V = (int32_t)divV;
	if (!(v % V))
		return v / V;
	else if (v < 0)
		return (v - V) / V;
	else
		return v / V;
}


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

static inline bool
_wsheet_is_contains(const struct wsheet* wsh, int32_t x, int32_t y) {
	return !(x < 0 || x >= wsh->divW * wsh->colN
		 || y < 0 || y >= wsh->divH * wsh->rowN);
}

/*
 * get division where line should be assigned to
 */
static inline struct div*
_wsheet_div_line(const struct wsheet* wsh, struct line* ln) {
	/*
	 * NOTE !!
	 * (x0, y0) is closed, but (x1, y1) is open.
	 * So, division should be calculated based on (x0, y0) of line!!
	 */
	int32_t c, r;
	c = _divI(ln->x0, wsh->divW);
	r = _divI(ln->y0, wsh->divH);
	return 	(c >= 0 && r >= 0)? &wsh->divs[r][c]: NULL;
}

/*
 * find lines in the given rectangle division group
 * (bi and ri are NOT included)
 */
static inline void
_wsheet_find_lines_(const struct wsheet* wsh, struct list_link* out,
		    int32_t ti, int32_t bi, int32_t li, int32_t ri) {
	int32_t i, j;
	/* fully included division - we don't need to check. */
	for (i = ti; i < bi; i++)
		for (j = li; j < ri; j++)
			nlist_add_nlist(out, &wsh->divs[i][j].lns);
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
EXTERN_UT_ONLY void
wsheet_find_lines(const struct wsheet* wsh, struct list_link* out,
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
	int32_t li = _divI(l, wsh->divW);
	/* top index */
	int32_t ti = _divI(t, wsh->divH);
	/* right index (exclude right line); */
	int32_t ri = (0 == r % wsh->divW)? r / wsh->divW - 1: _divI(r, wsh->divW);
	/* bottom index (exclude bottom line); */
	int32_t bi = (0 == b % wsh->divH)? b / wsh->divH - 1: _divI(b, wsh->divH);

	if (r <= l || b <= t)
		return; /* empty rectangle.. nothing to do... */

	if (li < 0)
		li = 0;
	if (ti < 0)
		ti = 0;
	if (ri >= wsh->colN)
		ri = wsh->colN - 1;
	if (bi >= wsh->rowN)
		bi = wsh->rowN - 1;


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
		ln = _node(lk)->v;
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
EXTERN_UT_ONLY void wsys_deinit(void) __attribute__((unused));
EXTERN_UT_ONLY void
wsys_deinit(void) {
	_deinit();
}

struct wsheet*
wsheet_create(void) {
	struct wsheet* sh = wmalloc(sizeof(*sh));
	wassert(sh);

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
wsheet_cutout_lines(struct wsheet* wsh,
		    int32_t l, int32_t t, int32_t r, int32_t b) {
	struct list_link   lns;
	struct line*       ln;
	struct line*       tmpl;
	int32_t            intersect;
	struct node*       n;

	list_init_link(&lns);

	wsheet_find_lines(wsh, &lns, l, t, r, b);

	list_foreach_item(n, &lns, struct node, lk) {
		ln = n->v;

		/*
		 * We don't care about below case
		 *   line is parallel with rectangle edge.
		 * So, line this is subset of rectangle edge is not removed!
		 * FIXME
		 *   Above case should be handled...
		 *   But that makes code complex.
		 *   So, let's skip at this moment.
		 */

		/* check for all for edge. */
		if (intersectX(&intersect, ln, l, t, b)) {
			tmpl = wmalloc(sizeof(*tmpl));
			tmpl->thick = ln->thick;
			tmpl->color = ln->color;

			if (ln->x0 < ln->x1) {
				line_set(tmpl, ln->x0, ln->y0, l, intersect);
				line_set(ln, l, intersect, ln->x1, ln->y1);
			} else {
				line_set(tmpl, l, intersect, ln->x1, ln->y1);
				line_set(ln, ln->x0, ln->y0, l, intersect);
			}
			/*
			 * order of lines also should be preserved
			 *   at division!
			 */
			div_add_line_prev(container_of(ln->divlk,
						       struct node,
						       lk),
					  tmpl);
		}
		if (intersectX(&intersect, ln, r, t, b)) {
			tmpl = wmalloc(sizeof(*tmpl));
			tmpl->thick = ln->thick;
			tmpl->color = ln->color;
			if (ln->x0 < ln->x1) {
				line_set(ln, ln->x0, ln->y0, r, intersect);
				line_set(tmpl, r, intersect, ln->x1, ln->y1);
			} else {
				line_set(ln, r, intersect, ln->x1, ln->y1);
				line_set(tmpl, ln->x0, ln->y0, r, intersect);
			}
			/*
			 * order of lines also should be preserved
			 *   at division!
			 */
			div_add_line_prev(container_of(ln->divlk,
						       struct node,
						       lk),
					  tmpl);
		}
		if (intersectY(&intersect, ln, t, l, r)) {
			tmpl = wmalloc(sizeof(*tmpl));
			tmpl->thick = ln->thick;
			tmpl->color = ln->color;
			if (ln->y0 < ln->y1) {
				line_set(tmpl, ln->x0, ln->y0, intersect, t);
				line_set(ln, intersect, t, ln->x1, ln->y1);
			} else {
				line_set(tmpl, intersect, t, ln->x1, ln->y1);
				line_set(ln, ln->x0, ln->y0, intersect, t);
			}
			/*
			 * order of lines also should be preserved
			 *   at division!
			 */
			div_add_line_prev(container_of(ln->divlk,
						       struct node,
						       lk),
					  tmpl);
		}
		if (intersectY(&intersect, ln, b, l, r)) {
			tmpl = wmalloc(sizeof(*tmpl));
			tmpl->thick = ln->thick;
			tmpl->color = ln->color;
			if (ln->y0 < ln->y1) {
				line_set(tmpl, intersect, b, ln->x1, ln->y1);
				line_set(ln, ln->x0, ln->y0, intersect, b);
			} else {
				line_set(tmpl, ln->x0, ln->y0, intersect, b);
				line_set(ln, intersect, b, ln->x1, ln->y1);
			}
			/*
			 * order of lines also should be preserved
			 *   at division!
			 */
			div_add_line_prev(container_of(ln->divlk,
						       struct node,
						       lk),
					  tmpl);
		}

		/* free link */
		list_del(ln->divlk);

		nmp_free(_node(ln->divlk));
		wfree(ln);
	}

	nlist_clean(&lns);
}

void
wsheet_add_line(struct wsheet* wsh,
		int32_t x0, int32_t y0,
		int32_t x1, int32_t y1,
		uint8_t thick,
		uint16_t color) {

	struct list_link   list;
	struct list_link   splits;
	struct node*       n;
	struct line*       l;
	struct line*       tmpl;
	int32_t	           v, i, ip; /* ip : Intersect Point */
	int32_t            vstart, vend, vstep; /* value min/max */

	list_init_link(&list);
	list_init_link(&splits);

	/* check invalid line data */
	if (x0 == x1 && y0 == y1)
		return;

	l = wmalloc(sizeof(*l));
	wassert(l);
	line_set(l, x0, y0, x1, y1);
	l->thick = thick;
	l->color = color;

	/* check horizontal - row - split */
	vstart = _divI(y0, wsh->divH);
	vend   = _divI(y1, wsh->divH);
	vstep  = (l->y0 < l->y1)? 1: -1;

	wassert((vstep > 0 && vstart <= vend) ||
		(vstep < 0 && vstart >= vend));
	for (i = vstart; i != vend + vstep; i += vstep) {
		v = (vstep > 0)? (i + 1) * wsh->divH: i * wsh->divH - 1;

		/* keep doing with remained one! */
		tmpl = wmalloc(sizeof(*tmpl));
		wassert(tmpl);
		tmpl->thick = thick;
		tmpl->color = color;
		if (1 == intersectY(&ip, l, v, INT32_MIN, INT32_MAX)) {
			line_set(tmpl, l->x0, l->y0, ip, v);
			line_set(l, ip, v, l->x1, l->y1);
		} else {
			/*
			 * parallel case should be handled
			 *   with non-intersection case
			 */
			nlist_add(&list, l);
			wfree(tmpl);
			break; /* exit loop */
		}

		if (i >= 0 && i < wsh->rowN)
			nlist_add(&list, tmpl);
		else
			wfree(tmpl);
	}

	/* check vertical - column - split */
	list_foreach_item(n, &list, struct node, lk) {
		l = n->v;

		vstart = _divI(l->x0, wsh->divW);
		vend   = _divI(l->x1, wsh->divW);
		vstep  = (l->x0 < l->x1)? 1: -1;

		wassert((vstep > 0 && vstart <= vend) ||
			(vstep < 0 && vstart >= vend));
		for (i = vstart; i != vend + vstep; i += vstep) {
			v = (vstep > 0)?
				(i + 1) * wsh->divW:
				i * wsh->divW - 1;

			/* keep doing with remained one! */
			tmpl = wmalloc(sizeof(*tmpl));
			wassert(tmpl);
			tmpl->thick = thick;
			tmpl->color = color;
			if (1 == intersectX(&ip, l, v, INT32_MIN, INT32_MAX)) {
				line_set(tmpl, l->x0, l->y0, v, ip);
				line_set(l, v, ip, l->x1, l->y1);
			} else {
				/*
				 * parallel case should be handled
				 *   with non-intersection case
				 */
				nlist_add(&splits, l);
				wfree(tmpl);
				break;
			}
			if (i >= 0 && i < wsh->colN)
				nlist_add(&splits, tmpl);
			else
				wfree(tmpl);
		}
	}
	nlist_clean(&list);

	/* assign to division */
	list_foreach_item(n, &splits, struct node, lk) {
		struct div* dv = _wsheet_div_line(wsh, n->v);
		if (dv && !line_is_empty(n->v))
			div_add_line(dv, n->v);
		else
			/* there is no div to add */
			wfree(n->v); /* free line object */
	}
	nlist_clean(&splits);
}

bool
wsheet_add_obj(struct wsheet* wsh,
	       uint16_t type, void* data,
	       int32_t l, int32_t t, int32_t r, int32_t b) {
	int32_t     i, j;
	struct obj* o = wmalloc(sizeof(*o));
	wassert(o);
	rect_set(&o->extent, l, t, r, b);
	o->ty = type;
	o->ref = 0;
	o->priv = data;

	for (i = 0; i < wsh->rowN; i++) {
		for (j = 0; j < wsh->colN; j++) {
			if (rect_is_overwrapped(&o->extent,
						&wsh->divs[i][j].boundary))
				div_add_obj(&wsh->divs[i][j], o);
		}
	}

	/* if obj is not added anywhere, destroy it! */
	if (!o->ref) {
		wfree(o);
		return false;
	} else
		return true;
}

void
wsheet_del_obj(struct wsheet* wsh, struct obj* o) {
	int32_t      i, j;
	for (i = 0; i < wsh->rowN; i++) {
		for (j = 0; j < wsh->colN; j++) {
			if (rect_is_overwrapped(&o->extent,
						&wsh->divs[i][j].boundary))
				div_del_obj(&wsh->divs[i][j], o);
		}
	}
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

	nlist_clean(&lns);

#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_check_end(DBG_PERF_DRAW_LINE);
#endif /* CONFIG_DBG_STATISTICS */

}
