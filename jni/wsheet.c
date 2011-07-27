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
#include <malloc.h>

#include "common.h"
#include "list.h"

#ifdef CONFIG_DUALCORE
#	include <pthread.h>
#endif /* CONFIG_DUALCORE */

/*
 * Top two priority!!!
 *     Saving memory! Increasing speed!
 *     (Number of lines can be reachs to 1,000,000!! We should consider this!)
 */

/*
 * x-sorted. That is, (x0 <= x1) should be alwasy true!.
 */
struct line {
	unsigned short	    x0, y0, x1, y1;
	unsigned short	    color; /* 16bit color */
	unsigned char	    alpha; /* alpha value of color (not used yet) */
	unsigned char	    thick; /* 1~255 is enough! */

	struct div*         div;
	struct list_link*   divlk;
};

struct rect {
	int l, t, r, b;
};

struct node {
	struct line*       ln;
	struct list_link   lk;
};

struct div {
	struct list_link   lns;
	struct rect        boundary;
};

struct wsheet {
	int	     divW, divH, colN, rowN;
	struct div** divs; /* divs[row][col] */
};

/*
 * Flag to check that this wsheet static values are initialized or not
 */
static int _initialized = 0;

#ifdef CONFIG_MEMPOOL
static struct mp* _nmp = NULL; /* node memory pool */
#endif /* CONFIG_MEMPOOL */

static void
_init(void) {
	ASSERT(!_initialized);
	_initialized = 1;

#ifdef CONFIG_MEMPOOL
	_nmp = mp_create(256 * 1024, sizeof(struct node));
#endif /* CONFIG_MEMPOOL */
}

static void _deinit(void) __attribute__((unused));
static void
_deinit(void) {
	if (!_initialized)
		return;
	_initialized = 0;

#ifdef CONFIG_MEMPOOL
	mp_destroy(_nmp);
#endif /* CONFIG_MEMPOOL */
}



static inline struct node*
_node(struct list_link* link) {
	return container_of(link, struct node, lk);
}

/********************************
 * Common basic functions
 ********************************/

static inline int
_rbg16to32(short color) {
	return 0xff000000
		| ((color & 0xf800) << 8)
		| ((color & 0x07e0) << 5)
		| ((color & 0x001f) << 3);
}


/********************************
 * Functions for line
 ********************************/
