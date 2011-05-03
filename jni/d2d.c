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

#include <stdio.h>
#include <math.h>
#include <string.h>

#include "common.h"

#ifdef CONFIG_DUALCORE
#	include <pthread.h>
#endif /* CONFIG_DUALCORE */


static void
_fill_rect_(int* pixels, int w, int h,
	   int color,
	   int l, int t, int r, int b) {
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

	register int*	ds = pixels;
	register int*	de;
	int*		d_finish;
	register int	col = color;
	int		rw = r - l;
	int		gap = w - rw;
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
	int* pixels;
	int w, h, color, l, t, r, b;
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


static void
_fill_rect(int* pixels, int w, int h,
	   int color,
	   int l, int t, int r, int b) {

#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_check_start(DBG_PERF_FILL_RECT);
#endif /* CONFIG_DBG_STATISTICS */

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

#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_check_end(DBG_PERF_FILL_RECT);
#endif /* CONFIG_DBG_STATISTICS */

}

static void
_copy_rect(int* dst, const int* src,
	   int dw, int dh, int dl, int dt,
	   int sw, int sh, int sl, int st,
	   int width, int height) {
	register const int* ss = src;
	register int*	    ds = dst + dw * dt + dl;
	register int	    s_gap = sw - width;
	register int	    d_gap = dw - width;
	const int*	    s_finish;

	s_finish = ss + sw * (height + st - 1) + sl + width;
	ss = ss + sw * st + sl;
	while (ss < s_finish) {
		memcpy(ds, ss, width * sizeof(*ds));
		ss += width + s_gap;
		ds += width + d_gap;
	}
}


/* pre-calculated value for filling circle!! : to increase performance!! */
static int _circle0[1][2] = {{0,1}};
static int _circle1[3][2] = {{0,1},{-1,2},{0,1}};
static int _circle2[5][2] = {{-1,2},{-2,3},{-2,3},{-2,3},{-1,2}};
static int _circle3[7][2] = {{-1,2},{-2,3},{-3,4},{-3,4},{-3,4},{-2,3},{-1,2}};
static int _circle4[9][2] = {{-1,2},{-3,4},{-3,4},{-4,5},{-4,5},
			     {-4,5},{-3,4},{-3,4},{-1,2}};

static int* _pre_drawn_circle[5] = {
	(int*)_circle0,
	(int*)_circle1,
	(int*)_circle2,
	(int*)_circle3,
	(int*)_circle4
};


/*
 * Characteristic of drawing line in "writer"
 * - length of line is usually very short. But number of lines are huge.
 *  (So, we can expect that loop count for each "_draw_basic_line"
 *    is not big!!!)
 */

/* 'thick' is 'radius'!! */
static void
_draw_line(int* pixels, int w, int h,
	   int color, int thick,
	   int x0, int y0, int x1, int y1) {
	int	tmp;
	char	steep;

	steep = ABS(y1 - y0) > ABS(x1 - x0);
	/* Check trivial case. */
	if (steep){
		SWAP(x0, y0, tmp);
		SWAP(x1, y1, tmp);
	}

	/* sorting by x */
	if (x0 > x1){
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

		int* pdc = _pre_drawn_circle[thick];
		int deltax = x1 - x0;
		int deltay = ABS(y1 - y0);
		/*
		 * original error = -(deltax + 1)/2.
		 * Buf for integer operation only...
		 */
		int error = -(deltax + 1);
		int ystep;
		int y = y0;
		int x, ys, ye, xs, xe;
		register int i, j;

		if (y0 < y1)
			ystep = 1;
		else
			ystep = -1;

		deltax *= 2;
		deltay *= 2;
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
draw_line(int* pixels, int w, int h,
	  int color, char thick,
	  int x0, int y0, int x1, int y1) {
	_draw_line(pixels, w, h, color, thick, x0, y0, x1, y1);
}


#include "jni.h"
/*
 * NOTE !!!
 * If array is passed as copy..... performance can be great problem..!!!
 * (In Eclair, memory is pinned.!! - so Ok)
 */

/*
 * Class:     com_yhc_writer_D2d
 * Method:    _nativeFill
 * Signature: ([IIIIIIII)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_D2d__1nativeFill(JNIEnv* env, jclass jclazz,
				     jintArray jarr, jint w, jint h,
				     jint color,
				     jint l, jint t, jint r, jint b) {
	jint* pixels = (*env)->GetIntArrayElements(env, jarr, NULL);

	_fill_rect(pixels, w, h, color, l, t, r, b);

	(*env)->ReleaseIntArrayElements(env, jarr, pixels, JNI_ABORT);
}

/*
 * Class:     com_yhc_writer_D2d
 * Method:    _nativeCopy
 * Signature: ([I[IIIIIIIIIII)V
p */
JNIEXPORT void JNICALL
Java_com_yhc_writer_D2d__1nativeCopy(JNIEnv* env, jclass jclazz,
				     jintArray jarr_dst, jintArray jarr_src,
				     jint dw, jint dh, jint dl, jint dt,
				     jint sw, jint sh, jint sl, jint st,
				     jint width, jint height) {
	jint* dst_pixels = (*env)->GetIntArrayElements(env, jarr_dst, NULL);
	jint* src_pixels = (*env)->GetIntArrayElements(env, jarr_src, NULL);

	_copy_rect(dst_pixels, src_pixels,
		   dw, dh, dl, dt,
		   sw, sh, sl, st,
		   width, height);

	(*env)->ReleaseIntArrayElements(env, jarr_dst, dst_pixels, JNI_ABORT);
	(*env)->ReleaseIntArrayElements(env, jarr_src, src_pixels, JNI_ABORT);

}

/*
 * Class:     com_yhc_writer_D2d
 * Method:    _nativeDrawLine
 * Signature: ([IIIIBIIII)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_D2d__1nativeDrawLine(JNIEnv* env, jclass jclazz,
					   jintArray jarr, jint w, jint h,
					   jint color, jbyte thick,
					   jint x0, jint y0,
					   jint x1, jint y1) {
	jint* pixels = (*env)->GetIntArrayElements(env, jarr, NULL);

	draw_line(pixels, w, h, color, thick, x0, y0, x1, y1);

	(*env)->ReleaseIntArrayElements(env, jarr, pixels, JNI_ABORT);

}

