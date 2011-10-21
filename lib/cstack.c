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

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "cstack.h"

/* Read Value - Head */
static inline void*
_rvH(const struct cstk* s) {
	return s->s[s->h];
}

/* Read Value - Tail */
static inline void*
_rvT(const struct cstk* s) {
	return s->s[s->t];
}

/* Write Value - Head */
static inline void
_wvH(struct cstk* s, void* v) {
	s->s[s->h] = v;
}

/* Write Value - Tail */
static inline void
_wvT(struct cstk* s, void* v) {
	s->s[s->t] = v;
}

static inline int32_t
_inc(const struct cstk* s, int32_t i) {
	/* s->lim is power of 2. So below code works */
	return ((i + 1) & (s->lim - 1));
}

static inline int32_t
_dec(const struct cstk* s, int32_t i) {
	/* s->lim is power of 2. So below code works */
	return ((i - 1) & (s->lim - 1));
}

static inline void
_incH(struct cstk* s) {
	s->h = _inc(s, s->h);
}

static inline void
_incT(struct cstk* s) {
	s->t = _inc(s, s->t);
}

static inline void
_decH(struct cstk* s) {
	s->h = _dec(s, s->h);
}

static inline void
_decT(struct cstk* s) {
	s->t = _dec(s, s->t);
}

static inline void
_vfree(struct cstk* s, void* v) {
	s->fcb? (*s->fcb)(v): wfree(v);
}

struct cstk*
cstk_create(int32_t bitsz, void(*fcb)(void*)) {
	struct cstk* s = wmalloc(sizeof *s);
	wassert(s && bitsz > 0 && bitsz < 30);
	s->h = s->t = 0;
	s->lim = 1 << bitsz;
	s->sz  = 0;
	s->fcb = fcb;
	s->s   = wmalloc(sizeof(void*) * s->lim);
	memset(s->s, 0, sizeof(void*) * s->lim);
	wassert(s->s);
	return s;
}

void
cstk_reset(struct cstk* s) {
	while (!cstk_is_empty(s))
		_vfree(s, cstk_pop(s));
	s->h = s->t = 0;
	/* memset(s->s, 0, sizeof(void*) * s->lim); */
}

void
cstk_destroy(struct cstk* s) {
	cstk_reset(s);
	wfree(s->s);
	wfree(s);
}

void
cstk_push(struct cstk* s, void* v) {
	wassert(s && v);

	if (cstk_is_full(s)) {
		/* remove oldest(head) */
		_vfree(s, _rvH(s));
		_incH(s);
		s->sz--;
	}
	_wvT(s, v);
	_incT(s);
	s->sz++;
}

void*
cstk_pop(struct cstk* s) {
	void* v;
	wassert(s);

	if (cstk_is_empty(s))
		return NULL;

	/* read newest */
	_decT(s);
	v = _rvT(s);
	_wvT(s, NULL);
	s->sz--;
	return v;
}

#ifdef CONFIG_TEST_EXECUTABLE
void
cstk_dump(struct cstk* s) {
	int i;
	printf("===== Stack dump =====\n"
	       "head : %d\n"
	       "tail : %d\n"
	       "sz   : %d\n"
	       "limit: %d\n"
	       "fcb  : %p\n"
	       "-----------\n",
	       s->h,
	       s->t,
	       s->sz,
	       s->lim,
	       s->fcb);
	for (i = 0; i < s->lim; i++)
		printf("  %p\n",s->s[i]);
	printf("===== Dump end =====\n");
}
#endif /* CONFIG_TEST_EXECUTABLE */