static inline void
_line_set(struct line* ln,
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
_line_boundary(const struct line* ln, struct rect* out) {
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
_rect_is_empty(const struct rect* r) {
	return r->l >= r->r || r->t >= r->b;
}

static inline int
_rect_contains(const struct rect* r, int x, int y) {
	return (x >= r->l && x < r->r && y >= r->t && y < r->b);
}

static inline void
_rect_set(struct rect* pr, int l, int t, int r, int b) {
	pr->l = l;
	pr->t = t;
	pr->r = r;
	pr->b = b;
}

static inline void
_rect_set_empty(struct rect* r) {
	r->l = r->r = 0;
	r->t = r->b = 0;
}

/********************************
 * Functions for list
 ********************************/

static inline void
_list_clean(struct list_link* head) {
	struct node* n;
	struct node* tmp;
	list_foreach_item_removal_safe(n, tmp, head, struct node, lk) {
#ifdef CONFIG_MEMPOOL
		mp_put(_nmp, n);
#else /* CONFIG_MEMPOOL */
		/* freeing memory for Line should be done elsewhere! */
		FREE(n);
#endif /* CONFIG_MEMPOOL */
	}
	list_init_link(head);
}

static void
_list_add_line(struct list_link* head, struct line* ln) {
	struct node* n;
#ifdef CONFIG_MEMPOOL
	n = (struct node*)mp_get(_nmp);
#else /* CONFIG_MEMPOOL */
	MALLOC(n, struct node*, sizeof(*n));
#endif /* CONFIG_MEMPOOL */
	n->ln = ln;
	list_add_last(head, &n->lk);
}

static inline void
_list_add_list(struct list_link* head, const struct list_link* in) {
	struct node* n;
	list_foreach_item(n, in, struct node, lk) {
		_list_add_line(head, n->ln);
	}
}

static inline void
_list_absorb_list(struct list_link* head, struct list_link* in) {
	/* absorb 'in' */

	/* if list size > 0 */
	if (in->_next != in) {
		head->_prev->_next = in->_next;
		in->_next->_prev = head->_prev;
		head->_prev = in->_prev;
		head->_prev->_next = head;
	}
	/* make absorbed list be empty */
	list_init_link(in);
}


/********************************
 * Common utility functions
 ********************************/
extern int g2d_splitX(int* out_intersecty,
		      int x0, int y0,
		      int x1, int y1,
		      int x,
		      int yt, int yb);
extern int g2d_splitY(int* out_intersectx,
		      int x0, int y0,
		      int x1, int y1,
		      int y,
		      int xl, int xr);

static inline int
_splitX(int* out_intersecty, struct line* ln, int x, int yt, int yb) {
	return g2d_splitX(out_intersecty,
			  ln->x0, ln->y0,
			  ln->x1, ln->y1,
			  x,
			  yt, yb);
}

static inline int
_splitY(int* out_intersectx, struct line* ln, int y, int xl, int xr) {
	return g2d_splitY(out_intersectx,
			  ln->x0, ln->y0,
			  ln->x1, ln->y1,
			  y,
			  xl, xr);
}


/********************************
 * Functions for div
 ********************************/
static inline void
_div_init(struct div* div, int l, int t, int r, int b) {
	list_init_link(&div->lns);
	_rect_set(&div->boundary, l, t, r, b);
}

static inline void
_div_clean(struct div* div) {
	struct node* n;
	list_foreach_item(n, &div->lns, struct node, lk) {
		FREE(n->ln);
	}
	_list_clean(&div->lns);
}

static inline void
_div_add(struct div* div, struct line* ln) {
	ln->div = div;
	_list_add_line(&div->lns, ln);
	ln->divlk = div->lns._prev;
}


static void
_div_find_lines(struct div* div,
		struct list_link* out,
		int l, int t, int r, int b) {
	struct node*   n;
	struct line*   ln;
	struct rect    rect;
	char	       b_intersected;

	_rect_set(&rect, l, t, r, b);

	list_foreach_item(n, &div->lns, struct node, lk) {
		ln = n->ln;
		b_intersected = 0;
		/* Check that line is expands on this rectangle region. */
		if (_rect_contains(&rect, ln->x0, ln->y0)
		     || _rect_contains(&rect, ln->x1, ln->y1)) {
			/* trivial case! */
			b_intersected = 1;
		} else {
			int intersect;;
			if (_splitX(&intersect, ln, l, t, b)
			    || _splitX(&intersect, ln, r, t, b)
			    || _splitY(&intersect, ln, t, l, r)
			    || _splitY(&intersect, ln, b, l, r)) {
				b_intersected = 1;
			}
		}
		if (b_intersected)
			_list_add_line(out, ln);
	}
}

/********************************
 * Functions for struct wsheet
 ********************************/
static inline struct wsheet*
_wsheet_create(void) {
	struct wsheet* sh;
	MALLOC(sh, struct wsheet*, sizeof(*sh));

	if (!_initialized)
		_init();

	return sh;
}

static inline void
_wsheet_init(struct wsheet* wsh, int divW, int divH, int colN, int rowN) {
	int i, j;

	wsh->divW = divW;
	wsh->divH = divH;
	wsh->colN = colN;
	wsh->rowN = rowN;
	MALLOC(wsh->divs, struct div**, sizeof(*wsh->divs) * rowN);
	for (i = 0; i < rowN; i++) {
		MALLOC(wsh->divs[i],
		       struct div*,
		       sizeof(*wsh->divs[i]) * colN);
		for (j = 0; j < colN; j++) {
			_div_init(&wsh->divs[i][j],
				  j * divW,
				  i * divH,
				  (j + 1) * divW,
				  (i + 1) * divH);
		}
	}
}

static inline void
_wsheet_clean(struct wsheet* wsh) {
	int i,j;
	for (i = 0; i < wsh->rowN; i++)
		for (j = 0; j < wsh->colN; j++)
			_div_clean(&wsh->divs[i][j]);
}

static inline void
_wsheet_destroy(struct wsheet* wsh) {
	int i,j;
	for (i = 0; i < wsh->rowN; i++) {
		for (j = 0; j < wsh->colN; j++)
			_div_clean(&wsh->divs[i][j]);
		FREE(wsh->divs[i]);
	}
	FREE(wsh->divs);
	FREE(wsh);
}

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
	int c, r;
	c = ln->x0 / wsh->divW;
	if (ln->y0 < ln->y1)
		r = ln->y0 / wsh->divH;
	else
		r = ln->y1 / wsh->divH;
	return &wsh->divs[r][c];
}

static inline int
_wsheet_splitX(struct wsheet* wsh,
	       int* out_intersecty,
	       struct line* ln, int x) {
	return _splitX(out_intersecty, ln, x, 0, wsh->divH * wsh->rowN);
}


static inline int
_wsheet_splitY(struct wsheet* wsh,
	       int* out_intersectx,
	       struct line* ln, int y) {
	return _splitY(out_intersectx, ln, y, 0, wsh->divW * wsh->colN);
}


static void
_wsheet_add(struct wsheet* wsh,
	    int x0, int y0,
	    int x1, int y1,
	    char thick,
	    unsigned short color) {

	struct list_link   list;
	struct list_link   splits;
	struct node*       n;
	struct line*       l;
	struct line*       tmpl;
	int	           v, intersect, i;
	int                vmin, vmax; /* value min/max */

	list_init_link(&list);
	list_init_link(&splits);

	/*
	 * NOTE :
	 *     check it here??? ==> should be checked in java!!
	 *     x0, y0 ... value should not be larger than LIMIT
	 *	 of unsigned short!!!
	 */
	MALLOC(l, struct line*, sizeof(*l));
	_line_set(l, x0, y0, x1, y1);
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
			_list_add_line(&list, l);
			break;
		}

		/* keep doing with remained one! */
		MALLOC(tmpl, struct line*, sizeof(*tmpl));
		if (l->y0 < l->y1) {
			_line_set(tmpl, l->x0, l->y0, intersect, v);
			_line_set(l, intersect, v, l->x1, l->y1);
		} else {
			_line_set(tmpl, l->x1, l->y1, intersect, v);
			_line_set(l, intersect, v, l->x0, l->y0);
		}
		tmpl->thick = thick;
		tmpl->color = color;
		_list_add_line(&list, tmpl);
	}

	/* check vertical - column - split */
	list_foreach_item(n, &list, struct node, lk) {
		l = n->ln;

		vmin = l->x0 / wsh->divW;
		vmax = l->x1 / wsh->divW;

		for (i = vmin; i <= vmax; i++) {
			v = (i + 1) * wsh->divW;
			if (!_wsheet_splitX(wsh, &intersect, l, v)) {
				_list_add_line(&splits, l);
				break;
			}

			MALLOC(tmpl, struct line*, sizeof(*tmpl));
			tmpl->thick = thick;
			tmpl->color = color;
			_line_set(tmpl, l->x0, l->y0, v, intersect);
			_list_add_line(&splits, tmpl);
			_line_set(l, v, intersect, l->x1, l->y1);
		}
	}
	_list_clean(&list);

	/* assign to division */
	list_foreach_item(n, &splits, struct node, lk) {
		_div_add(_wsheet_div(wsh, n->ln), n->ln);
	}
	_list_clean(&splits);
}


