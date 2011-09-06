/*****************************************************************************
 *    Copyright (C) 2011 Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of ylib
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

#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>

#include "common.h"
#include "mempool.h"
/*
 * fbp memory strip (FbpMS)
 * ------------------------
 *
 *     * U(used) / F(free)
 *
 *                fbp
 *             +-------+
 *             |   F   | <- index [mp_sz()-1]
 *             +-------+
 *             |   F   |
 *             +-------+
 *             |  ...  |
 *             +-------+
 *             |   F   |
 *             +-------+
 *      fbi -> |   F   |
 *             +-------+
 *             |   U   |
 *             +-------+
 *             |  ...  |
 *             +-------+
 *             |   U   |
 *             +-------+
 *             |   U   | <- index [0]
 *             +-------+
 *
 *
 * fbp format
 * ----------
 *
 *         0   1   2   <- group index
 *       +---+---+---+ ...
 *       | F | F | F |
 *       | b | b | b |
 *       | p | p | p | ...
 *       | M | M | M |
 *       | S | S | S |
 *       |...|...|...|
 *       +---+---+---+ ...
 *
 *
 * memory block group structure
 * ----------------------------
 *
 * Similar with 'fbp format'
 *
 *
 * Expanding Algorithm
 * -------------------
 *
 * Add additional strip to fbp and block group.
 *
 *
 * Shrinking
 * ---------
 *
 * Actually, shrinking with enough performance is very difficult.
 * Consider it later!
 *
 */

struct _dummy {
	void* dummy;
};

struct _blk {
	/*
	 * 'i' is required to tracking used block.
	 * If sw don't need to track used block,
	 *   this variable 'i' is not required.
	 * So, we can save memory space.
	 * (Actually, writer don't need to track used block.
	 *  But for reusable module, this is implemented at this time.)
	 *
	 * [!NOTE!]
	 * Why 'struct {} d' is used?
	 * Data format of this structure is
	 *
	 * requrested memory address
	 *         |
	 *         v
	 *     +---+--------------------------+
	 *     | i | requested memory address |
	 *     +---+--------------------------+
	 *
	 * So, returned address should be aligned one.
	 * And usually, this is aligned by sizeof(void*)
	 */
	uint32_t      i; /* index of free block pointer */
	struct _dummy d;
};

struct mp {
	unsigned char**   grp;    /* groups of blocks */
	uint32_t          grpsz;  /* size of grp - number of element in group*/
	uint32_t          nrgrp;  /* number of group allocated */
	struct _blk***    fbp;    /* Free Block Pointer group */
	uint32_t          esz;    /* Element SiZe */
	uint32_t          fbi;    /* Free Block Index */
	pthread_mutex_t   m;      /* Mutex for MT safety */
};


static inline void
_init_lock(struct mp* mp) {
	pthread_mutex_init(&mp->m, NULL);
}

static inline void
_lock(struct mp* mp) {
	if (pthread_mutex_lock(&mp->m))
		wassert(0);
}

static inline void
_unlock(struct mp* mp) {
	if (pthread_mutex_unlock(&mp->m))
		wassert(0);
}

static inline void
_destroy_lock(struct mp* mp) {
	pthread_mutex_destroy(&mp->m);
}

static inline uint32_t
_align_adjust(struct mp* mp) {
	return sizeof(void*) - (mp->esz % sizeof(void*));
}

static inline uint32_t
_esz(struct mp* mp) {
	return mp->esz + _align_adjust(mp);
}

static inline uint32_t
_blksz(struct mp* mp) {
	return sizeof(struct _blk) - sizeof(struct _dummy) + _esz(mp);
}

static inline struct _blk*
_blk(struct mp* mp, uint32_t i) {
	return *(mp->fbp[i / mp->grpsz] + i % mp->grpsz);
}

static inline void
_setfbp(struct mp* mp, uint32_t i, struct _blk* b) {
	*(mp->fbp[i / mp->grpsz] + i % mp->grpsz) = b;
}

