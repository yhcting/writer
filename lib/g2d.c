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

#include "common.h"
#include "g2d.h"


/*
 * v0, s : closed
 * v1, l : open
 */
static inline int
_intersect_overwrap(int32_t* it0, int32_t* it1,
		    int32_t  v0,  int32_t  v1,
		    int32_t  s,   int32_t  l) {
	int32_t i0, i1;
	/* empty line */
	if (v0 == v1)
		return 0;

	if (v0 < s) {
		/* v0 is smaller */
		if (v1 <= s)
			return 0;
		else if (v1 > l)
			i1 = l;
		else
			i1 = v1;
		i0 = s;

	} else if (v0 >= l) {
		/* v0 is larger */
		if (v1 < s)
			i1 = s - 1;
		else if (v1 >= l)
			return 0;
		else
			i1 = v1;
		i0 = l - 1;

	} else {
		/* v0 is in the middle */
		if (v1 < s)
			i1 = s - 1;
		else if (v1 >= l)
			i1 = l;
		else
			i1 = v1;
		i0 = v0;
	}
	if (i0 == i1)
		return 0;
	else {
		*it0 = i0;
		*it1 = i1;
	}
	return 2;
}

int
line_intersectx(int32_t* ity0, int32_t* ity1,
		int32_t x0, int32_t y0,
		int32_t x1, int32_t y1,
		int32_t x,
		int32_t yt, int32_t yb) {
	wassert(yt < yb);
	if (x0 == x1) {
		if (x == x0)
			return _intersect_overwrap(ity0, ity1, y0, y1, yt, yb);
		else
			return 0; /* parallel */
	} else {
		float s = (float)(x - x0) / (float)(x1 - x0);
		if (0 <= s && s < 1) {
			int32_t y =
				_round_off(s * (float)(y1 - y0) + (float)y0);
			if ((x == x1 && y == y1)
			    || yt > y || y >= yb)
				return 0;
			*ity0 = y;
			return 1;
		} else
			return 0;
	}
}


int
line_intersecty(int32_t* itx0, int32_t* itx1,
		int32_t x0, int32_t y0,
		int32_t x1, int32_t y1,
		int32_t y,
		int32_t xl, int32_t xr) {
	wassert(xl < xr);
	if (y0 == y1) {
		if (y == y0)
			return _intersect_overwrap(itx0, itx1, x0, x1, xl, xr);
		else
			return 0; /* parallel */
	} else {
		float s = (float)(y - y0) / (float)(y1 - y0);
		if (0 <= s && s < 1) {
			int32_t x =
				_round_off(s * (float)(x1 - x0) + (float)x0);
			if ((y == y1 && x == x1)
			    || xl > x || x >= xr)
				return 0;
			*itx0 = x;
			return 1;
		} else
			return 0;
	}
}

bool
rect_is_overlap_line(int32_t l, int32_t t, int32_t r, int32_t b,
		     int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	/*
	 * Check that line is expands on this rectangle region.
	 */

	/*
	 * Note:
	 *   p1 is open set.
	 */
	if ((x0 < l && x1 <= l)
	    || (x0 >= r && x1 >= r)
	    || (y0 < t && y1 <= t)
	    || (y0 >= b && y1 >= b))
		/* trivially not intersected */
		return false;
	/* check that whether one of points are in rect */
	else if (rect_is_in(l, t, r, b, x0, y0)
		 /* check for open point */
		 || (x1 > l && x1 < (r - 1)
		     && y1 > t && y1 < (b - 1 ))) {
		return true;
	} else {
		int32_t i0, i1;
		if (line_intersectx(&i0, &i1,
				    x0, y0, x1, y1, l, t, b)
		    || line_intersectx(&i0, &i1,
				       x0, y0, x1, y1, r - 1, t, b)
		    || line_intersecty(&i0, &i1,
				       x0, y0, x1, y1, t, l, r)
		    || line_intersecty(&i0, &i1,
				       x0, y0, x1, y1, b - 1, l, r)) {
			return true;
		}
	}
	return false;
}



