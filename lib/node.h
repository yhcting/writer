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

#ifndef _NODe_h_
#define _NODe_h_

#include "list.h"
#include "common.h"

struct node {
	struct list_link   lk;
	void*              v;  /* value of this node */
};

static inline struct node*
nodelk(const struct list_link* lk) {
	return container_of(lk, struct node, lk);
}

static inline struct node*
node_next(const struct node* n) {
	return container_of(n->lk._next, struct node, lk);
}

static inline struct node*
node_prev(const struct node* n) {
	return container_of(n->lk._prev, struct node, lk);
}

static inline void
node_set_next(struct node* n, struct node* next) {
	n->lk._next = &next->lk;
}

static inline void
node_set_prev(struct node* n, struct node* prev) {
	n->lk._prev = &prev->lk;
}

static inline struct node*
nodelk_next(const struct list_link* lk) {
	return container_of(lk->_next, struct node, lk);
}

static inline struct node*
nodelk_prev(const struct list_link* lk) {
	return container_of(lk->_prev, struct node, lk);
}

static inline struct node*
nodelk_first(const struct list_link* head) {
	return nodelk_next(head);
}

static inline struct node*
nodelk_last(const struct list_link* head) {
	return nodelk_prev(head);
}

static inline void
node_init(struct node* n) {
	list_init_link(&n->lk);
	n->v = NULL;
}

static inline struct node*
node_alloc(void* v) {
	struct node* n;
	n = wmalloc(sizeof(*n));
	n->v = v;
	return n;
}

static inline void*
node_free(struct node* n) {
	void* v = n->v;
	wfree(n);
	return v;
}
/*
 * deep free (node->v is also freed by wfree
 * Assumption: (NULL != n->v) && n->lk is well defined.
 */
static inline void
node_free_deep(struct node* n) {
	wfree(node_free(n));
}

static inline void
node_free_deep2(struct node* n, void(*vfree)(void*)) {
	(vfree)? (*vfree)(node_free(n)): node_free_deep(n);
}


/*****************************************************************************
 *
 * Node List Functions
 *
 *****************************************************************************/
static inline void
nlist_add(struct list_link* head, void* v) {
	list_add_last(head, &node_alloc(v)->lk);
}

static inline void
nlist_del(struct node* n) {
	list_del(&n->lk);
}

static inline void
nlist_free(struct list_link* head) {
	struct node *n, *tmp;
	list_foreach_item_safe(n, tmp, head, struct node, lk)
		/*
		 * to improve performance, nmp_free() is used instead of
		 *   node_free().
		 */
		wfree(n);

	list_init_link(head);
}

static inline void
nlist_free_deep(struct list_link* head) {
	struct node *n, *tmp;
	list_foreach_item_safe(n, tmp, head, struct node, lk) {
		/*
		 * to improve performance node_free_deep() is not used.
		 */
		wfree(n->v);
		wfree(n);
	}
	list_init_link(head);
}

static inline void
nlist_free_deep2(struct list_link* head, void(*vfree)(void*)) {
	struct node *n, *tmp;
	list_foreach_item_safe(n, tmp, head, struct node, lk) {
		/*
		 * to improve performance node_free_deep2() is not used.
		 */
		(vfree)? vfree(n->v): wfree(n->v);
		wfree(n);
	}
	list_init_link(head);
}

static inline void
nlist_add_nlist(struct list_link* head, const struct list_link* in) {
	struct node* n;
	list_foreach_item(n, in, struct node, lk) {
		nlist_add(head, n->v);
	}
}

#define nlist_foreach(n, hd) list_foreach_item(n, hd, struct node, lk)
#define nlist_foreach_safe(n, tmp, hd)				\
	list_foreach_item_safe(n, tmp, hd, struct node, lk)


#endif /* _NODe_h_ */