static inline uint32_t
_sz(struct mp* mp) {
	return mp->nrgrp * mp->grpsz;
}

/*
 * expand memory pool by 1 group
 */
static void
_expand(struct mp* mp) {
	uint32_t        i;
	unsigned char** newgrp;
	struct _blk***  newfbp;
	uint32_t        blksz;

	/* pre-calulate frequently used value */
	blksz = _blksz(mp);

	newgrp = wmalloc(sizeof(*newgrp) * (mp->nrgrp + 1));
	newfbp = wmalloc(sizeof(*newfbp) * (mp->nrgrp + 1));
	wassert(newgrp && newfbp);

	/* allocate new fbp group */
	newfbp[mp->nrgrp] = wmalloc(sizeof(**newfbp) * mp->grpsz);
	/* allocate new block group */
	newgrp[mp->nrgrp] = wmalloc(sizeof(**newgrp) * mp->grpsz * blksz);
	wassert(newfbp[mp->nrgrp] && newgrp[mp->nrgrp]);

	/* initialize fbp & block group */
	for (i = 0; i < mp->grpsz; i++) {
		newfbp[mp->nrgrp][i]
			= (struct _blk*)(newgrp[mp->nrgrp] + i * blksz);
		newfbp[mp->nrgrp][i]->i = _sz(mp) + i;
	}

	/* keep previous values */
	memcpy(newgrp, mp->grp, mp->nrgrp * sizeof(*newgrp));
	memcpy(newfbp, mp->fbp, mp->nrgrp * sizeof(*newfbp));

	/* update mp structure */
	wfree(mp->grp);
	wfree(mp->fbp);
	mp->grp = newgrp;
	mp->fbp = newfbp;
	mp->nrgrp++;
}

struct mp*
mp_create(uint32_t grpsz, uint32_t elemsz) {
	struct mp* mp;

	wassert(grpsz > 0 && elemsz > 0);
	mp = wmalloc(sizeof(*mp));
	mp->grp = wmalloc(sizeof(*mp->grp));
	mp->grpsz = grpsz;
	mp->nrgrp = 0;
	mp->fbp = wmalloc(sizeof(*mp->fbp));
	mp->esz = elemsz;
	mp->fbi = 0;
	_init_lock(mp);

	/* allocate 1-block-group for initial state */
	_expand(mp);

	return mp;
}

void
mp_destroy(struct mp* mp) {
	uint32_t i;
	_destroy_lock(mp);
	for (i = 0; i < mp->nrgrp; i++) {
		wfree(mp->fbp[i]);
		wfree(mp->grp[i]);
	}
	wfree(mp->fbp);
	wfree(mp->grp);
	wfree(mp);
}

/*
 * get one block from pool.
 */
void*
mp_get(struct mp* mp) {
	struct _blk* b;

	_lock(mp);
	if (mp->fbi >= _sz(mp))
		_expand(mp);

	b = _blk(mp, mp->fbi);
	mp->fbi++;
	_unlock(mp);

	return (void*)(&b->d);
}

/*
 * return block to pool.
 */
void
mp_put(struct mp* mp, void* block) {
	struct _blk* b;
	struct _blk* ub; /* used block */
	uint32_t     ti;

	b = container_of(block, struct _blk, d);
	_lock(mp);
	wassert(mp->fbi > 0);
	mp->fbi--;
	ub = _blk(mp, mp->fbi);

	/* swap free block pointer */
	ti = b->i; b->i = ub->i; ub->i = ti;

	_setfbp(mp, ub->i, ub);
	_setfbp(mp, b->i, b);
	_unlock(mp);
}

/*
 * return number of element size
 */
uint32_t
mp_sz(struct mp* mp) {
	uint32_t sz;
	_lock(mp);
	sz = _sz(mp);
	_unlock(mp);
	return sz;
}