/*
 * find lines in the given rectangle division group
 * (bi and ri are NOT included)
 */
static inline void
_wsheet_find_lines_(struct wsheet* wsh, struct list_link* out,
		    int ti, int bi, int li, int ri) {
	int i, j;
	/* fully included division - we don't need to check. */
	for (i = ti; i < bi; i++)
		for (j = li; j < ri; j++)
			_list_add_list(out, &wsh->divs[i][j].lns);
}

#if defined(CONFIG_DUALCORE) && !defined(CONFIG_ARCH_ARM)

struct _wsheet_find_lines_arg {
	struct wsheet*     wsh;
	struct list_link*  out;
	int                ti, bi, li, ri;
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
static void
_wsheet_find_lines(struct wsheet* wsh, struct list_link* out,
		   int l, int t, int r, int b) {

#define __horizontal_strip()						\
	do {								\
		/* horizontal strip */					\
		for (i = li; i <= ri; i++) {				\
			/* top strip */					\
			_div_find_lines(&wsh->divs[ti][i], out, l, t, r, b); \
			if (bi > ti) {					\
				/* bottom strip */			\
				_div_find_lines(&wsh->divs[bi][i], out,	\
						l, t, r, b);		\
			}						\
		}							\
	} while (0)

#define __vertical_strip()						\
	do {								\
		/* vertical strip except for 4-vertex-position in rect. */ \
		for (i = ti + 1; i < bi; i++) {				\
			/* left strip */				\
			_div_find_lines(&wsh->divs[i][li], out, l, t, r, b); \
			if (ri > li) {					\
				/* right strip */			\
				_div_find_lines(&wsh->divs[i][ri], out,	\
						l, t, r, b);		\
			}						\
		}							\
	} while (0)


	int i;
	/* left index */
	int li = l / wsh->divW;
	/* top index */
	int ti = t / wsh->divH;
	/* right index (exclude right line); */
	int ri = (0 == r % wsh->divW)? r / wsh->divW - 1: r / wsh->divW;
	/* bottom index (exclude bottom line); */
	int bi = (0 == b % wsh->divH)? b / wsh->divH - 1: b / wsh->divH;

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
			ASSERT(0);

		/* Not fully included division. - we need to check */

		__horizontal_strip();
		__vertical_strip();

		if (pthread_join(thd, &ret))
			ASSERT(0);

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

static void
_wsheet_cutout(struct wsheet* wsh, int l, int t, int r, int b) {
	struct list_link   lns;
	struct line*       oln; /* original line */
	struct line*       ln;
	struct line*       tmpl;
	int                intersect;
	struct node*       n;

	list_init_link(&lns);

	_wsheet_find_lines(wsh, &lns, l, t, r, b);

	list_foreach_item(n, &lns, struct node, lk) {
		ln = oln = n->ln;

		/* check for all for edge. */
		if (_splitX(&intersect, ln, l, t, b)) {
			/* NOTE: X-sorted */
			MALLOC(tmpl, struct line*, sizeof(*tmpl));
			_line_set(tmpl, ln->x0, ln->y0, l, intersect);
			tmpl->thick = oln->thick;
			tmpl->color = oln->color;
			_div_add(oln->div, tmpl);
			_line_set(ln, l, intersect, ln->x1, ln->y1);
		}
		if (_splitX(&intersect, ln, r, t, b)) {
			/* NOTE X-sorted */
			MALLOC(tmpl, struct line*, sizeof(*tmpl));
			_line_set(tmpl, ln->x1, ln->y1, r, intersect);
			tmpl->thick = oln->thick;
			tmpl->color = oln->color;
			_div_add(oln->div, tmpl);
			_line_set(ln, r, intersect, ln->x0, ln->y0);
		}
		if (_splitY(&intersect, ln, t, l, r)) {
			MALLOC(tmpl, struct line*, sizeof(*tmpl));
			tmpl->thick = oln->thick;
			tmpl->color = oln->color;
			if (ln->y0 < ln->y1) {
				_line_set(tmpl, ln->x0, ln->y0, intersect, t);
				_line_set(ln, ln->x1, ln->y1, intersect, t);
			} else {
				_line_set(tmpl, ln->x1, ln->y1, intersect, t);
				_line_set(ln, ln->x0, ln->y0, intersect, t);
			}
			_div_add(oln->div, tmpl);
		}
		if (_splitY(&intersect, ln, b, l, r)) {
			MALLOC(tmpl, struct line*, sizeof(*tmpl));
			tmpl->thick = oln->thick;
			tmpl->color = oln->color;
			if (ln->y0 < ln->y1) {
				_line_set(tmpl, ln->x1, ln->y1, intersect, b);
				_line_set(ln, ln->x0, ln->y0, intersect, b);
			} else {
				_line_set(tmpl, ln->x0, ln->y0, intersect, b);
				_line_set(ln, ln->x1, ln->y1, intersect, b);
			}
			_div_add(oln->div, tmpl);
		}

		/* free link */
		list_del(oln->divlk);
#ifdef CONFIG_MEMPOOL
		mp_put(_nmp, _node(oln->divlk));
#else /* CONFIG_MEMPOOL */
		FREE(_node(oln->divlk));
#endif /* CONFIG_MEMPOOL */
		FREE(oln);
	}

	_list_clean(&lns);
}



#include "jni.h"


/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeCreateWsheet
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeCreateWsheet(JNIEnv* env, jclass jclazz) {
	return (jint)_wsheet_create();
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeInitWsheet
 * Signature: (IIIII)I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeInitWsheet(JNIEnv* env, jclass jclazz,
					      jint sheet,
					      jint divW, jint divH,
					      jint colN, jint rowN) {
#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_init();
#endif /* CONFIG_DBG_STATISTICS */

	_wsheet_init((struct wsheet*)sheet, divW, divH, colN, rowN);
	return 1;
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeDestroyWsheet
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_WSheet__1nativeDestroyWsheet(JNIEnv* env, jclass jclazz,
						 jint sheet) {
	_wsheet_destroy((struct wsheet*)sheet);
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeWidth
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeWidth(JNIEnv* env, jclass jclazz,
					 jint sheet) {
	return ((struct wsheet*)sheet)->divW * ((struct wsheet*)sheet)->colN;
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeHeight
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeHeight(JNIEnv* env, jclass jclazz,
					  jint sheet) {
	return ((struct wsheet*)sheet)->divH * ((struct wsheet*)sheet)->rowN;
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeCutout
 * Signature: (IIIII)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_WSheet__1nativeCutout(JNIEnv* env, jclass jclazz,
					    jint sheet,
					    jint l, jint t, jint r, jint b) {
#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_print(DBG_PERF_FILL_RECT);
	dbg_tpf_print(DBG_PERF_FIND_LINE);
	dbg_tpf_print(DBG_PERF_DRAW_LINE);
#endif /* CONFIG_DBG_STATISTICS */
	_wsheet_cutout((struct wsheet*)sheet, l, t, r, b);
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeAdd
 * Signature: (IIIIICS)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_WSheet__1nativeAdd(JNIEnv* env, jclass jclazz,
				       jint sheet,
				       jint x0, jint y0,
				       jint x1, jint y1,
				       jbyte thick, jshort color) {
	_wsheet_add((struct wsheet*)sheet, x0, y0, x1, y1, thick, color);
}


extern void
draw_line(int* pixels, int w, int h,
	  int color, char thick,
	  int x0, int y0, int x1, int y1);


#ifdef CONFIG_DUALCORE
struct _draw_arg {
	struct list_link* head;
	struct list_link* lk;
	int*              pixels;
	int               w, h, ox, oy;
	float             zf;
};

static void
_draw_(struct list_link* head,
       struct list_link* lk,
       int* pixels,
       int w, int h, int ox, int oy,
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

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeDraw
 * Signature: (I[IIIIIIIIIF)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_WSheet__1nativeDraw(JNIEnv* env, jclass jclazz,
					jint sheet,
					jintArray jarr,
					jint w, jint h,
					jint ox, jint oy,
					jint l, jint t, jint r, jint b,
					jfloat zf) {

	jint* pixels = (*env)->GetIntArrayElements(env, jarr, NULL);

	struct wsheet*     wsh = (struct wsheet*)sheet;
	struct list_link   lns;

#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_check_start(DBG_PERF_DRAW_LINE);
#endif /* CONFIG_DBG_STATISTICS */

	list_init_link(&lns);

	_wsheet_find_lines(wsh, &lns, l, t, r, b);


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
			ASSERT(0);

		_draw_(&lns, lns._next->_next, pixels, w, h, ox, oy, zf);

		if (pthread_join(thd, &ret))
			ASSERT(0);

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

	_list_clean(&lns);

#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_check_end(DBG_PERF_DRAW_LINE);
#endif /* CONFIG_DBG_STATISTICS */

	(*env)->ReleaseIntArrayElements(env, jarr, pixels, JNI_ABORT);
}



/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeSave
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeSave(JNIEnv* env, jclass jclazz,
					jint sheet, jstring jpath) {

	int             ret  = 0;
	const char*     path = (*env)->GetStringUTFChars(env, jpath, NULL);
	FILE*           fp   = fopen(path, "w");
	struct wsheet*  wsh  = (struct wsheet*)sheet;
	struct node*    n;
	struct line*    ln;
	int	        i, j, sz;

	if (NULL == fp)
		goto bail;

#define _FW(vAL)  fwrite(&(vAL), sizeof(vAL), 1, fp)

	_FW(wsh->divW);
	_FW(wsh->divH);
	_FW(wsh->colN);
	_FW(wsh->rowN);

	for (i = 0; i < wsh->rowN; i++) {
		for (j = 0; j < wsh->colN; j++) {
			sz = list_size(&wsh->divs[i][j].lns);
			_FW(sz);
			list_foreach_item(n,
					  &wsh->divs[i][j].lns,
					  struct node,
					  lk) {
				ln = n->ln;
				_FW(ln->x0);
				_FW(ln->y0);
				_FW(ln->x1);
				_FW(ln->y1);
				_FW(ln->color);
				_FW(ln->thick);
				_FW(ln->alpha);
			}

		}
	}

#undef _FW

	ret = 1; // success

 bail:
	if (NULL != fp)
		fclose(fp);
	(*env)->ReleaseStringUTFChars(env, jpath, path);
	return ret;
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeLoad
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeLoad(JNIEnv* env, jclass jclazz,
					jint sheet, jstring jpath) {

	int             ret  = 0;
	const char*     path = (*env)->GetStringUTFChars(env, jpath, NULL);
	FILE*           fp   = fopen(path, "r");
	struct wsheet*  wsh  = (struct wsheet*)sheet;
	struct line*    ln;
	int	        i, j, k, sz;

	if (NULL == fp)
		goto bail;

#define _FR(vAL)  fread(&(vAL), sizeof(vAL), 1, fp)

	_FR(wsh->divW);
	_FR(wsh->divH);
	_FR(wsh->colN);
	_FR(wsh->rowN);

	_wsheet_init(wsh, wsh->divW, wsh->divH, wsh->colN, wsh->rowN);

	for (i = 0; i < wsh->rowN; i++) {
		for (j = 0; j < wsh->colN; j++) {
			_FR(sz);
			for (k = 0; k < sz; k++) {
				MALLOC(ln, struct line*, sizeof(*ln));
				_FR(ln->x0);
				_FR(ln->y0);
				_FR(ln->x1);
				_FR(ln->y1);
				_FR(ln->color);
				_FR(ln->thick);
				_FR(ln->alpha);
				_div_add(&wsh->divs[i][j], ln);
			}
		}
	}

#undef _FR

	ret = 1; // success

 bail:
	if (NULL != fp)
		fclose(fp);
	(*env)->ReleaseStringUTFChars(env, jpath, path);
	return ret;
}

































//============================= TEST START =================================

#ifdef CONFIG_TEST_EXECUTABLE

#include <assert.h>

static const int    _DIV_W  = 20;
static const int    _DIV_H  = 10;
static const int    _COL_N  = 10;
static const int    _ROW_N  = 10;

static void _line_sanity_check(struct wsheet* wsh)
{
	struct list_link   lout;
	struct node*       n;
	struct line*       ln;
	int	           minx, maxx, miny, maxy, li, ri, ti, bi;

	list_init_link(&lout);
	_wsheet_find_lines(wsh, &lout,
			   0, 0,
			   wsh->divW * wsh->colN,
			   wsh->divH * wsh->rowN);
	list_foreach_item(n, &lout, struct node, lk) {
		ln = n->ln;
		minx = MIN(ln->x0, ln->x1);
		maxx = MAX(ln->x0, ln->x1);
		miny = MIN(ln->y0, ln->y1);
		maxy = MAX(ln->y0, ln->y1);
		li = minx/wsh->divW;
		ri = (0 == maxx % wsh->divW)?
			maxx / wsh->divW - 1:
			maxx / wsh->divW;
		ti = miny / wsh->divH;
		bi = (0 == maxy % wsh->divH)?
			maxy / wsh->divH - 1:
			maxy / wsh->divH;
	}
	_list_clean(&lout);
}

static void _test_add(struct wsheet* wsh)
{
	struct list_link lout;

	list_init_link(&lout);
	/***************
	 * extends over only one division
	 ***************/
	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 1, 2, 2, 1, 0);

	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[1][1].lns)
	       );


	_wsheet_clean(wsh);
	_wsheet_add(wsh, _DIV_W-1, 2, 0, 1, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[1][1].lns)
	       );

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 0, 2, _DIV_H-1, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[1][1].lns)
	       );

	_wsheet_clean(wsh);
	_wsheet_add(wsh, _DIV_W-1, _DIV_H-1, 0, 0, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[1][1].lns)
	       );


	/***************
	 * extends over two division
	 ***************/
	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 1, _DIV_W + 1, 2, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 1 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[0][2].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[1][1].lns)
	       );

	_wsheet_clean(wsh);
	_wsheet_add(wsh, _DIV_W, 2, 1, 1, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[2][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[1][1].lns)
	       );

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 2, 1, 1, _DIV_H + 1, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 1 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[1][1].lns)
	       );

