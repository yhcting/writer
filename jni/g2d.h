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

#include "gtype.h"

/*
 * (x1, y1) and yb is open
 * @return
 *     0 (false) : not intersected
 *     1 (true)  : intersected at one point.
 *                 (*out_intersect is valid)
 *     2 (true)  : overwrapped in parallel.
 *                 (*out_intersect is invalid)
 */
int
g2d_intersectX(int32_t* out_intersecty,
	       int32_t x0, int32_t y0,
	       int32_t x1, int32_t y1,
	       int32_t x,
	       int32_t yt, int32_t yb);

/*
 * (x1, y1) and xr is open
 */
int
g2d_intersectY(int32_t* out_intersectx,
	       int32_t x0, int32_t y0,
	       int32_t x1, int32_t y1,
	       int32_t y,
	       int32_t xl, int32_t xr);


/*
 * yb is open
 */
static inline int
intersectX(int32_t* out_intersecty,
	   const struct line* ln,
	   int32_t x, int32_t yt, int32_t yb) {
	return g2d_intersectX(out_intersecty,
			      ln->x0, ln->y0,
			      ln->x1, ln->y1,
			      x,
			      yt, yb);
}

/*
 * xr is open
 */
static inline int
intersectY(int32_t* out_intersectx,
	   const struct line* ln,
	   int32_t y, int32_t xl, int32_t xr) {
	return g2d_intersectY(out_intersectx,
			      ln->x0, ln->y0,
			      ln->x1, ln->y1,
			      y,
			      xl, xr);
}


/********************************
 * Functions for line
 ********************************/
static inline bool
line_is_empty(const struct line* ln) {
	return (ln->x0 == ln->x1 && ln->y0 == ln->y1);
}

static inline void
line_set(struct line* ln,
	  int32_t x0, int32_t y0,
	  int32_t x1, int32_t y1) {
	ln->x0 = x0;
	ln->y0 = y0;
	ln->x1 = x1;
	ln->y1 = y1;
}

static inline void
line_boundary(const struct line* ln, struct rect* out) {
	out->l = ln->x0;
	out->r = ln->x1;
	if (ln->y0 < ln->y1) {
		out->t = ln->y0;
		out->b = ln->y1;
	} else {
		out->t = ln->y1;
		out->b = ln->y0;
	}
}

/********************************
 * Functions for rect
 ********************************/
static inline int32_t
rect_is_empty(const struct rect* r) {
	return r->l >= r->r || r->t >= r->b;
}

static inline bool
rect_is_overwrapped(const struct rect* r0, const struct rect* r1) {
	return !(r0->r <= r1->l || r0->t >= r1->b
		 || r1->r <= r0->l || r1->t >= r0->b);
}

static inline bool
rect_is_contains(const struct rect* r, int32_t x, int32_t y) {
	return (x >= r->l && x < r->r && y >= r->t && y < r->b);
}

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

#endif /* _G2d_h_ */
