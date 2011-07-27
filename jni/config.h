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

#ifndef __CONFIg_h__
#define __CONFIg_h__

/*
 * For ARM specific optimization!
 *
 * Dualcore optimization test result on ARM
 *     FILL_RECT : optimized code is somewhat slower
 *     FIND_LINE : almost same
 *     DRAW_LINE : duualcore is faster effectively
 */
#define CONFIG_ARCH_ARM

/* Simple configuration for selective compile */
/* #define CONFIG_TEST_EXECUTABLE */

/*
 * Optimization for dual-core
 * This is not well-structured code.
 * Only for staging and prototyping
 */
#define CONFIG_DUALCORE

/*
 * use memory pool for line/node
 * (Not fully implemented yet!!)
 * pros
 *     performance enhencment
 *     (we can save time for calling 'malloc')
 * cons
 *     memory usage is kept as it's maximum vaule.
 *
 * Real test shows that this makes improvement.
 * But, it's not dominant on performance.
 */
#define CONFIG_MEMPOOL

#define CONFIG_DBG_STATISTICS

#endif /* __CONFIg_h__ */
