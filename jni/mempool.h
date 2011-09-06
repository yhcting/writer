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

#ifndef _MEMPOOl_h_
#define _MEMPOOl_h_

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


#endif /* _MEMPOOl_h_ */
