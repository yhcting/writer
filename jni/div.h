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
#include "g2d.h"
#include "nmp.h"
#include "listut.h"

void
div_find_lines(const struct div* div,
	       struct list_link* out,
	       int32_t l, int32_t t, int32_t r, int32_t b);

void
div_clean(struct div* div);

static inline void
div_add_obj(struct div* div, struct obj* o) {
	nlist_add(&div->objs, o);
	o->ref++;
}

void
div_del_obj(struct div* div, struct obj* o);

static inline void
div_init(struct div* div,
	 int32_t l, int32_t t, int32_t r, int32_t b) {
	list_init_link(&div->lns);
	list_init_link(&div->objs);
	rect_set(&div->boundary, l, t, r, b);
}

static inline void
div_add_line(struct div* div, struct line* ln) {
	ln->div = div;
	nlist_add(&div->lns, ln);
	ln->divlk = div->lns._prev;
}

#endif /* _DIv_h_ */
