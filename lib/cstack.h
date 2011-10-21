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

/*
 * Cyclic Stack!
 */

#ifndef _CSTACk_h_
#define _CSTACk_h_

#include <stdint.h>
#include <stdbool.h>

#include "common.h"

struct cstk {
	int32_t h,    /* head index - oldest slot */
		t,    /* tail index - next empty (newest) */
		lim,  /* limit size of stack array */
		sz;   /* size of stack - nr of item */
	void**  s;    /* actual stack */
	void  (*fcb)(void*); /* call for free item */
};

/*
 * @bitsz : # bits to use as an size. So, 2^bitsz is real stack size.
 */
struct cstk*
cstk_create(int32_t bitsz, void(*fcb)(void*));

/*
 * clean stack items and reset stack state.
 */
void
cstk_reset(struct cstk*);

void
cstk_destroy(struct cstk*);

void
cstk_push(struct cstk*, void* v);

void*
cstk_pop(struct cstk*);

static inline int32_t
cstk_sz(struct cstk* s) {
	return s->sz;
}

static inline bool
cstk_is_empty(const struct cstk* s) {
	return ((s->h == s->t) && !s->s[s->h]);
}

static inline bool
cstk_is_full(const struct cstk* s) {
	return ((s->h == s->t) && s->s[s->h]);
}

#ifdef CONFIG_TEST_EXECUTABLE
void
cstk_dump(struct cstk*);
#else /* CONFIG_TEST_EXECUTABLE */
static inline void
cstk_dump(struct cstk* c){}
#endif /* CONFIG_TEST_EXECUTABLE */


#endif /* _CSTACk_h_ */
