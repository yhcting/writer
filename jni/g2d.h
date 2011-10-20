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

#ifndef _G2d_h_
#define _G2d_h_

#include <stdbool.h>
#include <stdint.h>

#include "common.h"

struct point {
	int32_t x, y;
};

/*
 * p0 is closed but p1 is open.
 */
struct line {
	struct point p0, p1;
};

/*
 * 'left' and 'top' is included at rect.
 * But, 'right' and 'bottom' is not included.
 * rect = { (x, y) | x is [l, r), y is [t, b) }
 */
struct rect {
	int32_t l, t, r, b;
};


/*****************************************************************************
 *
 * Notation : '_c' means function for close point.
 *    Function which have '_c' postfix, handles all point and edge of
 *      line and rect considered as closed point/edge.
 *    Because of easier arithmetic of integer based pixel geometry!
 *
 *****************************************************************************/

/********************************
 * Functions for point
 ********************************/
static inline struct point*
point_create(int32_t x, int32_t y) {
	struct point* p = wmalloc(sizeof *p);
	wassert(p);
	p->x = x;
	p->y = y;
	return p;
}

static inline void
point_destroy(struct point* p) {
	wfree(p);
}

static inline bool
point_is_same(const struct point* p0, const struct point* p1) {
	return (p0->x == p1->x && p0->y == p1->y);
}

static inline void
point_set(struct point* pt, int32_t x, int32_t y) {
	pt->x = x;
	pt->y = y;
}

static inline bool
point_is_equal(const struct point* p0, const struct point* p1) {
	return p0->x == p1->x && p0->y == p1->y;
}

/********************************
 * Functions for line
 ********************************/
static inline struct line*
line_create(int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	struct line* l = wmalloc(sizeof *l);
	wassert(l);
	point_set(&l->p0, x0, y0);
	point_set(&l->p1, x1, y1);
	return l;
}

static inline struct line*
line_create_pt(const struct point* p0, const struct point* p1) {
	struct line* l = wmalloc(sizeof *l);
	wassert(l);
	l->p0 = *p0;
	l->p1 = *p1;
	return l;
}


static inline void
line_set(struct line* ln,
	  int32_t x0, int32_t y0,
	  int32_t x1, int32_t y1) {
	point_set(&ln->p0, x0, y0);
	point_set(&ln->p1, x1, y1);
}

static inline bool
line_is_equal(const struct line* l0, const struct line* l1) {
	return point_is_equal(&l0->p0, &l1->p0)
		&& point_is_equal(&l0->p1, &l1->p1);
}

/*
 * Get minimal rectangle boundary that can contains given line.
 */
static inline void
line_boundary( struct rect* out, const struct line* ln) {
	out->l = min(ln->p0.x, ln->p1.x);
	out->r = max(ln->p0.x, ln->p1.x);
	out->t = min(ln->p0.y, ln->p1.y);
	out->b = max(ln->p0.y, ln->p1.y);
}

/*
 *
 * (x1, y1) and yb is open
 * @ity0 : closer intersection point from (x0, y0)
 * @ity1 : farest intersection point from (x0, y0) - open point
 *         this is only valid when line is overwrapped.
 *
 * See examples for deep understanding.
 *
 *                     (7,5)
 *                 CL----O----R
 *               (5,5)     (10,5)
 * ity0 == C / ity1 == O
 *
 *
 *                   (7,5)
 *                 L---O-----CR
 *               (5,5)     (10,5)
 * ity0 == (9,5) / ity1 == (4,5)
 *
 * @return
 *     0 (false) : not intersected
 *     1 (true)  : intersected at one point.
 *                 (*ity0 is valid, but *ity1 is not touched)
 *     2 (true)  : overwrapped in parallel.
 *                 (*ity0 and *ity1 is valid)
 */
int
line_intersectx(int32_t* ity0, int32_t* ity1,
		int32_t x0, int32_t y0,
		int32_t x1, int32_t y1,
		int32_t x,
		int32_t yt, int32_t yb);

