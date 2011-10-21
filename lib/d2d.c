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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "common.h"

#ifdef CONFIG_DUALCORE
#	include <pthread.h>
#endif /* CONFIG_DUALCORE */


static void
_fill_rect_(int32_t* pixels, int32_t w, int32_t h,
	    int32_t color,
	    int32_t l, int32_t t, int32_t r, int32_t b) {
	if (l < 0)
		l=0;

	if (t < 0)
		t=0;

	if (r > w)
		r=w;

	if (b > h)
		b=h;

	if ((t >= b) || (l >= r))
		return ; /* nothing to draw. */

	register int32_t*    ds = pixels;
	register int32_t*    de;
	int32_t*             d_finish;
	register int32_t     col = color;
	int32_t              rw = r - l;
	int32_t              gap = w - rw;
	/* NOTE: 'r' and 'b' is open */
	d_finish = ds + w * (b - 1) + r;
	ds = ds + w * t + l;
	while (ds < d_finish) {
		de = ds + rw;
		UNROLL16(*ds = col; ds++;, rw, ds < de);
		ds += gap;
	}
}

#if defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM)
struct _fill_rect_arg {
	int32_t* pixels;
	int32_t  w, h, color, l, t, r, b;
};

static void*
_fill_rect_worker(void* arg) {
	struct _fill_rect_arg* a = arg;
	_fill_rect_(a->pixels,
		    a->w, a->h,
		    a->color,
		    a->l, a->t, a->r, a->b);
	return NULL;
}

#endif /* defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM) */


void
fill_rect(int32_t* pixels, int32_t w, int32_t h,
	  int32_t  color,
	  int32_t  l, int32_t t, int32_t r, int32_t b) {

	dbg_tpf_check_start(DBG_PERF_FILL_RECT);

#if defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM)

	{ /* Just scope */
		pthread_t             thd;
		void*                 ret;
		struct _fill_rect_arg arg;
		arg.pixels = pixels;
		arg.w      = w;
		arg.h      = h;
		arg.color  = color;
		arg.l      = l;
		arg.t      = t;
		arg.r      = r;
		arg.b      = b/2;
		if (pthread_create(&thd, NULL, &_fill_rect_worker, &arg))
			ASSERT(0);
		_fill_rect_(pixels, w, h, color, l, b - b/2, r, b);

		if (pthread_join(thd, &ret))
			ASSERT(0);
	} /* Just scope */

#else /* defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM) */

	_fill_rect_(pixels, w, h, color, l, t, r, b);

#endif /* defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM) */

	dbg_tpf_check_end(DBG_PERF_FILL_RECT);
}

void
copy_rect(int32_t* dst, const int32_t* src,
	  int32_t dw, int32_t dh, int32_t dl, int32_t dt,
	  int32_t sw, int32_t sh, int32_t sl, int32_t st,
	  int32_t width, int32_t height) {
	register const int32_t* ss = src;
	register int32_t*       ds = dst + dw * dt + dl;
	register int32_t        s_gap = sw - width;
	register int32_t        d_gap = dw - width;
	const int32_t*	        s_finish;

	s_finish = ss + sw * (height + st - 1) + sl + width;
	ss = ss + sw * st + sl;
	while (ss < s_finish) {
		memcpy(ds, ss, width * sizeof(*ds));
		ss += width + s_gap;
		ds += width + d_gap;
	}
}


/* pre-calculated value for filling circle!! : to increase performance!! */
static int32_t _circle0[1][2] = {{0,1}};
static int32_t _circle1[3][2] = {{0,1},{-1,2},{0,1}};
static int32_t _circle2[5][2] = {{-1,2},{-2,3},{-2,3},{-2,3},{-1,2}};
static int32_t _circle3[7][2] = {{-1,2},{-2,3},{-3,4},{-3,4},
				 {-3,4},{-2,3},{-1,2}};
static int32_t _circle4[9][2] = {{-1,2},{-3,4},{-3,4},{-4,5},{-4,5},
				 {-4,5},{-3,4},{-3,4},{-1,2}};

static int32_t* _pre_drawn_circle[5] = {
	(int32_t*)_circle0,
	(int32_t*)_circle1,
	(int32_t*)_circle2,
	(int32_t*)_circle3,
	(int32_t*)_circle4
};


/*
 * Characteristic of drawing line in "writer"
 * - length of line is usually very short. But number of lines are huge.
 *  (So, we can expect that loop count for each "_draw_basic_line"
 *    is not big!!!)
 */

/* 'thick' is 'radius'!! */
static void
_draw_line(int32_t* pixels, int32_t w, int32_t h,
	   int32_t color, int32_t thick,
	   int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	bool    b01swap; /* (x0, y0) and (x1, y1) is swapped or not */
	int32_t	tmp;
	int8_t  steep;

	b01swap = false;
	steep = ABS(y1 - y0) > ABS(x1 - x0);
	/* Check trivial case. */
	if (steep){
		SWAP(x0, y0, tmp);
		SWAP(x1, y1, tmp);
	}

	/* sorting by x */
	if (x0 > x1){
		b01swap = true;
		SWAP(x0, x1, tmp);
		SWAP(y0, y1, tmp);
	}

	{ /* Just Scope */
#define PLOT(x,y)							\
		ys = ((y - thick) < 0)? -y: -thick;			\
		ye = ((y + thick) >= h)? h - y: thick + 1;		\
		pdc = _pre_drawn_circle[thick] + 2 * (ys + thick);	\
		ys += y;						\
		ye += y;						\
		for (i = ys; i < ye; i++) {				\
			xs = *pdc++ + x;				\
			xe = *pdc++ + x;				\
			if (xs < 0)					\
				xs = 0;					\
			if (xe > w)					\
				xe = w;					\
			for (j = xs; j < xe; j++)			\
				pixels[i * w + j] = color;		\
		}

		int32_t* pdc = _pre_drawn_circle[thick];
		int32_t deltax = x1 - x0;
		int32_t deltay = ABS(y1 - y0);
		/*
		 * original error = -(deltax + 1)/2.
		 * Buf for integer operation only...
		 */
		int32_t error = -(deltax + 1);
		int32_t ystep;
		int32_t y = y0;
		int32_t x, ys, ye, xs, xe;
		register int32_t i, j;

		if (y0 < y1)
			ystep = 1;
		else
			ystep = -1;

		deltax *= 2;
		deltay *= 2;

		/* To handle this fact "(x1, y1) is open" */
		if (b01swap)
			x0++;
		else
			x1--;

		if (steep) {
			for (x = x0; x <= x1; x++) {
				PLOT(y, x);
				/*
				 * because 'error' is double of real
				 *   error value.
				 */
				error += deltay;
				if (error >= 0) {
					y += ystep;
					error -= deltax;
				}
			}
		} else {
			for (x = x0; x <= x1; x++) {
				PLOT(x, y);
				/*
				 * because 'error' is double of real
				 *   error value.
				 */
				error += deltay;
				if (error >= 0) {
					y += ystep;
					error -= deltax;
				}
			}
		}
#undef PLOT
	} /* Just Scope */
}

void
draw_line(int32_t* pixels, int32_t w, int32_t h,
	  int32_t color,  uint8_t thick,
	  int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	_draw_line(pixels, w, h, color, thick, x0, y0, x1, y1);
}

