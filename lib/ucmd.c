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

#include "common.h"
#include "gtype.h"
#include "g2d.h"
#include "node.h"
#include "list.h"
#include "ucmd.h"
#include "ahash.h"
#include "curve.h"
#include "wsheet.h"


/******************************************
 * ut functions
 ******************************************/


/******************************************
 * UCMD - Curve
 ******************************************/
struct _curve_link {
	struct list_link    lk;
	struct list_link    crvlk;
};

static inline struct _curve_link*
_curve_linklk(struct list_link* lk) {
	return container_of(lk, struct _curve_link, lk);
}

static int
_curve_alloc(struct ucmd* uc) {
	struct ucmdd_crv* d = &uc->d.crv;
	list_init_link(&d->pcrvl);
	list_init_link(&d->links);
	return 0;
}

static void
_curve_free_done(struct ucmd* uc) {
	struct ucmdd_crv* d = &uc->d.crv;
	wassert(0 == list_size(&d->links));
	/* free node */
	nlist_free(&d->pcrvl);
}

static void
_curve_free_undone(struct ucmd* uc) {
	struct ucmdd_crv*    d = &uc->d.crv;
	struct node*         n;
	struct _curve_link  *clk, *tmp;
	/*
	 * Free memories of curve.
	 * (This is the moment for curve to be really freed
	 *   - removed even from history)
	 */

	/* free all _curve_links */
	list_foreach_item_safe(clk, tmp, &d->links,
			       struct _curve_link, lk)
		wfree(clk);
	list_init_link(&d->links);

	/* free all real curves */
	nlist_foreach(n, &d->pcrvl)
		crv_destroy(n->v);
	nlist_free(&d->pcrvl);
}

static int
_curve_start(struct ucmd* uc) {
	/* struct ucmdd_crv* d = &uc->d.crv; */
	/* nothing to do */
	return 0;
}


static int
_curve_end(struct ucmd* uc) {
	/* struct ucmdd_crv* d = &uc->d.crv; */
	return 0;
}

static int
_curve_undo(struct ucmd* uc) {
	struct ucmdd_crv*     d = &uc->d.crv;
	struct node*          n;
	struct curve*         crv;
	struct _curve_link*   clk;

	wassert(0 == list_size(&d->links));
	nlist_foreach(n, &d->pcrvl) {
		crv = n->v;
		/* backup link information */
		clk = wmalloc(sizeof(*clk));
		clk->crvlk = crv->lk;
		list_add_last(&d->links, &clk->lk);

		/* unlink from division curve list */
		list_del(&crv->lk);
	}
	return 0;
}

static int
_curve_redo(struct ucmd* uc) {
	struct ucmdd_crv*     d = &uc->d.crv;
	struct node*          n;
	struct curve*         crv;
	struct _curve_link   *clk, *tmp;

	wassert(list_size(&d->pcrvl) == list_size(&d->links));

	clk = _curve_linklk(list_first(&d->links));
	nlist_foreach(n, &d->pcrvl) {
		crv = n->v;
		/* restore link information */
		list_add(clk->crvlk._prev, clk->crvlk._next, &crv->lk);
		clk = _curve_linklk(clk->lk._next);
	}

	/* free all _curve_links */
	list_foreach_item_safe(clk, tmp, &d->links, struct _curve_link, lk)
		wfree(clk);
	list_init_link(&d->links);
	return 0;
}

void
ucmd_crv_data(struct ucmd* uc, struct curve* crv) {
	if (!uc)
		return; /* nothing to do */

	wassert(UCMD_CURVE == uc->ty
		&& UCMD_ST_PROGRESS == uc->state);
	nlist_add(&uc->d.crv.pcrvl, crv);
}



/******************************************
 * UCMD - Cut
 ******************************************/

static int
_cut_alloc(struct ucmd* uc) {
	struct ucmdd_cut* d = &uc->d.cut;
	list_init_link(&d->lrm);
	list_init_link(&d->ladd);
	return 0;
}

static void
_cut_free_done(struct ucmd* uc) {
	/* destory removed curves, and all nodes */
	struct ucmdd_cut* d = &uc->d.cut;
	struct node*      n;
	nlist_foreach(n, &d->lrm)
		crv_destroy(n->v);
	nlist_free(&d->lrm);
	nlist_free(&d->ladd);
}

static void
_cut_free_undone(struct ucmd* uc) {
	/* destory newly added curves, and all nodes */
	struct ucmdd_cut* d = &uc->d.cut;
	struct node*      n;
	nlist_foreach(n, &d->ladd)
		crv_destroy(n->v);
	nlist_free(&d->ladd);
	nlist_free(&d->lrm);
}

static int
_cut_start(struct ucmd* uc) {
	return 0;
}

static int
_cut_end(struct ucmd* uc) {
	return 0;
}