	_wsheet_clean(wsh);
	/* _DIV_W >= 2*_DIV_H */
	_wsheet_add(wsh, _DIV_W, _DIV_H, _DIV_W*2, _DIV_H*2, 1, 0);
	ASSERT(0 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 1 == list_size(&wsh->divs[1][1].lns)
	       && 0 == list_size(&wsh->divs[2][1].lns)
	       && 0 == list_size(&wsh->divs[1][2].lns)
	       && 0 == list_size(&wsh->divs[2][2].lns)
	       && 0 == list_size(&wsh->divs[3][2].lns)
	       && 0 == list_size(&wsh->divs[2][3].lns)
	       && 0 == list_size(&wsh->divs[3][3].lns)
	       );

	/***************
	 * extends over three division
	 ***************/
	_wsheet_clean(wsh);
	_wsheet_add(wsh, _DIV_W*2+1, 2, 1, 1, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 1 == list_size(&wsh->divs[0][1].lns)
	       && 1 == list_size(&wsh->divs[0][2].lns)
	       && 0 == list_size(&wsh->divs[0][3].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[2][0].lns)
	       && 0 == list_size(&wsh->divs[3][0].lns)
	       );
	_line_sanity_check(wsh);



	_wsheet_clean(wsh);
	_wsheet_add(wsh, 2, _DIV_H*2+1, 1, 1, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[0][2].lns)
	       && 0 == list_size(&wsh->divs[0][3].lns)
	       && 1 == list_size(&wsh->divs[1][0].lns)
	       && 1 == list_size(&wsh->divs[2][0].lns)
	       && 0 == list_size(&wsh->divs[3][0].lns)
	       );
	_line_sanity_check(wsh);

	_wsheet_clean(wsh);
	_wsheet_add(wsh, _DIV_W-1, 1, _DIV_W*2-1, _DIV_H+1, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 1 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 1 == list_size(&wsh->divs[1][1].lns)
	       );
	_line_sanity_check(wsh);

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, _DIV_H-1, _DIV_W+1, _DIV_H+1, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 1 == list_size(&wsh->divs[1][0].lns)
	       && 1 == list_size(&wsh->divs[1][1].lns)
	       );
	_line_sanity_check(wsh);

	/***************
	 * extends over several division
	 ***************/
	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, _DIV_H-1, _DIV_W*3+1, _DIV_H*2+1, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[0][2].lns)
	       && 0 == list_size(&wsh->divs[0][3].lns)
	       && 1 == list_size(&wsh->divs[1][0].lns)
	       && 1 == list_size(&wsh->divs[1][1].lns)
	       && 1 == list_size(&wsh->divs[1][2].lns)
	       && 0 == list_size(&wsh->divs[1][3].lns)
	       && 0 == list_size(&wsh->divs[2][0].lns)
	       && 0 == list_size(&wsh->divs[2][1].lns)
	       && 1 == list_size(&wsh->divs[2][2].lns)
	       && 1 == list_size(&wsh->divs[2][3].lns)
	       && 0 == list_size(&wsh->divs[3][0].lns)
	       && 0 == list_size(&wsh->divs[3][1].lns)
	       && 0 == list_size(&wsh->divs[3][2].lns)
	       && 0 == list_size(&wsh->divs[3][3].lns)
	       );
	_line_sanity_check(wsh);

	_wsheet_clean(wsh);
	_wsheet_add(wsh, _DIV_W-1, 1, _DIV_W*2+1, _DIV_H*6-1, 1, 0);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[2][0].lns)
	       && 1 == list_size(&wsh->divs[0][1].lns)
	       && 1 == list_size(&wsh->divs[1][1].lns)
	       && 1 == list_size(&wsh->divs[2][1].lns)
	       && 1 == list_size(&wsh->divs[3][1].lns)
	       && 1 == list_size(&wsh->divs[4][1].lns)
	       && 1 == list_size(&wsh->divs[5][1].lns)
	       && 0 == list_size(&wsh->divs[6][1].lns)
	       && 0 == list_size(&wsh->divs[0][2].lns)
	       && 0 == list_size(&wsh->divs[1][2].lns)
	       && 0 == list_size(&wsh->divs[3][2].lns)
	       && 0 == list_size(&wsh->divs[4][2].lns)
	       && 1 == list_size(&wsh->divs[5][2].lns)
	       );
	_line_sanity_check(wsh);


	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, _DIV_H*4-1, _DIV_W*2+1, 1, 1, 0);
	ASSERT(0 == list_size(&wsh->divs[0][0].lns)
	       && 1 == list_size(&wsh->divs[3][0].lns)
	       && 1 == list_size(&wsh->divs[2][0].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[3][1].lns)
	       && 1 == list_size(&wsh->divs[2][1].lns)
	       && 1 == list_size(&wsh->divs[1][1].lns)
	       && 1 == list_size(&wsh->divs[0][1].lns)
	       && 1 == list_size(&wsh->divs[0][2].lns)
	       && 0 == list_size(&wsh->divs[1][2].lns)
	       );
	_line_sanity_check(wsh);

}

