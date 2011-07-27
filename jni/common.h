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

#ifndef _COMMOn_H_
#define _COMMOn_H_

#ifndef offset_of
#       define offset_of(type, member) ((unsigned long) &((type*)0)->member)
#endif

#ifndef container_of
#       define container_of(ptr, type, member)			\
	((type*)(((char*)(ptr)) - offset_of(type, member)))
#endif


#ifdef CONFIG_TEST_EXECUTABLE

#include <assert.h>

extern int g_used_memblk;

#   define ASSERT(x) assert(x)

#   define MALLOC(pTR, tYPE, sIZE)		\
	do {					\
		(pTR) = (tYPE)malloc(sIZE);	\
		g_used_memblk++;		\
	} while(0);

#   define FREE(pTR)				\
	do {					\
		free(pTR);			\
		g_used_memblk--;		\
	} while(0);

#else /* CONFIG_TEST_EXECUTABLE */

#   define ASSERT(x)

#   define MALLOC(pTR, tYPE, sIZE)		\
	do {					\
		(pTR) = (tYPE)malloc(sIZE);	\
	} while(0);

#   define FREE(pTR)				\
	do {					\
		free(pTR);			\
	} while(0);

#endif /* CONFIG_TEST_EXECUTABLE */



#define ABS(x) (((x) > 0)? (x): -(x))
#define SWAP(x,y,tmp) do { tmp = x; x = y; y = tmp; } while(0)

#define UNROLL16( eXPR, cOUNT, cOND)		\
	switch( (cOUNT) & 0xF ) {		\
	case 0: while (cOND){			\
			eXPR;			\
		case 15: eXPR;			\
		case 14: eXPR;			\
		case 13: eXPR;			\
		case 12: eXPR;			\
		case 11: eXPR;			\
		case 10: eXPR;			\
		case 9: eXPR;			\
		case 8: eXPR;			\
		case 7: eXPR;			\
		case 6: eXPR;			\
		case 5: eXPR;			\
		case 4: eXPR;			\
		case 3: eXPR;			\
		case 2: eXPR;			\
		case 1: eXPR;			\
		}				\
	}


#define MIN(x,y) (((x) < (y))? x: y)
#define MAX(x,y) (((x) < (y))? y: x)



static inline int _round_off(float x) {
	return (x > 0)? (int)(x + .5f): (int)(x - .5f);
}

#ifdef CONFIG_DBG_STATISTICS

/* performance category */
enum {
	DBG_PERF_FILL_RECT = 0,
	DBG_PERF_FIND_LINE,
	DBG_PERF_DRAW_LINE,
	DBG_PERF_NR,
};

void
dbg_tpf_check_start(int cat);

void
dbg_tpf_check_end(int cat);

void
dbg_tpf_print(int cat);

void
dbg_tpf_init(void);

#endif /* CONFIG_DBG_STATISTICS */

#ifdef CONFIG_MEMPOOL

/****************
 * NOTE!
 *    Memory pool should be multi-thread-safe!!!
 *    (mp_create / mp_destroy are not MT safe!)
 ****************/

/*
 * mp : Memory Pool
 */
struct mp;

/*
 * size of pool will be expanded if needed.
 * this is just 'initial' size
 * grpsz  : memory block gropu size (number of element)
 * elemsz : element size (in bytes)
 */
struct mp*
mp_create(int elem_nr, int elem_sz);

void
mp_destroy(struct mp*);

/*
 * get one block from pool.
 */
void*
mp_get(struct mp*);

/*
 * return block to pool.
 */
void
mp_put(struct mp*, void* block);

/*
 * return number of element size
 */
int
mp_sz(struct mp*);


#endif /* CONFIG_MEMPOOL */

#endif /* _COMMOn_H_ */
