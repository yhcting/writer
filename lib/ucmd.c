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

static void
_curve_notify(struct ucmd* uc, void* d0, void* d1) {
	struct ucmdd_crv* d = &uc->d.crv;
	nlist_add(&d->pcrvl, d0);
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
static void
_cut_notify(struct ucmd* uc, void* d0, void* d1) {
	struct ucmdd_cut* d = &uc->d.cut;
	list_replace(d0, &d->lrm);
	list_replace(d1, &d->ladd);
}


/******************************************
 * UCMD - Zoom
 ******************************************/

static int
_zoom_alloc(struct ucmd* uc) {
	return 0;
}

static void
_zoom_free_done(struct ucmd* uc) {
}

static void
_zoom_free_undone(struct ucmd* uc) {
}

static int
_zoom_start(struct ucmd* uc) {
	return 0;
}

static int
_zoom_end(struct ucmd* uc) {
	return 0;
}

static int
_zoom_undo(struct ucmd* uc) {
	return 0;
}

static int
_zoom_redo(struct ucmd* uc) {
	return 0;
}

static void
_zoom_notify(struct ucmd* uc, void* d0, void* d1) {
}


/******************************************
 * UCMD - Move
nn ******************************************/


static int
_move_alloc(struct ucmd* uc) {
	return 0;
}

static void
_move_free_done(struct ucmd* uc) {
}

static void
_move_free_undone(struct ucmd* uc) {
}

static int
_move_start(struct ucmd* uc) {
	return 0;
}

static int
_move_end(struct ucmd* uc) {
	return 0;
}

static int
_move_undo(struct ucmd* uc) {
	return 0;
}

static int
_move_redo(struct ucmd* uc) {
	return 0;
}

static void
_move_notify(struct ucmd* uc, void* d0, void* d1) {
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
		.__notify = &_##pref##_notify,			\
	}

static const struct ucmd _ucmd_init[] = {
	__INIT_UCMD(UCMD_CURVE, curve),
	__INIT_UCMD(UCMD_CUT,   cut),
	__INIT_UCMD(UCMD_ZOOM,  zoom),
	__INIT_UCMD(UCMD_MOVE,  move)
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

