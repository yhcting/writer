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

#include <malloc.h>

#include "common.h"
#include "gtype.h"
#include "g2d.h"
#include "list.h"
#include "div.h"

/********************************
 * Functions for div
 ********************************/

void
div_clean(struct div* div) {
	struct curve *n, *tmp;
	/* clean lines */
	list_foreach_item_safe(n, tmp, &div->crvs, struct curve, lk)
		crv_destroy(n);

	list_init_link(&div->crvs);

#if 0
	list_foreach_item_safe(n, tmp, &div->objs, struct curve, lk)
		_del_obj_node_deep(div, n);

	list_init_link(&div->objs);
#endif
}

/*
 * @out : list type < struct linend >
 */
void
div_find_lines_draw(const struct div* div,
		    struct list_link* out,
		    int32_t l, int32_t t, int32_t r, int32_t b) {
	struct lines_draw* ld;
	struct curve*      crv;
	crv_foreach(crv, &div->crvs) {
		ld = lines_draw_create(crv->color, crv->thick);
		crv_find_lines_draw(crv, &ld->lns, l, t, r, b);
		list_add_last(out, &ld->lk);
	}
}

void
div_get_lines_draw(struct div* div, struct list_link* hd) {
	struct lines_draw*  ld;
	struct curve*       crv;
	const struct point *p0, *p1, *ptend;
	crv_foreach(crv, &div->crvs) {
		ld = lines_draw_create(crv->color, crv->thick);

		crv_foreach_point2(crv, p0, p1, ptend)
			linend_add_last2(&ld->lns, p0, p1);

		list_add_last(hd, &ld->lk);
	}
}

void
div_cutout(struct div* div,
	   int32_t l, int32_t t, int32_t r, int32_t b) {
	struct curve       *crv, *tmp;
	struct list_link    in;
	struct list_link    out;

	list_init_link(&in);
	list_init_link(&out);

	crv_foreach_safe(crv, tmp, &div->crvs) {
		crv_split(crv, &in, &out, l, t, r, b);
		/*
		 * replace 'crv' with out-curve-list in the list.
		 */
		list_link(&crv_prev(crv)->lk, list_first(&out));
		list_link(list_last(&out),    &crv_next(crv)->lk);
		crv_destroy(crv);
		crv_list_free(&in);
		list_init_link(&out);
	}
}

#if 0
void
div_del_obj(struct div* div, struct obj* o) {
	struct node* n;
	list_foreach_item(n, &div->objs, struct node, lk)
		if (n->v == o)
			break;

	_del_obj_node_deep(div, n);
}
#endif
