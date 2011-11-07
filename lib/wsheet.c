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
#include <stdbool.h>
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
#include "curve.h"


/* #define _DEBUG */

/*
 * Flag to check that this wsheet static values are initialized or not
 */
static bool _initialized = false;

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
/*
 * @i:      column index of division.
 * @return: x-coordinate value of left of given column division-index
 */
static inline int32_t
_divL(const struct wsheet* wsh, int32_t i) {
	return i * wsh->divW;
}

static inline int32_t
_divR(const struct wsheet* wsh, int32_t i) {
	return (i + 1) * wsh->divW;
}

static inline int32_t
_divT(const struct wsheet* wsh, int32_t i) {
	return i * wsh->divH;
}

static inline int32_t
_divB(const struct wsheet* wsh, int32_t i) {
	return (i + 1) * wsh->divH;
}

/*
 * create curve with list of pointnd
 */
static inline struct curve*
_create_curve_pointnd_list(struct list_link* hd) {
	struct pointnd* ptn;
	struct point*   pt;
	struct curve*   crv = crv_create(list_size(hd));

	wassert(crv);
	pt = crv->pts;
	pointnd_foreach(ptn, hd)
		*pt++ = ptn->pt;
	return crv;
}



static void
_init(void) {
	wassert(!_initialized);
	his_init();

	_initialized = true;
}

