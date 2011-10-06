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
#include "node.h"
#include "div.h"
#include "wsheet.h"
#include "g2d.h"
#include "d2d.h"
#include "ucmd.h"
#include "history.h"

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

	nmp_create(CONFIG_NMP_SZ);
	his_init();

	_initialized = true;
}

static void _deinit(void) __attribute__((unused));
static void
_deinit(void) {
	if (!_initialized)
		return;

	_initialized = false;

	nmp_destroy();
	his_deinit();

}


static inline struct ucmd*
_ucmd_quick_start(enum ucmd_ty ty, struct wsheet* wsh) {
	struct ucmd* uc = ucmd_create(ty, wsh);
	wassert(uc);
	wsheet_set_ucmd(wsh, uc);
	ucmd_alloc(uc);
	ucmd_start(uc);
	return uc;
}

static inline void
_ucmd_quick_end(struct ucmd* uc) {
	ucmd_end(uc);
	/* there is no active user command */
	wsheet_set_ucmd(uc->wsh, NULL);
	his_add(uc);
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
EXTERN_UT void
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
		ln = nodelk(lk)->v;
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
EXTERN_UT void wsys_deinit(void) __attribute__((unused));
EXTERN_UT void
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

	wsh->ucmd = NULL; /* there is no active user commmand */
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

/*
 * # of lines may be increased for each division.
 */
static void
_cutout_split_lines(struct list_link* lns,
		    int32_t l, int32_t t, int32_t r, int32_t b) {

	/* Internal function */
	void __split_set(struct line* ln,
			 bool b_out2in,
			 int32_t x, int32_t y) {
		struct node* (*add_func)(struct node*, struct line*);
		struct line* new = wmalloc(sizeof(*new));
		new->thick = ln->thick;
		new->color = ln->color;
		if (b_out2in) {
			line_set(new, ln->x0, ln->y0, x, y);
			line_set(ln, x, y, ln->x1, ln->y1);
			add_func = &div_add_line_prev;
		} else {
			line_set(new, x, y, ln->x1, ln->y1);
			line_set(ln, ln->x0, ln->y0, x, y);
			add_func = &div_add_line_next;
		}
		(*add_func)(container_of(ln->divlk, struct node, lk), new);
	}

	struct line*       ln;
	int32_t            intersect;
	struct node*       n;

	list_foreach_item(n, lns, struct node, lk) {
		ln = n->v;

		/*
		 * FIXME: We don't care about below case
		 *   line is parallel with rectangle edge.
		 * So, line this is subset of rectangle edge is not removed!
		 */

		/* check for all for edge. */
		if (intersectX(&intersect, ln, l, t, b)) {
			__split_set(ln,
				    (ln->x0 < ln->x1),
				    l, intersect);
		}

		if (intersectX(&intersect, ln, r, t, b)) {
			__split_set(ln,
				    (ln->x0 > ln->x1),
				    r, intersect);
		}

		if (intersectY(&intersect, ln, t, l, r)) {
			__split_set(ln,
				    (ln->y0 < ln->y1),
				    intersect, t);
		}

		if (intersectY(&intersect, ln, b, l, r)) {
			__split_set(ln,
				    (ln->y0 > ln->y1),
				    intersect, b);
		}
		/* free link */
		list_del(ln->divlk);

		nmp_free(nodelk(ln->divlk));
		wfree(ln);
	}
}

/*
 * SMP optimization is possible.
 * But, not yet.
 * If there is performance issue at 'cutout', this optimization can introduced.
 */
void
wsheet_cutout_lines(struct wsheet* wsh,
		    int32_t l, int32_t t, int32_t r, int32_t b) {
	struct list_link   lns;
#if 0
	struct line*       ln;
	struct node*       n;
	struct ahash*      ah;
#endif

	list_init_link(&lns);
	wsheet_find_lines(wsh, &lns, l, t, r, b);
	_cutout_split_lines(&lns, l, t, r, b);
	/* splitting lines is done. */
#if 0
#error implmenet this!!
	/*
	 * Concept.
	 *
	 *  * Hashing all node address that should be cutout.
	 *  * If prev is NOT in hash, this node is 'start of
	 *      curve'.
	 *  * If next is NOT in hash, this node is 'end of
	 *      curve'.
	 *  * If next and prev are all in hash, this is part of
	 *      curve. So, link is NOT changed (preserved.)
	 *
	 * Here is Steps
	 *
	 *  * Iterates lines to find 'start' or 'end' of curve.
	 *  * If 'start of curve' is found, do following steps.
	 *    Prev is stored as 'prev' at 'struct curve' and
	 *      node is attached to 'curve' structure.
	 *    (At this moment, node lost its original 'prev' link.
	 *     But, it already stored at 'curve' structure.)
	 *    Then, continue to follow line link until find end of
	 *      curve.
	 *  * If 'end of curve' is found, ignore this.
	 */

	ah = ahash_create();
	wassert(ah);

	/* hashing all lines */
	list_foreach_item(n, lns, struct node, lk) {
		ahash_add(ah, n->v);
	}

	/*
	 * Find start of curve. And make curve history...
	 */
	list_foreach_item(n, lns, struct node, lk) {
		if ahash_check(ah, div_prev_line(n->v))
	}


	ahash_destroy(ah);


		/* free link */
		list_del(ln->divlk);

		nmp_free(nodelk(ln->divlk));
		wfree(ln);
#endif

	nlist_free(&lns);
}

static void
_line_split(struct line*      l,
	    struct list_link* list,
	    int32_t  v0,    int32_t v1,
	    uint32_t divsz, uint32_t divN,
	    uint8_t  thick, uint16_t color,
	    int (*intersectF)(int32_t*, const struct line*,
			      int32_t, int32_t, int32_t),
	    void (*lnset)(struct line*, struct line*,
			  int32_t, int32_t)) {

	struct line*       tmpl;
	int32_t	           v, i, ip; /* ip : Intersect Point */
	int32_t            vstart, vend, vstep; /* value min/max */

	/* check horizontal - row - split */
	vstart = _divI(v0, divsz);
	vend   = _divI(v1, divsz);
	vstep  = (v0 < v1)? 1: -1;

	wassert((vstep > 0 && vstart <= vend) ||
		(vstep < 0 && vstart >= vend));
	for (i = vstart; i != vend + vstep; i += vstep) {
		v = (vstep > 0)? (i + 1) * divsz: i * divsz - 1;

		/* keep doing with remained one! */
		tmpl = wmalloc(sizeof(*tmpl));
		wassert(tmpl);
		tmpl->thick = thick;
		tmpl->color = color;
		if (1 == (*intersectF)(&ip, l, v, INT32_MIN, INT32_MAX)) {
			(*lnset)(tmpl, l, ip, v);
		} else {
			/*
			 * parallel case should be handled
			 *   with non-intersection case
			 */
			nlist_add(list, l);
			wfree(tmpl);
			break; /* exit loop */
		}

		if (i >= 0 && i < divN)
			nlist_add(list, tmpl);
		else
			wfree(tmpl);
	}
}


EXTERN_UT void
wsheet_add_line(struct wsheet* wsh,
		int32_t x0, int32_t y0,
		int32_t x1, int32_t y1,
		uint8_t thick,
		uint16_t color) {

	void lnsetX(struct line* tmpl, struct line* l,
		    int32_t ip, int32_t v) {
		line_set(tmpl, l->x0, l->y0, ip, v);
		line_set(l, ip, v, l->x1, l->y1);
	}

	void lnsetY(struct line* tmpl, struct line* l,
		    int32_t ip, int32_t v) {
		line_set(tmpl, l->x0, l->y0, v, ip);
		line_set(l, v, ip, l->x1, l->y1);
	}

	struct list_link   list;
	struct list_link   splits;
	struct node*       n;
	struct line*       l;

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

	_line_split(l, &list, l->y0, l->y1,
		    wsh->divH, wsh->rowN,
		    thick, color,
		    &intersectY,
		    &lnsetX);

	/* check vertical - column - split */
	list_foreach_item(n, &list, struct node, lk) {
		l = n->v;

		_line_split(l, &splits, l->x0, l->x1,
			    wsh->divW, wsh->colN,
			    thick, color,
			    &intersectX,
			    &lnsetY);
	}
	nlist_free(&list);

	/* assign to division */
	list_foreach_item(n, &splits, struct node, lk) {
		struct node* divn; /* node of division */
		struct div*  dv = _wsheet_div_line(wsh, n->v);
		if (dv && !line_is_empty(n->v)) {
			divn = div_add_line(dv, n->v);
			if (wsh->ucmd)
				ucmd_notify(wsh->ucmd, divn);
		} else
			/* there is no div to add */
			wfree(n->v); /* free line object */
	}
	nlist_free(&splits);
}

void
wsheet_add_curve(struct wsheet* wsh,
		 int32_t* pts, int32_t nr_pt,
		 uint8_t  thick,
		 uint16_t color) {
	int i;
	struct ucmd* uc;

	uc = _ucmd_quick_start(UCMD_CURVE, wsh);
	for (i = 1; i < nr_pt; i++)
		wsheet_add_line(wsh,
				pts[2*(i-1)], pts[2*(i-1)+1],
				pts[2*i], pts[2*i+1],
				thick, color);
	_ucmd_quick_end(uc);
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

	nlist_free(&lns);

#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_check_end(DBG_PERF_DRAW_LINE);
#endif /* CONFIG_DBG_STATISTICS */

}
