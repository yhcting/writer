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

#ifndef __LISt_h__
#define __LISt_h__

#define list_decl_head(hd) struct list_link hd = {&hd, &hd}

/**
 * If possible DO NOT access struct directly!.
 */
struct list_link {
	struct list_link *_next, *_prev;
};

/**
 * initialize list head.
 */
static inline void
list_init_link(struct list_link* link) {
	link->_next = link->_prev = link;
}

static inline int
list_is_empty(const struct list_link* head) {
	return head->_next == head;
}

static inline void
list_add(struct list_link* prev,
	   struct list_link* next,
	   struct list_link* anew) {
	next->_prev = prev->_next = anew;
	anew->_next = next; anew->_prev = prev;
}

static inline void
list_add_next(struct list_link* link, struct list_link* anew) {
	list_add(link, link->_next, anew);
}

static inline void
list_add_prev(struct list_link* link, struct list_link* anew) {
	list_add(link->_prev, link, anew);
}

static inline void
list_add_first(struct list_link* head, struct list_link* anew) {
	list_add_next(head, anew);
}

static inline void
list_add_last(struct list_link* head, struct list_link* anew) {
	list_add_prev(head, anew);
}

static inline void
__list_del(struct list_link* prev, struct list_link* next) {
	prev->_next = next;
	next->_prev = prev;
}

static inline void
list_del(struct list_link* link) {
	__list_del(link->_prev, link->_next);
}

static inline void
list_replace(struct list_link* old, struct list_link* anew) {
	anew->_next = old->_next;
	anew->_next->_prev = anew;
	anew->_prev = old->_prev;
	anew->_prev->_next = anew;
}

static inline void
list_absorb(struct list_link* head, struct list_link* in) {
	/* absorb 'in' */

	/* if list size > 0 */
	if (in->_next != in) {
		head->_prev->_next = in->_next;
		in->_next->_prev = head->_prev;
		head->_prev = in->_prev;
		head->_prev->_next = head;
	}
	/* make absorbed list be empty */
	list_init_link(in);
}

static inline void
list_link(struct list_link* prev, struct list_link* next) {
	prev->_next = next;
	next->_prev = prev;
}

/**
 * @pos     : the &struct list_link to use as a loop cursor
 * @head    : head of list (&struct list_link)
 */
#define list_foreach(pos, head)					\
        for ((pos) = (head)->_next; (pos) != (head); (pos) = (pos)->_next)

#define list_foreach_backward(pos, head)				\
        for ((pos) = (head)->_prev; (pos) != (head); (pos) = (pos)->_prev)

/**
 * @pos     : the &struct list_link to use as a loop cursor
 * @n       : another &struct list_link to use as temporary storage
 * @head    : head of list (&struct list_link)
 */
#define list_foreach_removal_safe(pos, n, head)	\
        for ((pos) = (head), (n) = (pos)->_next;	\
	     (pos) != (head);				\
	     (pos) = (n), (n) = (pos)->_next)

#define list_foreach_removal_safe_backward(pos, n, head)	\
        for ((pos) = (head), (n) = (pos)->_prev;		\
	     (pos) != (head);					\
	     (pos) = (n), (n) = (pos)->_prev)
/**
 * @pos     : the @type* to use as a loop cursor.
 * @head    : the head for list (&struct list_link)
 * @type    : the type of the struct of *@pos
 * @member  : the name of the list_link within the struct.
 */
#define list_foreach_item(pos, head, type, member)			\
        for ((pos) = container_of((head)->_next, type, member);		\
	     &(pos)->member != (head);					\
	     (pos) = container_of((pos)->member._next, type, member))

#define list_foreach_item_backward(pos, head, type, member)		\
        for ((pos) = container_of((head)->_prev, type, member);		\
	     &(pos)->member != (head);					\
	     (pos) = container_of((pos)->member._prev, type, member))

/**
 * @type    : the type of the struct of *@pos
 * @pos     : the @type* to use as a loop cursor.
 * @n       : another @type* to use as temporary storage.
 * @head    : the head for list (&struct list_link)
 * @member  : the name of the list_link within the struct.
 */
#define list_foreach_item_removal_safe(pos, n, head, type, member)	\
        for ((pos) = container_of((head)->_next, type, member),		\
		     (n) = container_of((pos)->member._next, type, member); \
	     &(pos)->member != (head);					\
	     (pos) = (n),						\
		     (n) = container_of((pos)->member._next, type, member))

#define list_foreach_item_removal_safe_backward(pos, n, head, type, member) \
        for ((pos) = container_of((head)->_prev, type, member),		\
		     (n) = container_of((pos)->member._prev, type, member); \
	     &(pos)->member != (head);					\
	     (pos) = (n),						\
		     (n) = container_of((pos)->member._prev, type, member))

static inline unsigned int
list_size(const struct list_link* head) {
	struct list_link*   pos;
	unsigned int size = 0;
	list_foreach(pos, head)
		size++;
	return size;
}

#endif /* __LISt_h__ */