static void _deinit(void) __attribute__((unused));
static void
_deinit(void) {
	if (!_initialized)
		return;
	_initialized = false;

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
_is_contains(const struct wsheet* wsh, int32_t x, int32_t y) {
	return !(x < 0 || x >= wsh->divW * wsh->colN
		 || y < 0 || y >= wsh->divH * wsh->rowN);
}

static inline bool
_is_valid_div_index(const struct wsheet* wsh, int32_t ri, int32_t ci) {
	return !(ri < 0 || ri >= wsh->rowN || ci < 0 || ci >= wsh->colN);
}

/*
 * get division where point belongs to.
 */
static inline struct div*
_div_point_belong_to(const struct wsheet* wsh, int32_t x0, int32_t y0) {
	int32_t c, r;
	c = _divI(x0, wsh->divW);
	r = _divI(y0, wsh->divH);
	return 	(c >= 0 && r >= 0)? &wsh->divs[r][c]: NULL;
}

/*
 * get division where line should be assigned to
 */
static inline struct div*
_div_line_belong_to(const struct wsheet* wsh, struct line* ln) {
	/*
	 * NOTE !!
	 * (x0, y0) is closed, but (x1, y1) is open.
	 * So, division should be calculated based on (x0, y0) of line!!
	 */
	return _div_point_belong_to(wsh, ln->p0.x, ln->p0.y);
}

/*
 * find lines in the given rectangle division group
 * (bi and ri are NOT included)
 */
static inline void
_get_div_lines_draw(const struct wsheet* wsh, struct list_link* out,
	     int32_t ti, int32_t bi, int32_t li, int32_t ri) {
	int32_t i, j;
	/* fully included division - we don't need to check. */
	for (i = ti; i < bi; i++)
		for (j = li; j < ri; j++)
			div_get_lines_draw(&wsh->divs[i][j], out);
}

/*
 * NOTE!!!:
 *   part/whole of lines that are included in the *rectangle will be returned.
 *   *rectange includes left/top line, but excludes right/bottom line.
 * @out : list of < struct lines_draw >
 */
EXTERN_UT void
wsheet_find_lines_draw(const struct wsheet* wsh, struct list_link* out,
		       int32_t l, int32_t t, int32_t r, int32_t b) {

#define __horizontal_strip()						\
	do {								\
		/* horizontal strip */					\
		for (i = li; i <= ri; i++) {				\
			/* top strip */					\
			div_find_lines_draw(&wsh->divs[ti][i],		\
					    out, l, t, r, b);		\
			if (bi > ti) {					\
				/* bottom strip */			\
				div_find_lines_draw(&wsh->divs[bi][i],	\
						    out, l, t, r, b);	\
			}						\
		}							\
	} while (0)

#define __vertical_strip()						\
	do {								\
		/* vertical strip except for 4-vertex-position in rect. */ \
		for (i = ti + 1; i < bi; i++) {				\
			/* left strip */				\
			div_find_lines_draw(&wsh->divs[i][li],		\
					    out, l, t, r, b);		\
			if (ri > li) {					\
				/* right strip */			\
				div_find_lines_draw(&wsh->divs[i][ri],	\
						    out, l, t, r, b);	\
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

	/* Not fully included division. - we need to check */
	__horizontal_strip();
	__vertical_strip();
	_get_div_lines_draw(wsh, out, ti + 1, bi, li + 1, ri);

	dbg_tpf_check_end(DBG_PERF_FIND_LINE);

#undef __vertical_strip
#undef __horizontal_strip
}

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


struct div*
wsheet_find_div(struct wsheet* wsh, int32_t x, int32_t y) {
	return _div_point_belong_to(wsh, x, y);
}

/*
 * SMP optimization is possible.
 * But, not yet.
 * If there is performance issue at 'cutout', this optimization can introduced.
 */
void
wsheet_cutout_lines(struct wsheet* wsh,
		    int32_t l, int32_t t, int32_t r, int32_t b) {
	struct ucmd* uc;
	int32_t i, j;
	/* list of node which value is 'struct curve' */
	struct list_link lrm, ladd;

	uc = _ucmd_quick_start(UCMD_CUT, wsh);

	list_init_link(&lrm);
	list_init_link(&ladd);
	for (i = 0; i < wsh->rowN; i++) {
		for (j = 0; j < wsh->colN; j++) {
			if (rect_is_overlap(_divL(wsh, j), _divT(wsh, i),
					    _divR(wsh, j), _divB(wsh, i),
					    l, t, r, b)) {
				/*
				 * division affected by cutout rectangle.
				 */
				div_cutout(&wsh->divs[i][j],
					   &lrm, &ladd, l, t, r, b);
			}
		}
	}

	ucmd_notify(uc, &lrm, &ladd);
	_ucmd_quick_end(uc);
}


/*
 * @return : 0 (next point)
 *           others (new division. again with updated point.)
 */
static int
_add_line(struct list_link*   hd,
	  int32_t l, int32_t t, int32_t r, int32_t b,
	  int32_t x0, int32_t y0, int32_t x1, int32_t y1,
	  int32_t* itx, int32_t* ity) {
	int32_t  i0, i1; /* intersect */

	wassert(!(x0 == x1 && y0 == y1)
		&& rect_is_in(l, t, r, b, x0, y0));
	/*
	 * Trivial case.
	 *   x1, y1 is already in boundary
	 */
	if (rect_is_line_in(l, t, r, b, x0, y0, x1, y1))
		return 0;

	/*
	 * General case.
	 */

	/*
	 * NOTE
	 * ----
	 *     1-pixel-error caused by open/close concept of line and rect
	 *       may leads that some line-drawn-pixels are out of
	 *       division pixel
	 *
	 *     See below
	 *     [ Notation ]
	 *         I : intersection point(pixel)
	 *         X : filled pixel (drawn)
	 *         m : missing pixel
	 *
	 *     [Point1]
	 *     +---+---+---+---+---+
	 *     |   | X | I | X |   | <-- This is area of div1
	 *     +---+---+---+---+---+
	 *     |   |   |   | X | X | <-- top of div0 / bottom of div1
	 *     +---+---+---+---+---+ [Point0]
	 *
	 *     This 1-pixel-error may cause memory corruption.
	 *     (Accessing out of allocated pixels)
	 *     So, this should be compensated and resolved at
	 *       LINE-DRAWING-ALGORITHM !!!
	 *     Important!
	 *       There is NO ERROR LARGER THAN 1-pixel!
	 *       This SPLIT ALGORITHM guarantees this!!
	 *
	 * WARNING
	 * -------
	 * If we don't use open/close concept at line
	 * There may be several pixel lost.
	 * This is critical
	 *
	 *     +---+---+---+---+---+
	 *     | X | I | m |   |   | <-- bottom of div1
	 *     +---+---+---+---+---+
	 *     |   |   | m | m | I | <-- top of div0
	 *     +---+---+---+---+---+ [Point0]
	 *
	 */
#define __split(func, v, pv, pit, min, max)			\
	switch (func(&i0, &i1, x0, y0, x1, y1, v, min, max)) {	\
	case 1:							\
		(pv) = (v);					\
		(pit) = i0;					\
		return 1;					\
								\
	case 2:							\
		wassert(0); /* this shouldn't happen! */	\
		break;						\
	}

	/*
	 * There should be one intersection point that is inside boundary!
	 *
	 * For example, if intersection with right is out of bounary (if slop
	 *   is large.), intersection with top should be inside boundary
	 *   vice versa.
	 *
	 *    +-----------------------+ <- 1 pixel out
	 *    | +-------------------+ |
	 *    | |                   | |
	 *    | |                   | |
	 *
	 * (l - 1, t - 1, r, b) is rect i-pixel-out
	 */
	__split(line_intersectx, l - 1, *itx, *ity, t - 1, b);
	__split(line_intersectx, r,     *itx, *ity, t - 1, b);
	__split(line_intersecty, t - 1, *ity, *itx, l - 1, r);
	/* last 'r + 1' to fill open point (r, b). */
	__split(line_intersecty, b,     *ity, *itx, l - 1, r + 1);
#undef __split

	/* SHOULDN'T reach here!! */
	wloge("***(%d, %d, %d, %d / (%d, %d) (%d, %d)***\n",
	      l, t, r, b, x0, y0, x1, y1);
	wassert(0);

	return 0;

#undef __add_to_list
}

void
wsheet_add_curve(struct wsheet* wsh,
		 const int32_t* pts, uint16_t nrpts,
		 uint8_t  thick,
		 uint16_t color) {


	void _add_pointnd_last(struct list_link* hd, int32_t x, int32_t y) {
		if (hd)
			pointnd_add_last(hd, x, y);
	}


	void __add_curve(struct div* div, struct list_link* hd) {
		if (hd && list_size(hd) > 0) {
			struct curve* crv = _create_curve_pointnd_list(hd);
			crv->color = color;
			crv->thick = thick;
			div_add_curve(div, crv);
			ucmd_notify(wsh->ucmd, crv, NULL);
			/* free memory for new start */
			pointnd_free_list(hd);
		}
	}

	struct ucmd* uc;
	int32_t   x0, y0, x1, y1, itx, ity;
	int32_t   ri, ci; /* row/column index */
	const int32_t  *pt, *ptend;
	struct list_link  hd, *phd;

	if (nrpts < 2) {
		wwarn();
		return;
	}
	uc = _ucmd_quick_start(UCMD_CURVE, wsh);

	list_init_link(&hd);
	pt = pts;
	ptend = pts + (nrpts * 2);

	/*
	 * initial setting
	 */
	x0 = *pt++;
	y0 = *pt++;
	x1 = *pt++;
	y1 = *pt++;

	/* wlogd("ADD : %d, %d, %d, %d", x0, y0, x1, y1); */
#define __update_division_info(x, y)					\
	do {								\
		ci = _divI(x, wsh->divW);				\
		ri = _divI(y, wsh->divH);				\
		phd = (_is_valid_div_index(wsh, ri, ci))? &hd: NULL;	\
	} while (0)

	__update_division_info(x0, y0);
	while (pt <= ptend) {
		_add_pointnd_last(phd, x0, y0);
		if (_add_line(phd,
			      _divL(wsh, ci), _divT(wsh, ri),
			      _divR(wsh, ci), _divB(wsh, ri),
			      x0, y0, x1, y1, &itx, &ity)) {
			/*
			 * add intersection point
			 * this is last point of this curve.
			 */
			_add_pointnd_last(phd, itx, ity);
			__add_curve(&wsh->divs[ri][ci], phd);

			/*
			 * division is changed!!
			 * update division
			 */
			x0 = itx;
			y0 = ity;
			__update_division_info(x0, y0);

		} else {
			x0 = x1;
			y0 = y1;
			x1 = *pt++;
			y1 = *pt++;
		}
	}
	/* add last point */
	_add_pointnd_last(phd, x0, y0);
	__add_curve(&wsh->divs[ri][ci], phd);

#undef __update_division_info

	_ucmd_quick_end(uc);
}

#if 0
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
			if (rect_is_overlap2(&o->extent,
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
			if (rect_is_overlap2(&o->extent,
					     &wsh->divs[i][j].boundary))
				div_del_obj(&wsh->divs[i][j], o);
		}
	}
}
#endif



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
	struct lines_draw* ld;
	struct linend*     ln;

	while (lk != head) {
		ld = container_of(lk, struct lines_draw, lk);
		linend_foreach(ln, &ld->lns)
			draw_line(pixels,
				  w, h,
				  _rbg16to32(ld->color),
				  _round_off(zf * (float)(ld->thick)),
				  _round_off(zf * (float)(ln->ln.p0.x - ox)),
				  _round_off(zf * (float)(ln->ln.p0.y - oy)),
				  _round_off(zf * (float)(ln->ln.p1.x - ox)),
				  _round_off(zf * (float)(ln->ln.p1.y - oy)));

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

#endif

void
wsheet_draw(struct wsheet* wsh,
	    int32_t* pixels,
	    int32_t w, int32_t h,
	    int32_t ox, int32_t oy,
	    int32_t l, int32_t t, int32_t r, int32_t b,
	    float zf) {

	/* list of struct lines_draw */
	struct list_link   lns;
	struct lines_draw* ld __attribute__((unused));

	dbg_tpf_check_start(DBG_PERF_DRAW_LINE);

	list_init_link(&lns);

	wsheet_find_lines_draw(wsh, &lns, l, t, r, b);

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

	lines_draw_foreach(ld, &lns) {
		struct linend* ln;
		linend_foreach(ln, &ld->lns) {
			draw_line(pixels,
				  w, h,
				  _rbg16to32(ld->color),
				  _round_off(zf * (float)(ld->thick)),
				  _round_off(zf * (float)(ln->ln.p0.x - ox)),
				  _round_off(zf * (float)(ln->ln.p0.y - oy)),
				  _round_off(zf * (float)(ln->ln.p1.x - ox)),
				  _round_off(zf * (float)(ln->ln.p1.y - oy)));
		}
	}

#endif /* CONFIG_DUALCORE */

	lines_draw_free_list_deep(&lns);

	dbg_tpf_check_end(DBG_PERF_DRAW_LINE);

}