static int
_cut_undo(struct ucmd* uc) {
	struct ucmdd_cut* d = &uc->d.cut;
	struct node*      n;
	struct curve*     crv;
	/* restore all 'removed curves' */
	nlist_foreach(n, &d->lrm) {
		crv = n->v;
		list_link(&crv_prev(crv)->lk, &crv->lk);
		list_link(&crv->lk,           &crv_next(crv)->lk);
	}
	return 0;
}

static int
_cut_redo(struct ucmd* uc) {
	struct ucmdd_cut* d = &uc->d.cut;
	struct node*      n;
	struct curve*     crv;
	/* restore newly added curves */
	nlist_foreach(n, &d->ladd) {
		crv = n->v;
		list_link(&crv_prev(crv)->lk, &crv->lk);
		list_link(&crv->lk,           &crv_next(crv)->lk);
	}
	return 0;
}

/*
 * @d0 : list of node of removed curves.
 * @d1 : list of node of newly added curves.
 */
void
ucmd_cut_data(struct ucmd* uc, struct list_link* lrm, struct list_link* ladd) {
	if (!uc) {
		/*
		 * [ TODO ]
		 * This code is somewhat duplicated with 'free_done' in ucmd.c
		 * Is there any better way?
		 */
		/* free notify information */
		struct node*      n;
		nlist_foreach(n, lrm)
			crv_destroy(n->v);
		nlist_free(lrm);
		nlist_free(ladd);
	} else {
		struct ucmdd_cut* d = &uc->d.cut;
		wassert(UCMD_CUT == uc->ty
			&& UCMD_ST_PROGRESS == uc->state);

		list_replace(lrm, &d->lrm);
		list_replace(ladd, &d->ladd);
	}
}


/******************************************
 * UCMD - ZMV
 ******************************************/

static int
_zmv_alloc(struct ucmd* uc) {
	/* nothing to do */
	return 0;
}

static void
_zmv_free_done(struct ucmd* uc) {
	/* nothing to do */
}

static void
_zmv_free_undone(struct ucmd* uc) {
	/* nothing to do */
}

static int
_zmv_start(struct ucmd* uc) {
	/* nothing to do */
	return 0;
}

static int
_zmv_end(struct ucmd* uc) {
	/* nothing to do */
	return 0;
}

static int
_zmv_undo(struct ucmd* uc) {
	return 0;
}

static int
_zmv_redo(struct ucmd* uc) {
	return 0;
}

void
ucmd_zmv_data_before(struct ucmd* uc,
		     int32_t l, int32_t t, int32_t r, int32_t b,
		     int32_t ox, int32_t oy, float zf) {
	if (!uc)
		return;

	struct ucmdd_zmv* zmv = &uc->d.zmv;
	wassert(UCMD_ZMV == uc->ty
		&& UCMD_ST_PROGRESS == uc->state);

	rect_set(&zmv->old.r, l, t, r, b);
	zmv->old.ox = ox;
	zmv->old.oy = oy;
	zmv->old.zf = zf;
}

void
ucmd_zmv_data_after(struct ucmd* uc,
		    int32_t l, int32_t t, int32_t r, int32_t b,
		    int32_t ox, int32_t oy, float zf) {
	if (!uc)
		return;

	struct ucmdd_zmv* zmv = &uc->d.zmv;
	wassert(UCMD_ZMV == uc->ty
		&& UCMD_ST_PROGRESS == uc->state);

	rect_set(&zmv->new.r, l, t, r, b);
	zmv->new.ox = ox;
	zmv->new.oy = oy;
	zmv->new.zf = zf;
}

/******************************************
 *
 ******************************************/

static void
_ucmd_common_free(struct ucmd* uc) {
	switch(uc->state) {

	case UCMD_ST_DONE:
		uc->___free_done(uc);
	break;

	case UCMD_ST_UNDONE:
		uc->___free_undone(uc);
	break;

	default:
		wassert(0);
	}
	return;
}

#define __INIT_UCMD(ucmd, pref)					\
	[ucmd] = {						\
		.ty       = ucmd,				\
		.state    = UCMD_ST_INIT,			\
		.__alloc  = &_##pref##_alloc,			\
		.__free   = &_ucmd_common_free,			\
		.___free_done = &_##pref##_free_done,		\
		.___free_undone = &_##pref##_free_undone,	\
		.__start  = &_##pref##_start,			\
		.__end    = &_##pref##_end,			\
		.__undo   = &_##pref##_undo,			\
		.__redo   = &_##pref##_redo,			\
	}

static const struct ucmd _ucmd_init[] = {
	__INIT_UCMD(UCMD_CURVE, curve),
	__INIT_UCMD(UCMD_ZMV,   zmv),
	__INIT_UCMD(UCMD_CUT,   cut),
};

#undef __INIT_UCMD


struct ucmd*
ucmd_create(enum ucmd_ty type, struct wsheet* wsh) {
	struct ucmd* uc = wmalloc(sizeof *uc);
	wassert(uc);
	*uc = _ucmd_init[type];
	uc->wsh = wsh;
	return uc;
}
