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
 * Code for unit test executable.
 */
/* #define CONFIG_TEST_EXECUTABLE */

/*
 * Android library
 */
#define CONFIG_ANDROID


/* #define CONFIG_DEBUG */

/*
 * For ARM specific optimization!
 *
 * Dualcore optimization test result on ARM
 *     FILL_RECT : optimized code is somewhat slower
 *     FIND_LINE : almost same
 *     DRAW_LINE : dualcore is faster effectively
 */
#define CONFIG_ARCH_ARM

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
/* #define CONFIG_MEMPOOL */

/* #define CONFIG_DBG_STATISTICS */

/*
 * Configuration for system constants.
 */

/*
 * size of node memory pool
 * (This is effective only when CONFIG_MEMPOOL is enabled)
 */
#define CONFIG_NMP_SZ           256*1024

/*
 * # of bits to use for user-command-history-size for undo/redo.
 * So, 2^(value) becomes size of history.
 */
#ifdef CONFIG_TEST_EXECUTABLE
#	define CONFIG_HISTORY_SZBITS   1
#else /* CONFIG_TEST_EXECUTABLE */
#	define CONFIG_HISTORY_SZBITS   4
#endif /* CONFIG_TEST_EXECUTABLE */


#endif /* __CONFIg_h__ */
