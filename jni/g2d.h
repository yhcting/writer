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

#include "gtype.h"

int
g2d_splitX(int* out_intersecty,
	   int x0, int y0,
	   int x1, int y1,
	   int x,
	   int yt, int yb);

int
g2d_splitY(int* out_intersectx,
	   int x0, int y0,
	   int x1, int y1,
	   int y,
	   int xl, int xr);


static inline int
splitX(int* out_intersecty, struct line* ln, int x, int yt, int yb) {
	return g2d_splitX(out_intersecty,
			  ln->x0, ln->y0,
			  ln->x1, ln->y1,
			  x,
			  yt, yb);
}

static inline int
splitY(int* out_intersectx, struct line* ln, int y, int xl, int xr) {
	return g2d_splitY(out_intersectx,
			  ln->x0, ln->y0,
			  ln->x1, ln->y1,
			  y,
			  xl, xr);
}


/********************************
 * Functions for line
 ********************************/
static inline void
line_set(struct line* ln,
	  unsigned short x0, unsigned short y0,
	  unsigned short x1, unsigned short y1) {
	if (x0 <= x1) {
		ln->x0 = x0;
		ln->y0 = y0;
		ln->x1 = x1;
		ln->y1 = y1;
	} else {
		ln->x0 = x1;
		ln->y0 = y1;
		ln->x1 = x0;
		ln->y1 = y0;
	}
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
static inline int
rect_is_empty(const struct rect* r) {
	return r->l >= r->r || r->t >= r->b;
}

static inline int
rect_contains(const struct rect* r, int x, int y) {
	return (x >= r->l && x < r->r && y >= r->t && y < r->b);
}

static inline void
rect_set(struct rect* pr, int l, int t, int r, int b) {
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