static inline int
line_intersectx2(int32_t* ity0, int32_t* ity1,
		 const struct point* p0,
		 const struct point* p1,
		 int32_t x,
		 int32_t yt, int32_t yb) {
	return line_intersectx(ity0, ity1,
			       p0->x, p0->y,
			       p1->x, p1->y,
			       x, yt, yb);
}

/*
 * yb is open
 */
static inline int
line_intersectx3(int32_t* ity0, int32_t* ity1,
		 const struct line* ln,
		 int32_t x, int32_t yt, int32_t yb) {
	return line_intersectx2(ity0, ity1, &ln->p0, &ln->p1, x, yt, yb);
}


/*
 * (x1, y1) and xr is open
 */
int
line_intersecty(int32_t* itx0, int32_t* itx1,
		int32_t x0, int32_t y0,
		int32_t x1, int32_t y1,
		int32_t y,
		int32_t xl, int32_t xr);

static inline int
line_intersecty2(int32_t* itx0, int32_t* itx1,
		 const struct point* p0,
		 const struct point* p1,
		 int32_t y,
		 int32_t xl, int32_t xr) {
	return line_intersecty(itx0, itx1,
			       p0->x, p0->y,
			       p1->x, p1->y,
			       y, xl, xr);
}


/*
 * xr is open
 */
static inline int
line_intersecty3(int32_t* itx0, int32_t* itx1,
		 const struct line* ln,
		 int32_t y, int32_t xl, int32_t xr) {
	return line_intersecty2(itx0, itx1,
				&ln->p0, &ln->p1, y, xl, xr);
}

/********************************
 * Functions for rect
 ********************************/
static inline void
rect_set(struct rect* pr,
	 int32_t l, int32_t t, int32_t r, int32_t b) {
	pr->l = l;
	pr->t = t;
	pr->r = r;
	pr->b = b;
}

static inline void
rect_set_empty(struct rect* r) {
	r->l = r->r = 0;
	r->t = r->b = 0;
}

static inline int32_t
rect_is_empty(const struct rect* r) {
	return r->l >= r->r || r->t >= r->b;
}

static inline bool
rect_is_equal(const struct rect* r0, const struct rect* r1) {
	return r0->l == r1->l && r0->t == r1->t
		&& r0->r == r1->r && r0->b == r1->b;
}

static inline bool
rect_is_overlap(int32_t l0, int32_t t0, int32_t r0, int32_t b0,
		int32_t l1, int32_t t1, int32_t r1, int32_t b1) {
	return !(r0 <= l1 || t0 >= b1 || r1 <= l0 || t1 >= b0);
}

static inline bool
rect_is_overlap2(const struct rect* r0, const struct rect* r1) {
	return rect_is_overlap(r0->l, r0->t, r0->r, r0->b,
			       r1->l, r1->t, r1->r, r1->b);
}

static inline bool
rect_is_in(int32_t l, int32_t t, int32_t r, int32_t b,
	    int32_t x, int32_t y) {
	return (x >= l && x < r && y >= t && y < b);
}

static inline bool
rect_is_in2(const struct rect* r, int32_t x, int32_t y) {
	return rect_is_in(r->l, r->t, r->r, r->b, x, y);
}

static inline bool
rect_is_in3(const struct rect* r, const struct point* p) {
	return rect_is_in2(r, p->x, p->y);
}


static inline bool
rect_is_line_in(int32_t l, int32_t t, int32_t r, int32_t b,
		int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	/* (x1, y1) is open point */
	return rect_is_in(l, t, r, b, x0, y0)
		&& ((l - 1) <= x1 && r >= x1)
		&& ((t - 1) <= y1 && b >= y1);
}

static inline bool
rect_is_line_in2(int32_t l, int32_t t, int32_t r, int32_t b,
		const struct line* ln) {
	return rect_is_line_in(l, t, r, b,
				ln->p0.x, ln->p0.y, ln->p1.x, ln->p1.y);
}

static inline bool
rect_is_line_in3(const struct rect* r, const struct line* l) {
	return rect_is_line_in2(r->l, r->t, r->r, r->b, l);
}

