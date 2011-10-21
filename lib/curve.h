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

#ifndef _CURVe_h_
#define _CURVe_h_

#include "common.h"
#include "list.h"

#include "g2d.h"
#include "gtype.h"

struct curve {
	/*
	 * It's very rare that curve is used out of linked list.
	 * And, it's not serious overhead in terms of memory.
	 */
	struct list_link lk;
	uint16_t         ref;   /* reference count */
	uint16_t         color; /* 16bit color */
	uint8_t	         alpha; /* alpha value of color (not used yet) */
	uint8_t	         thick; /* 1~255 is enough! */
	uint16_t         nrpts; /* # of points in pts */
	struct point     pts[1];
};

#define crv_foreach(crv, hd)				\
	list_foreach_item(crv, hd, struct curve, lk)

#define crv_foreach_removal_safe(crv, tmp, hd)				\
	list_foreach_item_removal_safe(crv, tmp, hd, struct curve, lk)

/*
 * @pt, @ptend : struct point*
 */
#define crv_foreach_point(crv, pt, ptend)			\
	for ((pt) = (crv)->pts, (ptend) = (pt) + crv->nrpts;	\
	     (pt) < (ptend);					\
	     ++(pt))

/*
 * @pt0, @pt1, @ptend : struct point*
 */
#define crv_foreach_point2(crv, pt0, pt1, ptend)		\
	for ((pt0) = (crv)->pts, (pt1) = (pt0) + 1,		\
		     (ptend) = (crv)->pts + (crv)->nrpts - 1;	\
	     (pt0) < (ptend);					\
	     ++(pt0), ++(pt1))
/*
 * Get X value of point
 */
static inline int32_t
crvX(const struct curve* c, uint16_t pti) {
	return c->pts[pti].x;
}

static inline int32_t
crvY(const struct curve* c, uint16_t pti) {
	return c->pts[pti].y;
}

static inline uint32_t
crv_sizeof(uint16_t nrpts) {
	return sizeof(struct curve) + sizeof(struct point) * (nrpts - 1);
}

static inline struct curve*
crvlk(struct list_link* lk) {
	return container_of(lk, struct curve, lk);
}

static inline struct curve*
crv_prev(struct curve* crv) {
	return crvlk(crv->lk._prev);
}

static inline struct curve*
crv_next(struct curve* crv) {
	return crvlk(crv->lk._next);
}

static inline void
crv_init(struct curve* crv, uint16_t nrpts) {
	list_init_link(&crv->lk);
	crv->nrpts = nrpts;
	crv->ref = 0;
	crv->color = 0; /* black by default */
	crv->thick = 0; /* 0 by default */
	crv->alpha = 0xff;
}

static inline struct curve*
crv_create(uint16_t nrpts) {
	struct curve * c;
	wassert(nrpts > 0);
	/* '-1' for remove default 'struct curve.pts[1]' */
	c = wmalloc(crv_sizeof(nrpts));
	wassert(c);
	crv_init(c, nrpts);
	return c;
}

static inline void
crv_destroy(struct curve* crv) {
	wfree(crv);
}

void
crv_dump(const struct curve* crv);

void
crv_list_dump(const struct list_link* hd);

static inline void
crv_copy_line_attr(struct curve* dst, const struct curve* src) {
	dst->color = src->color;
	dst->alpha = src->alpha;
	dst->thick = src->thick;
}

/*
 * @hd : head of 'struct curve' list
 */
static inline void
crv_list_free(struct list_link* hd) {
	struct curve *c, *tmp;
	crv_foreach_removal_safe(c, tmp, hd)
		crv_destroy(c);
	list_init_link(hd);
}

/*
 * @hd : head of 'struct curve' list
 */
static inline int32_t
crv_list_nr_pts(struct list_link* hd) {
	struct curve* c;
	int32_t       cnt = 0;
	crv_foreach(c, hd)
		cnt += c->nrpts;
	return cnt;
}

#if 0
static inline struct curve*
crv_get(struct curve* c) {
	c->ref++;
	return c;
}

static inline void
crv_put(struct curve* c) {
	wassert(c->ref);
	c->ref--;
	if (!c->ref)
		wfree(crv);
}
#endif

static inline void
crv_to_pointnd_list(const struct curve* crv, struct list_link* hd) {
	const struct point *pt, *ptend;
	crv_foreach_point(crv, pt, ptend)
		pointnd_add_last(hd, pt->x, pt->y);
}

static inline void
crv_to_linend_list(const struct curve* crv, struct list_link* hd) {
	const struct point *p0, *p1, *ptend;
	crv_foreach_point2(crv, p0, p1, ptend)
		linend_add_last(hd, p0->x, p0->y, p1->x, p1->y);
}

/*
 * 'crv->nrpts' should be same with 'list_size(hd)'.
 * @hd : list head of 'struct pointnd'
 */
static inline void
crv_copy_points(struct curve* crv, const struct list_link* hd) {
	struct pointnd*  pn;
	struct point*    p = crv->pts;
	wassert(list_size(hd) == crv->nrpts);
	list_foreach_item(pn, hd, struct pointnd, lk)
		*p++ = pn->pt;
}

/*
 * @out : list type < struct linend >
 */
void
crv_find_lines_draw(const struct curve* crv,
		    struct list_link* out,
		    int32_t l, int32_t t, int32_t r, int32_t b);

/*
 * split given curve to several
 * order of each curve is preserved.
 *
 * @in_rect : curves those are inside give rect.
 *            list of 'struct curve'
 * @out_rect: curves those are outside given rect.
 *            list of 'struct curve'
 */
void
crv_split(const struct curve* crv,
	  struct list_link* in_rect,
	  struct list_link* out_rect,
	  int32_t l, int32_t t, int32_t r, int32_t b);

#endif /* _CURVe_h_ */
