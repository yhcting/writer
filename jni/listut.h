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

#ifndef _LISTUTIl_h_
#define _LISTUTIl_h_

#include "common.h"

#include "gtype.h"
#include "list.h"
#include "nmp.h"


static inline void
nlist_clean(struct list_link* head)
	__attribute__((unused));
static inline void
nlist_clean(struct list_link* head) {
	struct node* n;
	struct node* tmp;
	list_foreach_item_removal_safe(n, tmp, head, struct node, lk) {
		nmp_free(n);
	}
	list_init_link(head);
}

static inline struct node*
nlist_alloc_node(void* v)
	__attribute__((unused));
static inline struct node*
nlist_alloc_node(void* v) {
	struct node* n;
	n = nmp_alloc();
	n->v = v;
	return n;
}

static inline void
nlist_add(struct list_link* head, void* v)
	__attribute__((unused));
static inline void
nlist_add(struct list_link* head, void* v) {
	struct node* n = nlist_alloc_node(v);
	list_add_last(head, &n->lk);
}

static inline void
nlist_del(struct node* n)
	__attribute__((unused));
static inline void
nlist_del(struct node* n) {
	list_del(&n->lk);
	nmp_free(n);
}

static inline void
nlist_add_nlist(struct list_link* head, const struct list_link* in)
	__attribute__((unused));
static inline void
nlist_add_nlist(struct list_link* head, const struct list_link* in) {
	struct node* n;
	list_foreach_item(n, in, struct node, lk) {
		nlist_add(head, n->v);
	}
}

#endif /* _LISTUTIl_h_ */
