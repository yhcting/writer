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
#include "node.h"

struct div {
	struct list_link   lns;  /* lines */
	struct list_link   objs; /* objects */
	struct rect        boundary;
};


/*
 * 'r' and 'b' is open
 */
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

/*
 * @return: node that is used at division.
 */
static inline struct node*
div_add_line(struct div* div, struct line* ln) {
	struct node* n = node_alloc(ln);
	wassert(n);
	ln->div = div;
	ln->divlk = &n->lk;
	list_add_last(&div->lns, &n->lk);
	return n;
}

static inline struct node*
div_add_line_prev(struct node* node, struct line* ln) {
	struct node* n = node_alloc(ln);
	wassert(n && ((struct line*)node->v)->div);
	ln->div = ((struct line*)node->v)->div;
	ln->divlk = &n->lk;
	list_add_prev(&node->lk, &n->lk);
	return n;
}

static inline struct node*
div_add_line_next(struct node* node, struct line* ln) {
	struct node* n = node_alloc(ln);
	wassert(n && ((struct line*)node->v)->div);
	ln->div = ((struct line*)node->v)->div;
	ln->divlk = &n->lk;
	list_add_next(&node->lk, &n->lk);
	return n;
}

static inline struct node*
div_prev_node(struct line* ln) {
	return container_of(ln->divlk->_prev, struct node, lk);
}

static inline struct line*
div_prev_line(struct line* ln) {
	return div_prev_node(ln)->v;
}

#endif /* _DIv_h_ */