/*
 * Is there any point that is included at both 'rect' and 'line'.
 */
bool
rect_is_overlap_line(int32_t l, int32_t t, int32_t r, int32_t b,
		     int32_t x0, int32_t y0, int32_t x1, int32_t y1);

static inline bool
rect_is_overlap_line2(int32_t l, int32_t t, int32_t r, int32_t b,
		      const struct point* pt0, const struct point* pt1) {
	return rect_is_overlap_line(l, t, r, b,
				    pt0->x, pt0->y, pt1->x, pt1->y);
}

static inline bool
rect_is_overlap_line3(const struct rect* r,
		      const struct point* pt0, const struct point* pt1) {
	return rect_is_overlap_line2(r->l, r->t, r->r, r->b, pt0, pt1);
}




/*
 * (ix0, iy0) - closed / (ix1, iy1) - open : intersect point.
 *
 * Algorithm for getting intersection point
 * ----------------------------------------
 * This is not simple due to 'open' concept.
 * First of all, we should define "What is intersection point on open line?"
 *
 * Conceptually, we can say about following cases without argue.
 *            +--------+
 *            |        |
 *            C        |
 *            |        |
 *            +-----O--+
 * [ 'C' is intersection, but NOT O ]
 *
 * But how about this case.
 *            +--------+
 *            |        |
 *            O        |
 *            |        |
 *            +---C----+
 * Can we say that there is not intersection with rectangle edge?
 * Definitely NO!
 * There is intersection some where between [C, O) with the line of bottom
 *   boundary of rectangle that is at some where above given open bottom edge.
 * So, we should say "There is 'one' intersection point.", but nobody can say
 *   exact coordinate of intersection becase, bottom edge is open.
 *
 * In case like above, this function returns intersection point of the line and
 *   bottom line that is one-pixel above.
 * That is, intersection between [C, O) and [(l, b-1), (r, b-1)) is calculated.
 *
 * To understand concepts, see comments of 'line_intersectx' above.
 *
 *
 * @return : number of intersection point.
 *           3 means overwrap with one of edges.
 *           (output at this moment is same with intersectX/Y())
 */
int
rect_intersect_line(int32_t* ix0, int32_t* iy0,
		    int32_t* ix1, int32_t* iy1,
		    int32_t l, int32_t t,
		    int32_t r, int32_t b,
		    int32_t x0, int32_t y0,
		    int32_t x1, int32_t y1);

static inline int
rect_intersect_line2(struct point*       i0,
		     struct point*       i1,
		     int32_t l, int32_t t,
		     int32_t r, int32_t b,
		     int32_t x0, int32_t y0,
		     int32_t x1, int32_t y1) {
	return rect_intersect_line(&i0->x, &i0->y,
				   &i1->x, &i1->y,
				   l, t, r, b,
				   x0, y0, x1, y1);
}

static inline int
rect_intersect_line3(struct point*       i0,
		     struct point*       i1,
		     int32_t l, int32_t t,
		     int32_t r, int32_t b,
		     const struct point* pt0,
		     const struct point* pt1) {
	return rect_intersect_line2(i0, i1,
				    l, t, r, b,
				    pt0->x, pt0->y,
				    pt1->x, pt1->y);
}

static inline int
rect_intersect_line4(struct point*       i0,
		     struct point*       i1,
		     const struct rect*  r,
		     const struct point* pt0,
		     const struct point* pt1) {
	return rect_intersect_line3(i0, i1,
				    r->l, r->t, r->r, r->b,
				    pt0, pt1);
}


/*
 * @return : number of intersection point.
 *           But, in case that line is overwraped with a edge of rect,
 *             return value is -1.
 *           '1' means '*pt0' is filled, but '*pt1' is not touched.
 * @pt0    : intersection point closer to l->p0.
 */
static inline int
rect_intersect_line5(struct point*      pt0,
		     struct point*      pt1,
		     const struct rect* r,
		     const struct line* l) {
	return rect_intersect_line4(pt0, pt1, r, &l->p0, &l->p1);
}


#endif /* _G2d_h_ */
