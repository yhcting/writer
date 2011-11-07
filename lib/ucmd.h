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

#ifndef _UCMd_h_
#define _UCMd_h_

#include "gtype.h"
#include "list.h"
#include "node.h"

struct wsheet;

/*
 * user command
 */
enum ucmd_ty {
	/*
	 * This value may be used as array index.
	 * So, starting from 0
	 */
	UCMD_CURVE = 0,
	UCMD_CUT,
	UCMD_ZOOM,
	UCMD_MOVE,
};

enum ucmd_st { /* state of ucmd */
	UCMD_ST_INIT,
	UCMD_ST_READY,    /* ready to use */
	UCMD_ST_PROGRESS, /* between 'start' and 'end' */
	UCMD_ST_DONE,     /* command is applied */
	UCMD_ST_UNDONE,   /* 'undo' is executed */
	UCMD_ST_UNKNOWN
};

/* ucmdd : ucmd Data */
struct ucmdd_crv {
	/* node link of 'struct curve*' */
	struct list_link pcrvl; /* Pointer CuRVe List */
	/* see ucmd.c for details */
	struct list_link links; /* link info. of curve. - for UNDO/REDO */
};

struct ucmdd_cut {
	/* list of node which value is 'struct curve' */
	struct list_link lrm;  /* curves removed */
	/* list of node which value is 'struct curve' */
	struct list_link ladd; /* curves newly added */
};

struct ucmd {
	enum ucmd_ty     ty;
	enum ucmd_st     state;

	struct wsheet*   wsh; /* sheet which command is exectued on */

	/* DO NOT call below functions directly */
	int  (*__alloc) (struct ucmd*);
	void (*__free)  (struct ucmd*);

	/* Below two functions are only for internal use! */
	void (*___free_done)  (struct ucmd*);
	void (*___free_undone)(struct ucmd*);

	int  (*__start) (struct ucmd*);
	int  (*__end)   (struct ucmd*);
	int  (*__undo)  (struct ucmd*);
	int  (*__redo)  (struct ucmd*);
	/* notify user command data */
	void (*__notify)(struct ucmd*, void*, void*);
	union {
		struct ucmdd_crv  crv;
		struct ucmdd_cut  cut;
	} d;
};

struct ucmd*
ucmd_create(enum ucmd_ty, struct wsheet*);

static inline int
ucmd_alloc(struct ucmd* uc) {
	int r;
	wassert(UCMD_ST_INIT == uc->state);
	r = uc->__alloc(uc);
	if (!(r < 0))
		uc->state = UCMD_ST_READY;
	return r;
}

static inline void
ucmd_free(struct ucmd* uc) {
	wassert(UCMD_ST_READY == uc->state
		|| UCMD_ST_DONE == uc->state
		|| UCMD_ST_UNDONE == uc->state);
	uc->__free(uc);
	wfree(uc);
}

static inline int
ucmd_start(struct ucmd* uc) {
	int r;
	wassert(UCMD_ST_READY == uc->state);
	r = uc->__start(uc);
	if (!(r < 0))
		uc->state = UCMD_ST_PROGRESS;
	return r;
}

static inline int
ucmd_end(struct ucmd* uc) {
	int r;
	wassert(UCMD_ST_PROGRESS == uc->state);
	r = uc->__end(uc);
	if (!(r < 0))
		uc->state = UCMD_ST_DONE;
	return r;
}

static inline int
ucmd_undo(struct ucmd* uc) {
	int r;
	wassert(UCMD_ST_DONE == uc->state);
	r = uc->__undo(uc);
	if (!(r < 0))
		uc->state = UCMD_ST_UNDONE;
	return r;
}

static inline int
ucmd_redo(struct ucmd* uc) {
	int r;
	wassert(UCMD_ST_UNDONE == uc->state);
	r = uc->__redo(uc);
	if (!(r < 0))
		uc->state = UCMD_ST_DONE;
	return r;
}

static inline void
ucmd_notify(struct ucmd* uc, void* d0, void* d1) {
	wassert(UCMD_ST_PROGRESS == uc->state);
	uc->__notify(uc, d0, d1);
}

#endif /* _UCMd_h_ */
