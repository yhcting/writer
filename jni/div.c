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

/********************************
 * Functions for div
 ********************************/
void
div_find_lines(struct div* div,
	       struct list_link* out,
	       int l, int t, int r, int b) {
	struct node*   n;
	struct line*   ln;
	struct rect    rect;
	char	       b_intersected;

	rect_set(&rect, l, t, r, b);

	list_foreach_item(n, &div->lns, struct node, lk) {
		ln = n->ln;
		b_intersected = 0;
		/* Check that line is expands on this rectangle region. */
		if (rect_contains(&rect, ln->x0, ln->y0)
		     || rect_contains(&rect, ln->x1, ln->y1)) {
			/* trivial case! */
			b_intersected = 1;
		} else {
			int intersect;;
			if (splitX(&intersect, ln, l, t, b)
			    || splitX(&intersect, ln, r, t, b)
			    || splitY(&intersect, ln, t, l, r)
			    || splitY(&intersect, ln, b, l, r)) {
				b_intersected = 1;
			}
		}
		if (b_intersected)
			wlist_add_line(out, ln);
	}
}
