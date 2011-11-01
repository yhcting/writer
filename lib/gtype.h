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

/*
 * Generic types
 */

#ifndef _GTYPe_h_
#define _GTYPe_h_

#include <stdint.h>

#include "common.h"
#include "list.h"
#include "g2d.h"

/*
 * point NoDe
 */
struct pointnd {
	struct list_link lk;
	struct point     pt;
};

/*
 * line NoDe
 */
struct linend {
	struct list_link lk;
	struct line      ln;
};

/*
 * data to draw lines
 */
struct lines_draw {
	struct list_link lk;  /* lines_draw link */
	struct list_link lns; /* head of 'struct linend' */
	uint16_t         color;
	uint8_t          thick;
};

/*
 * all objects except for line segment.
 */
struct obj {
	struct list_link    lk;
	struct rect         extent;
	uint16_t            ty;   /* object type */
	int16_t             ref;  /* reference count */
	void*               priv; /* object private data */
};




/********************************
 * Functions for point list node
 ********************************/

#define pointnd_foreach(ptnd, hd)			\
	list_foreach_item(ptnd, hd, struct pointnd, lk)

#define pointnd_foreach_safe(ptnd, tmp, hd)				\
	list_foreach_item_safe(ptnd, tmp, hd, struct pointnd, lk)

static inline struct pointnd*
pointnd_create(int32_t x, int32_t y) {
	struct pointnd* n = wmalloc(sizeof *n);
	wassert(n);
	point_set(&n->pt, x, y);
	return n;
}

static inline void
pointnd_destroy(struct pointnd* p) {
	wfree(p);
}

static inline struct pointnd*
pointndlk(const struct list_link* lk) {
	return container_of(lk, struct pointnd, lk);
}

static inline void
pointnd_add_last(struct list_link* hd, int32_t x, int32_t y) {
	struct pointnd* ptn = pointnd_create(x, y);
	list_add_last(hd, &ptn->lk);
}

static inline void
pointnd_add_last2(struct list_link* hd, const struct point* p) {
	struct pointnd* ptn = pointnd_create(p->x, p->y);
	list_add_last(hd, &ptn->lk);
}


static inline void
pointnd_free_list(struct list_link* hd) {
	struct pointnd *p, *tmp;
	pointnd_foreach_safe(p, tmp, hd)
		pointnd_destroy(p);
	list_init_link(hd);
}

/********************************
 * Functions for line list node
 ********************************/

#define linend_foreach(lnd, hd)				\
	list_foreach_item(lnd, hd, struct linend, lk)

#define linend_foreach_safe(lnd, tmp, hd)			\
	list_foreach_item_safe(lnd, tmp, hd, struct linend, lk)

static inline struct linend*
linend_create(int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	struct linend* n = wmalloc(sizeof *n);
	wassert(n);
	line_set(&n->ln, x0, y0, x1, y1);
	return n;
}

static inline void
linend_destroy(struct linend* l) {
	wfree(l);
}

static inline void
linend_dump(const struct linend* l) {
	wlogd("(%d, %d) (%d, %d)",
	      l->ln.p0.x, l->ln.p0.y,
	      l->ln.p1.x, l->ln.p1.y);
}

static inline void
linend_list_dump(const struct list_link* hd) {
	const struct linend* l;
	wlogd("========= Linend list [ %p ] : %d =========\n",
	      hd, list_size(hd));
	linend_foreach(l, hd)
		linend_dump(l);
}

static inline struct linend*
linendlk(const struct list_link* lk) {
	return container_of(lk, struct linend, lk);
}

static inline void
linend_add_last(struct list_link* hd,
		int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	struct linend* ln = linend_create(x0, y0, x1, y1);
	list_add_last(hd, &ln->lk);
}

static inline void
linend_add_last2(struct list_link* hd,
		 const struct point* p0, const struct point* p1) {
	linend_add_last(hd, p0->x, p0->y, p1->x, p1->y);
}

static inline void
linend_free_list(struct list_link* hd) {
	struct linend *p, *tmp;
	linend_foreach_safe(p, tmp, hd)
		linend_destroy(p);
	list_init_link(hd);
}

/********************************
 * Functions for lines to draw
 ********************************/

#define lines_draw_foreach(ld, hd)				\
	list_foreach_item(ld, hd, struct lines_draw, lk)

#define lines_draw_foreach_safe(ld, tmp, hd)				\
	list_foreach_item_safe(ld, tmp, hd, struct lines_draw, lk)

static inline struct lines_draw*
lines_draw_create(uint16_t color, uint8_t thick) {
	struct lines_draw* ld = wmalloc(sizeof *ld);
	ld->color = color;
	ld->thick = thick;
	list_init_link(&ld->lns);
	return ld;
}

static inline void
lines_draw_destroy(struct lines_draw* ld) {
	wfree(ld);
}

static inline void
lines_draw_dump(const struct lines_draw* l) {
	wlogd("** color:%x, thick:%d **\n", l->color, l->thick);
	linend_list_dump(&l->lns);
}

static inline void
lines_draw_list_dump(const struct list_link* hd) {
	const struct lines_draw* l;
	wlogd("========= Lines_draw list [ %p ] : %d =========\n",
	      hd, list_size(hd));
	lines_draw_foreach(l, hd)
		lines_draw_dump(l);
}

static inline void
lines_draw_destroy_deep(struct lines_draw* ld) {
	linend_free_list(&ld->lns);
	wfree(ld);
}

static inline void
lines_draw_free_list(struct list_link* hd) {
	struct lines_draw *p, *tmp;
	lines_draw_foreach_safe(p, tmp, hd)
		lines_draw_destroy(p);
	list_init_link(hd);
}

static inline void
lines_draw_free_list_deep(struct list_link* hd) {
	struct lines_draw *p, *tmp;
	lines_draw_foreach_safe(p, tmp, hd)
		lines_draw_destroy_deep(p);
	list_init_link(hd);
}

#endif /* _GTYPe_h_ */
