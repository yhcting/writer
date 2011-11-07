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

#ifndef _DIv_h_
#define _DIv_h_

#include <stdint.h>
#include <stdbool.h>

#include "common.h"

#include "gtype.h"
#include "g2d.h"
#include "list.h"
#include "curve.h"

/*
 * division node
 */

struct div {
	/* curves - list of 'struct curve' */
	struct list_link   crvs;
	struct list_link   objs; /* objects */
	struct rect        boundary;
};


void
div_clean(struct div* div);

static inline void
div_init(struct div* div,
	 int32_t l, int32_t t, int32_t r, int32_t b) {
	list_init_link(&div->crvs);
	list_init_link(&div->objs);
	rect_set(&div->boundary, l, t, r, b);
}

static inline int32_t
div_nr_curve(const struct div* div) {
	return list_size(&div->crvs);
}

static inline int32_t
div_nr_lines(const struct div* div) {
	int32_t       cnt = 0;
	struct curve* crv;
	crv_foreach(crv, &div->crvs)
		cnt += crv->nrpts - 1;
	return cnt;
}

static inline int32_t
div_nr_points(const struct div* div) {
	int32_t       cnt = 0;
	struct curve* crv;
	crv_foreach(crv, &div->crvs)
		cnt += crv->nrpts;
	return cnt;
}

static inline void
div_to_pointnd_list(struct div* div, struct list_link* hd) {
	struct curve* crv;
	crv_foreach(crv, &div->crvs)
		crv_to_pointnd_list(crv, hd);
}

static inline void
div_add_curve(struct div* div, struct curve* crv) {
	list_add_last(&div->crvs, &crv->lk);
}

/*
 * @out : list type < struct lines_draw >
 */
void
div_find_lines_draw(const struct div* div,
		       struct list_link* out,
		       int32_t l, int32_t t, int32_t r, int32_t b);

void
div_cutout(struct div* div,
	   /* list of node which value is 'struct curve' */
	   struct list_link* lrm, /* curves removed */
	   /* list of node which value is 'struct curve' */
	   struct list_link* ladd, /* curves newly added */
	   int32_t l, int32_t t, int32_t r, int32_t b);

void
div_get_lines_draw(struct div* div, struct list_link* hd);

void
div_dump(struct div* div);

#endif /* _DIv_h_ */
