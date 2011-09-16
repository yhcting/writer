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

#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "list.h"

#define _MAX_HBITS 32
#define _MIN_HBITS 4

/* hash node */
struct _hn {
	struct list_link      lk;
	uintptr_t	      a;  /* address */
};

struct ahash {
	struct list_link*     map;
	uint32_t	      sz;	   /* hash size */
	uint8_t	              mapbits;	   /* bits of map table = 1<<mapbits */
};

static inline uint32_t
_hmapsz(const struct ahash* h) {
	return (1 << h->mapbits);
}

static inline uint32_t
_hv__(uint32_t mapbits, uintptr_t hv) {
	return hv >> (sizeof(uintptr_t) * 8 - mapbits);
}

static inline uint32_t
_hv_(const struct ahash* h, uintptr_t hv) {
	return _hv__(h->mapbits, hv);
}

static inline uint32_t
_hv(const struct ahash* h, const struct _hn* n) {
	return _hv_(h, n->a);
}

/* Modify hash space to 'bits'*/
static struct ahash*
_hmodify(struct ahash* h, uint32_t bits) {
	uint32_t            i;
	struct _hn         *n, *tmp;
	struct list_link*   oldmap;
	uint32_t            oldmapsz;

	if (bits < _MIN_HBITS)
		bits = _MIN_HBITS;
	if (bits > _MAX_HBITS)
		bits = _MAX_HBITS;

	if (h->mapbits == bits)
		return h; /* nothing to do */

	oldmap = h->map;
	oldmapsz = _hmapsz(h);

	h->mapbits = bits; /* map size is changed here */
	h->map = wmalloc(sizeof(struct list_link) * _hmapsz(h));
	wassert(h->map);
	for (i=0; i<_hmapsz(h); i++)
		list_init_link(&h->map[i]);
	/* re assign hash nodes */
	for (i=0; i<oldmapsz; i++) {
		list_foreach_item_removal_safe(n,
					       tmp,
					       &oldmap[i],
					       struct _hn,
					       lk) {
			list_del(&n->lk);
			list_add_last(&h->map[_hv(h, n)], &n->lk);
		}
	}
	wfree(oldmap);
	return h;
}

static struct _hn*
_hfind(const struct ahash* h, uintptr_t a) {
	struct _hn*	     n;
	struct list_link*    hd = &h->map[_hv_(h, a)];

	list_foreach_item(n, hd, struct _hn, lk)
		if (n->a == a)
			break;

	return (&n->lk == hd)? NULL: n;
}

static struct _hn*
_ncreate(uintptr_t a) {
	struct _hn* n = wmalloc(sizeof(*n));
	wassert(n);
	n->a = a;
	list_init_link(&n->lk);
	return n;
}

static inline void
_ndestroy(const struct ahash* h, struct _hn* n) {
	wfree(n);
}

struct ahash*
ahash_create(void) {
	uint32_t       i;
	struct ahash*  h = wmalloc(sizeof(*h));
	wassert(h);
	h->sz = 0;
	h->mapbits = _MIN_HBITS;
	h->map = (struct list_link*)wmalloc(sizeof(struct list_link)
					    * _hmapsz(h));
	wassert(h->map);
	for (i=0; i<_hmapsz(h); i++)
		list_init_link(&h->map[i]);
	return h;
}

void
ahash_destroy(struct ahash* h) {
	uint32_t     i;
	struct _hn  *n, *tmp;
	for (i=0; i<_hmapsz(h); i++) {
		list_foreach_item_removal_safe(n,
					       tmp,
					       &h->map[i],
					       struct _hn,
					       lk) {
			list_del(&n->lk);
			_ndestroy(h, n);
		}
	}
	wfree(h->map);
	wfree(h);
}

uint32_t
ahash_sz(const struct ahash* h) {
	return h->sz;
}

struct ahash*
ahash_add(struct ahash* h, void* p) {
	uintptr_t   a = (uintptr_t)p;
	struct _hn* n = _hfind(h, a);

	if (n)
		/* nothing to do */
		return h;
	else {
		/* we need to expand hash map size if hash seems to be full */
		if (h->sz > _hmapsz(h))
			_hmodify(h, h->mapbits+1);
		n = _ncreate(a);
		list_add_last(&h->map[_hv(h, n)], &n->lk);
		h->sz++;
	}

	return h;
}

struct ahash*
ahash_del(struct ahash* h, void* p) {
	uintptr_t   a = (uintptr_t)p;
	struct _hn* n = _hfind(h, a);
	if (n) {
		list_del(&n->lk);
		_ndestroy(h, n);
		h->sz--;
		if (h->sz < _hmapsz(h) / 4)
			_hmodify(h, h->mapbits-1);
	}

	return h;
}

bool
ahash_check(const struct ahash* h, void* p) {
	struct _hn* n = _hfind(h, (uintptr_t)p);
	return n? true: false;
}
