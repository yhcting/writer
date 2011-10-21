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
#include "wsheet.h"

#if 0 /* YHCTODO */
/******************************************
 * ut functions
 ******************************************/

/* head/tail : division node. */
static void
_free_div_nodes(struct node* head, struct node* tail) {
	struct node* n = NULL;
	wassert(head && tail);

	/*
	 * NOTE
	 *   DO NOT use list del !!
	 *   'head' and 'tail' may not be valid link !
	 * node_next(tail) may be invalid address. But, it's ok.
	 */
	while (n != tail) {
		n = head;
		head = node_next(head);
		node_free_deep(n);
	}
}


/******************************************
 * UCMD - Curve
 ******************************************/

static int
_curve_alloc(struct ucmd* uc) {
	struct curves* c = &uc->d.crvs;
	list_init_link(&c->divns);
	list_init_link(&c->crvs);
	return 0;
}

static void
_curve_free(struct ucmd* uc) {
	struct curves* c = &uc->d.crvs;
	struct curve  *crv, *ctmp;

	/*
	 * 'c->divns' should be freed at 'command end'
	 */
	wassert(0 == list_size(&c->divns));
	switch(uc->state) {

	case UCMD_ST_DONE: {
		/*
		 * nothing do do.
		 * division node SHOULD NOT be freed
		 */
		;
	} break;

	case UCMD_ST_UNDONE: {
		/*
		 * 'undo' is executed for the lines added at division.
		 * So, there is no other place to free memory of these lines.
		 * (Division structure doesn't have link to these lines.)
		 * Therefore free those here!
		 */
		list_foreach_item(crv, &c->crvs, struct curve, lk)
			_free_div_nodes(crv->head, crv->tail);
	} break;

	default:
		wassert(0);
	}

	/*
	 * delete curve nodes itself
	 */
	list_foreach_item_removal_safe(crv, ctmp,
				       &c->crvs, struct curve, lk) {
		list_del(&crv->lk);
		wfree(crv);
	}

	return;
}

static int
_curve_start(struct ucmd* uc) {
	/* nothing to do */
	return 0;
}


/*
 * Concept.
 *
 *  * Hashing all node address that should be cutout.
 *  * If prev is NOT in hash, this node is 'start of
 *      curve'.
 *  * If next is NOT in hash, this node is 'end of
 *      curve'.
 *  * If next and prev are all in hash, this is part of
 *      curve. So, link is NOT changed (preserved.)
 *
 * Here are steps
 *
 *  * Iterates lines to find 'start' or 'end' of curve.
 *  * If 'start of curve' is found, do following steps.
 *    Prev is stored as 'prev' at 'struct curves' and
 *      node is attached to 'curve' structure.
 *    (At this moment, node lost its original 'prev' link.
 *     But, it already stored at 'curve' structure.)
 *    Then, continue to follow line link until find end of
 *      curve.
 *  * If 'end of curve' is found, ignore this.
 */
static int
_curve_end(struct ucmd* uc) {
	struct ahash*  ah;
	struct node*   n;
	struct curves* c = &uc->d.crvs;

	ah = ahash_create();
	wassert(ah);

	/* hash all division nodes of curve */
	list_foreach_item(n, &c->divns, struct node, lk)
		ahash_add(ah, n->v);

	/*
	 * Find start of curve. And make curve history...
	 */
	list_foreach_item(n, &c->divns, struct node, lk) {
		struct curve* crv;
		struct node* divn = n->v; /* division node */

		if (ahash_check(ah, node_prev(divn)))
			continue;
		/* 'n->v' is start of curve */
		wassert(((struct line*)divn->v)->divlk == &divn->lk);

		while (ahash_check(ah, divn))
			divn = node_next(divn);

		divn = node_prev(divn);
		/* 'divn' is end of curve */
		wassert(((struct line*)divn->v)->divlk == &divn->lk);

		/* 'n->v' is head 'divn' is tail of curve */
		crv = wmalloc(sizeof *crv);
		crv->head = n->v;
		crv->tail = divn;
		list_add_last(&c->crvs, &crv->lk);
	}

	/*
	 * now we don't need to keep divns.
	 * let's free it.
	 */
	nlist_free(&c->divns);

	ahash_destroy(ah);

	return 0;
}

static int
_curve_undo(struct ucmd* uc) {
	struct curves* c = &uc->d.crvs;
	struct curve*  crv;

	list_foreach_item(crv, &c->crvs, struct curve, lk) {
		/* unlink curve from division */
		list_link(&node_prev(crv->head)->lk,
			  &node_next(crv->tail)->lk);
	}
	return 0;
}

static int
_curve_redo(struct ucmd* uc) {
	struct curves* c = &uc->d.crvs;
	struct node   *p, *n; /* prev and next of this curve */
	struct curve*  crv;

	/*
	 * re-link curve to original division.
	 */
	list_foreach_item(crv, &c->crvs, struct curve, lk) {
		/* c->head and c->tail are keeping it's division link */
		p = node_prev(crv->head);
		n = node_next(crv->tail);
		/* add curve - sub list of division node. */
		node_set_next(p, crv->head);
		node_set_prev(n, crv->tail);
	}

	return 0;
}

static void
_curve_notify(struct ucmd* uc, void* data) {
	struct curves* c = &uc->d.crvs;
	nlist_add(&c->divns, data);
}



/******************************************
 * UCMD - Cut
 ******************************************/

static int
_cut_alloc(struct ucmd* uc) {
	return 0;
}

static void
_cut_free(struct ucmd* uc) {
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
	return 0;
}

static int
_cut_redo(struct ucmd* uc) {
	return 0;
}

static void
_cut_notify(struct ucmd* uc, void* data) {
}


/******************************************
 * UCMD - Zoom
 ******************************************/

static int
_zoom_alloc(struct ucmd* uc) {
	return 0;
}

static void
_zoom_free(struct ucmd* uc) {
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
_zoom_notify(struct ucmd* uc, void* data) {
}


/******************************************
 * UCMD - Move
 ******************************************/


static int
_move_alloc(struct ucmd* uc) {
	return 0;
}

static void
_move_free(struct ucmd* uc) {
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
_move_notify(struct ucmd* uc, void* data) {
}

/******************************************
 * Others
 ******************************************/


#define __INIT_UCMD(ucmd, pref)			\
	[ucmd] = {				\
		.ty       = ucmd,		\
		.state    = UCMD_ST_INIT,	\
		.__alloc  = &_##pref##_alloc,	\
		.__free   = &_##pref##_free,	\
		.__start  = &_##pref##_start,	\
		.__end    = &_##pref##_end,	\
		.__undo   = &_##pref##_undo,	\
		.__redo   = &_##pref##_redo,	\
		.__notify = &_##pref##_notify,	\
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

#endif
