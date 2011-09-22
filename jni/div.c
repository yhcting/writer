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
#include "listut.h"
#include "div.h"

/********************************
 * Functions for div
 ********************************/
static void
_del_line_node(struct div* div, struct node* n) {
	wassert(&n->lk != &div->objs);
	if (&n->lk == &div->objs)
		return;

	wfree(n->v);
	nlist_del(n);
}

static void
_del_obj_node(struct div* div, struct node* n) {
	struct obj* o = n->v;
	wassert(&n->lk != &div->objs);
	if (&n->lk == &div->objs)
		return;

	nlist_del(n);
	o->ref--;

	/*
	 * object is not refered anymore.
	 * free it!
	 */
	if (!o->ref)
		wfree(o);
}


void
div_clean(struct div* div) {
	struct node *n, *tmp;
	/* clean lines */
	list_foreach_item_removal_safe(n, tmp,  &div->lns, struct node, lk)
		_del_line_node(div, n);

	list_foreach_item_removal_safe(n, tmp, &div->objs, struct node, lk)
		_del_obj_node(div, n);
}

void
div_find_lines(const struct div* div,
	       struct list_link* out,
	       int32_t l, int32_t t, int32_t r, int32_t b) {
	struct node*   n;
	struct line*   ln;
	struct rect    rect;
	bool           b_intersected;

	rect_set(&rect, l, t, r, b);

	list_foreach_item(n, &div->lns, struct node, lk) {
		ln = n->v;
		b_intersected = false;
		/*
		 * Check that line is expands on this rectangle region.
		 */

		/*
		 * Note:
		 *   ln->x1 and ln->y1 is open set.
		 */
		if ((ln->x0 < l && ln->x1 <= l)
		    || (ln->x0 >= r && ln->x1 >= r)
		    || (ln->y0 < t && ln->y1 <= t)
		    || (ln->y0 >= b && ln->y1 >= b))
			/* trivially not intersected */
			;
		else if (rect_is_contains(&rect, ln->x0, ln->y0)
		    /* check for open point */
		    || (ln->x1 > l && ln->x1 < (r - 1)
			&& ln->y1 > t && ln->y1 < (b - 1 ))) {
			/* trivial case! */
			b_intersected = true;
		} else {
			int32_t intersect;;
			if (intersectX(&intersect, ln, l, t, b)
			    || intersectX(&intersect, ln, r, t, b)
			    || intersectY(&intersect, ln, t, l, r)
			    || intersectY(&intersect, ln, b, l, r)) {
				b_intersected = true;
			}
		}
		if (b_intersected)
			nlist_add(out, ln);
	}
}

void
div_find_objs(const struct div* div,
	      struct list_link* out,
	      int32_t l, int32_t t, int32_t r, int32_t b) {
	struct node*   n;
	struct obj*    o;
	struct rect    rect;

	rect_set(&rect, l, t, r, b);

	list_foreach_item(n, &div->objs, struct node, lk) {
		o = n->v;
		if (rect_is_overwrapped(&o->extent, &rect))
			nlist_add(out, o);
	}
}

void
div_del_obj(struct div* div, struct obj* o) {
	struct node* n;
	list_foreach_item(n, &div->objs, struct node, lk)
		if (n->v == o)
			break;

	_del_obj_node(div, n);
}