int
rect_intersect_line(int32_t* ix0, int32_t* iy0,
		    int32_t* ix1, int32_t* iy1,
		    int32_t l, int32_t t,
		    int32_t r, int32_t b,
		    int32_t x0, int32_t y0,
		    int32_t x1, int32_t y1) {
	/* check that whether one of points are in rect */
	int32_t      i0, i1, c; /* c: compenstation */
	struct point pt[2], *p = pt;

	if ((x0 < l && x1 <= l)
	    || (x0 >= r && x1 >= r)
	    || (y0 < t && y1 <= t)
	    || (y0 >= b && y1 >= b)
	    || (rect_is_line_in(l, r, t, b, x0, y0, x1, y1)))
		/* trivially not intersected */
		return 0;
	/*
	 * To handle case like below, variable 'c' is used.
	 * Calculating just boundary, function can't find intersection cases
	 *   like below (close point at open edge...)
	 * For more details, see comments of 'rect_intersect_line' at g2d.h.
	 *            +--------+    +---O----+
	 *            |        |    |        |
	 *            |        O or |        C ...
	 *            |        |    |        |
	 *            +---C----+    +--------+
	 * First point is close but second point is open.
	 * So, intersection with open edge as an first point of line
	 *   should NOT on open edge.
	 * Therefore, by using '-1' to open edge, we can caclulate first point
	 *   that is really in the rectangle NOT on OPEN EDGE.
	 *
	 *
	 * "if (! (p > pt && p->x == pt->x && p->y == pt->y))" is used to check
	 *   check exceptional case : Example is...
	 *            C--------+
	 *            |        |
	 *            |   O    |
	 *            |        |
	 *            +--------+
	 * In this case, C is considered as intersection with top and left.
	 * So, event if there is only ONE intersection point, logic may say,
	 *   there is TWO intersection with out this check!.
	 *
	 * bedge_br : is this 'bottom' or 'right' edge?
	 *
	 */
#define __check_intersect(bedge_br, v, min, max, x, y)			\
	c = (bedge_br && x##0 > x##1)? -1: 0;				\
	switch (line_intersect##x(&i0, &i1,				\
				  x0, y0, x1, y1, (v) + c, min, max)) {	\
	case 1: {							\
		p->x = (v) + c;						\
		p->y = i0;						\
		/* # of intersection point is at most 2 */		\
		wassert(p < pt + 2);					\
		/* To avoid getting duplicated intersection point */	\
		if (!(p > pt && p->x == pt->x && p->y == pt->y))	\
			p++;						\
	} break;							\
									\
	case 2: {							\
		/* overlapped with edge */				\
		*i##x##0 = *i##x##1 = (v) + c;				\
		*i##y##0 = i0;						\
		*i##y##1 = i1;						\
		return 3;						\
	}								\
	}

	__check_intersect(false, l, t, b, x, y);
	__check_intersect(true,  r, t, b, x, y);
	__check_intersect(false, t, l, r, y, x);
	__check_intersect(true,  b, l, r, y, x);

	switch (p - pt) {
	case 0: {
	} return 0;

	case 1: {
		*ix0 = pt[0].x;
		*iy0 = pt[0].y;
	} return 1;

	case 2: {
		int32_t v0, v1;
		if (pt[0].x == pt[1].x) {
			v0 = ABS(pt[0].y - y0);
			v1 = ABS(pt[1].y - y0);
		} else {
			v0 = ABS(pt[0].x - x0);
			v1 = ABS(pt[1].x - x0);
		}

		if (v0 < v1) {
			/* pt[0] is closer from (x0, y0) */
			*ix0 = pt[0].x;
			*iy0 = pt[0].y;
			*ix1 = pt[1].x;
			*iy1 = pt[1].y;
		} else {
			*ix1 = pt[0].x;
			*iy1 = pt[0].y;
			*ix0 = pt[1].x;
			*iy0 = pt[1].y;
		}
	} return 2;
	}
	/* SHOULDN'T reach here */
	wassert(0);
	return 0; /* to make compiler be happy */

#undef __check_intersect
}
