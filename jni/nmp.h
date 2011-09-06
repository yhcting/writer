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
 * Memory Pool for Node
 */
#ifndef _NMp_h_
#define _NMp_h_

#ifdef CONFIG_MEMPOOL

#include "common.h"
#include "gtype.h"
#include "mempool.h"

extern struct mp* g_wsheet_nmp;

static inline void
nmp_create(int grpsz) {
	g_wsheet_nmp = mp_create(grpsz, sizeof(struct node));
}

static inline void
nmp_destroy(void) {
	mp_destroy(g_wsheet_nmp);
}

static inline struct node*
nmp_alloc(void) {
	return (struct node*)mp_get(g_wsheet_nmp);
}

static inline void
nmp_free(struct node* n) {
	mp_put(g_wsheet_nmp, n);
}

#else /* CONFIG_MEMPOOL */

#include "gtype.h"

#define nmp_create(x)
#define nmp_destroy()

static inline struct node*
nmp_alloc(void) {
	return wmalloc(sizeof(struct node));
}

static inline void
nmp_free(struct node* n) {
	wfree(n);
}


#endif /* CONFIG_MEMPOOL */


#endif /* _NMp_h_ */