static void _test_find_lines(struct wsheet* wsh)
{
	struct list_link lns;

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 1, _DIV_W-1, 1, 1, 0);
	_wsheet_add(wsh, 1, 3, _DIV_W-1, _DIV_H-1, 1, 0);
	_wsheet_add(wsh, 1, _DIV_H-1, _DIV_W-1, _DIV_H-2, 1, 0);

	list_init_link(&lns);
	_wsheet_find_lines(wsh, &lns, 0, 0, 1, 1);
	ASSERT(0 == list_size(&lns));

	_list_clean(&lns);
	_wsheet_find_lines(wsh, &lns, 0, 0, 2, 2);
	ASSERT(1 == list_size(&lns));

	_list_clean(&lns);
	_wsheet_find_lines(wsh, &lns, 0, 0, 4, 4);
	ASSERT(2 == list_size(&lns));

	_list_clean(&lns);
	_wsheet_find_lines(wsh, &lns, 1, 1, 2, 2);
	ASSERT(1 == list_size(&lns));

	_list_clean(&lns);
	_wsheet_find_lines(wsh, &lns, 2, 2, 4, 4);
	ASSERT(1 == list_size(&lns));

	_list_clean(&lns);
	_wsheet_find_lines(wsh, &lns, 3, 0, 4, _DIV_H);
	ASSERT(3 == list_size(&lns));

	_list_clean(&lns);
}

