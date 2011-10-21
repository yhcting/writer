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

#include "common.h"
#include "curve.h"

void
crv_dump(const struct curve* crv) {
	const struct point *p, *pe;
	int                 i;
	wlogd("=== Curve [ %p ] ===\n"
	      "    ref:    %u\n"
	      "    color:  %x\n"
	      "    alpha:  %u\n"
	      "    thick:  %u\n"
	      "    nrpts:  %u\n",
	      crv,
	      crv->ref,
	      crv->color,
	      crv->alpha,
	      crv->thick,
	      crv->nrpts);
	p = crv->pts;
	pe = p + crv->nrpts;
	i = 0;
	while (p < pe) {
		i++;
		if (!(i % 5))
			wlogd("\n");
		if (1 == (i % 5))
			wlogd("    ");
		wlogd("(%d, %d) ", p->x, p->y);
		p++;
	}
	wlogd("\n");
}

void
crv_list_dump(const struct list_link* hd) {
	const struct curve* crv;
	int                 i = 0;
	wlogd("========== Curve List ===========\n");
	crv_foreach(crv, hd) {
		wlogd("[ %d ] : ", i);
		crv_dump(crv);
		i++;
	}
}


void
crv_find_lines_draw(const struct curve* crv,
		    struct list_link* out,
		    int32_t l, int32_t t, int32_t r, int32_t b) {
	const struct point *p0, *p1, *ptend;
	crv_foreach_point2(crv, p0, p1, ptend) {
		if (rect_is_overlap_line2(l, t, r, b, p0, p1))
			linend_add_last2(out, p0, p1);
	}
}

void
crv_split(const struct curve* crv,
	  struct list_link* in,
	  struct list_link* out,
	  int32_t l, int32_t t, int32_t r, int32_t b) {
	const struct point *p0, *p1, *pend;
	struct point        i0, i1;
	struct rect         rect;
	struct list_link*   hd; /* head to add */
	struct list_link    ptns;

	struct list_link*
	__switch_hd(const struct list_link* hd) {
		return ((hd) == in)? out: in;
	}

	struct list_link*
	__get_hd(const struct point* p) {
		return (rect_is_in3(&rect, p))? in: out;
	}
	void
	__add_sub_curve(struct list_link* hd, struct list_link* pts) {
		struct curve* c;
		unsigned int  nrpts = list_size(pts);
		if (nrpts <= 0)
			return; /* nothing to do */
		wassert(nrpts > 1);
		c = crv_create((uint16_t)nrpts);
		crv_copy_line_attr(c, crv);
		crv_copy_points(c, pts);
		list_add_last(hd, &c->lk);
		pointnd_free_list(pts);
	}

	wassert(crv->nrpts > 1);
	rect_set(&rect, l, t, r, b);
	list_init_link(&ptns);
	hd = __get_hd(&crv->pts[0]);
	/* add first point */
	pointnd_add_last2(&ptns, &crv->pts[0]);
	crv_foreach_point2(crv, p0, p1, pend) {
		struct list_link* hdsv;
		/*
		 * bp : base point
		 * ip : intersect point
		 */
#define __add_intersect_point(bp, ip)			\
		if (!point_is_equal(bp, ip)) {		\
			pointnd_add_last2(&ptns, ip);	\
			__add_sub_curve(hd, &ptns);	\
			hd = __switch_hd(hd);		\
			pointnd_add_last2(&ptns, ip);	\
		}

		switch(rect_intersect_line4(&i0, &i1, &rect, p0, p1)) {

		case 1: {
			__add_intersect_point(p0, &i0);
		} break;

			/* case 2 and case 3 are handled with same way */
		case 2:
		case 3: {
			__add_intersect_point(p0, &i0);
			wassert(in == hd);
			__add_intersect_point(p1, &i1);
		} break;

		case 0: {
			; /* nothing to do */
		} break;

		default:
			wassert(0); /* SHOULDN'T reach here! */
		}

		pointnd_add_last2(&ptns, p1);
		hdsv = hd;
		hd = __get_hd(p1);
		if (hd != hdsv)
			__add_sub_curve(hdsv, &ptns);

#undef __add_intersect_point

	}
	__add_sub_curve(hd, &ptns);
}