static void _test_cutout(struct wsheet* wsh)
{
	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 1, 3, 3, 1, 0);
	_wsheet_cutout(wsh, 0, 0, _DIV_W, _DIV_H);
	ASSERT(0 == list_size(&wsh->divs[0][0].lns));

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 0, 0, 1, 1, 1, 0);
	_wsheet_cutout(wsh, 0, 0, _DIV_W, _DIV_H);
	ASSERT(0 == list_size(&wsh->divs[0][0].lns));

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 3, 3, _DIV_W, _DIV_H, 1, 0);
	_wsheet_cutout(wsh, 0, 0, _DIV_W, _DIV_H);
	ASSERT(0 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[1][1].lns)
	       );

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, _DIV_H, _DIV_W-1, _DIV_H, 1, 0);
	_wsheet_cutout(wsh, 0, 0, _DIV_W, _DIV_H);
	ASSERT(0 == list_size(&wsh->divs[0][0].lns)
	       && 1 == list_size(&wsh->divs[1][0].lns)
	       );

	_wsheet_clean(wsh);
	_wsheet_add(wsh, _DIV_W, 1, _DIV_W, _DIV_H-1, 1, 0);
	_wsheet_cutout(wsh, 0, 0, _DIV_W, _DIV_H);
	ASSERT(0 == list_size(&wsh->divs[0][0].lns)
	       && 1 == list_size(&wsh->divs[0][1].lns)
	       );


	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 1, _DIV_W, _DIV_H, 1, 0);
	_wsheet_cutout(wsh, 1, 1, _DIV_W, _DIV_H);
	ASSERT(0 == list_size(&wsh->divs[0][0].lns)
	       && 0 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[1][1].lns)
	       );

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 1, _DIV_W-2, 1, 1, 0);
	_wsheet_cutout(wsh, 2, 0, _DIV_W-1, 3);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns));

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 2, 1, _DIV_W-1, 1, 1, 0);
	_wsheet_cutout(wsh, 1, 0, _DIV_W-2, 3);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns));

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 1, _DIV_W-1, 1, 1, 0);
	_wsheet_cutout(wsh, 2, 0, _DIV_W-2, 3);
	ASSERT(2 == list_size(&wsh->divs[0][0].lns));


	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 1, 1, _DIV_H-2, 1, 0);
	_wsheet_cutout(wsh, 0, 2, 3, _DIV_H-1);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns));

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 2, 1, _DIV_H-1, 1, 0);
	_wsheet_cutout(wsh, 0, 1, 3, _DIV_H-2);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns));

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 1, 1, _DIV_H-1, 1, 0);
	_wsheet_cutout(wsh, 0, 2, 3, _DIV_H-2);
	ASSERT(2 == list_size(&wsh->divs[0][0].lns));

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 2, 2, _DIV_W-2, _DIV_H-2, 1, 0);
	_wsheet_cutout(wsh, 0, 0, _DIV_W-4, _DIV_H-3);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns));

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 2, 2, _DIV_W-2, _DIV_H-2, 1, 0);
	_wsheet_cutout(wsh, 0, 0, _DIV_W-1, _DIV_H-3);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns));

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 2, 2, _DIV_W-2, _DIV_H-2, 1, 0);
	_wsheet_cutout(wsh, 3, 3, _DIV_W-3, _DIV_H-3);
	ASSERT(2 == list_size(&wsh->divs[0][0].lns));

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 2, 2, _DIV_W*2-2, 3, 1, 0);
	_wsheet_cutout(wsh, 0, 0, _DIV_W*2-4, _DIV_H*2-3);
	ASSERT(0 == list_size(&wsh->divs[0][0].lns)
	       && 1 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[0][2].lns)
	       );

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 2, 2, 3, _DIV_H*2-2, 1, 0);
	_wsheet_cutout(wsh, 0, 0, _DIV_W-1, _DIV_H*2-3);
	ASSERT(0 == list_size(&wsh->divs[0][0].lns)
	       && 1 == list_size(&wsh->divs[1][0].lns)
	       );

	_wsheet_clean(wsh);
	_wsheet_add(wsh, _DIV_W-1, 1, _DIV_W*2+2, _DIV_H*3-2, 1, 0);
	_wsheet_cutout(wsh, 3, 4, _DIV_W*2, _DIV_H*2+1);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns)
	       && 1 == list_size(&wsh->divs[0][1].lns)
	       && 0 == list_size(&wsh->divs[1][0].lns)
	       && 0 == list_size(&wsh->divs[1][1].lns)
	       && 0 == list_size(&wsh->divs[1][2].lns)
	       && 1 == list_size(&wsh->divs[2][1].lns)
	       && 1 == list_size(&wsh->divs[2][2].lns)
	       );

	_wsheet_clean(wsh);
	_wsheet_add(wsh, 1, 1, 2, 1, 1, 0);
	_wsheet_add(wsh, 1, 1, 1, _DIV_H-1, 1, 0);
	_wsheet_cutout(wsh, 0, 0, 3, 3);
	ASSERT(1 == list_size(&wsh->divs[0][0].lns));

}

void test_wsheet(void)
{
	struct wsheet* wsh;
	wsh = _wsheet_create();
	_wsheet_init(wsh, 20, 10, 10, 10);
	_test_add(wsh);
	_test_find_lines(wsh);
	_test_cutout(wsh);

	_wsheet_destroy(wsh);
	_deinit();
}

#endif /* CONFIG_TEST_EXECUTABLE */
