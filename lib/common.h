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

#ifndef _COMMOn_H_
#define _COMMOn_H_

#include <stdint.h>

#ifndef offset_of
#       define offset_of(type, member) ((uintptr_t) &((type*)0)->member)
#endif

#ifndef container_of
#       define container_of(ptr, type, member)			\
	((type*)(((int8_t*)(ptr)) - offset_of(type, member)))
#endif

#ifndef NULL
#       define NULL ((void*)0)
#endif


#ifdef CONFIG_ANDROID

#define wjni_get_int_array_direct(env, jarr)	\
	(*env)->GetIntArrayElements(env, jarr, NULL)
#define wjni_release_int_array_direct(env, jarr, var)	\
	(*env)->ReleaseIntArrayElements(env, jarr, var, JNI_ABORT)

#else /* CONFIG_ANDROID */

#define wjni_get_int_array_direct(env, jarr)			\
	((jint*)(*env)->GetPrimitiveArrayCritical(env, jarr, NULL))
#define wjni_release_int_array_direct(env, jarr, var)	\
	(*env)->ReleasePrimitiveArrayCritical(env, jarr, var, 0)

#endif /* CONFIG_ANDROID */


/*
 * To be free from compiler warning
 * (cast from pointer to integer of different size)
 * (cast to pointer from integer of different size)
 */
#define ptr2jlong(v) ((jlong)((intptr_t)(v)))
#define jlong2ptr(v) ((void*)((intptr_t)(v)))

#define arrsz(a) (sizeof(a) / sizeof(a[0]))

#ifndef min
#define min(x, y) ((x) < (y))? (x): (y)
#endif

#ifndef max
#define max(x, y) ((x) < (y))? (y): (x)
#endif


#ifdef CONFIG_DEBUG

#ifdef CONFIG_ANDROID

#include <android/log.h>

#define _TAG_ "libwriter"

#define wlogv(...) __android_log_print(ANDROID_LOG_VERBOSE, _TAG_, __VA_ARGS__)
#define wlogd(...) __android_log_print(ANDROID_LOG_DEBUG  , _TAG_, __VA_ARGS__)
#define wlogi(...) __android_log_print(ANDROID_LOG_INFO   , _TAG_, __VA_ARGS__)
#define wlogw(...) __android_log_print(ANDROID_LOG_WARN   , _TAG_, __VA_ARGS__)
#define wloge(...) __android_log_print(ANDROID_LOG_ERROR  , _TAG_, __VA_ARGS__)
static inline void wwarn() {}

#else /* CONFIG_ANDROID */

#include <stdio.h>

#define wlogv(...) printf(__VA_ARGS__);
#define wlogd(...) printf(__VA_ARGS__);
#define wlogi(...) printf(__VA_ARGS__);
#define wlogw(...) printf(__VA_ARGS__);
#define wloge(...) printf(__VA_ARGS__);
static inline void wwarn() {}

#endif /* CONFIG_ANDROID */

#else /* CONFIG_DEBUG */

static inline void wlogv() {}
static inline void wlogd() {}
static inline void wlogi() {}
static inline void wlogw() {}
static inline void wloge() {}
static inline void wwarn() {}

#endif /* CONFIG_DEBUG */



#ifdef CONFIG_TEST_EXECUTABLE

/*
 * function is declared global only at UNIT TEST
 */
#define DECL_EXTERN_UT(x) x
#define EXTERN_UT

#include <assert.h>

#define wassert(x)  assert(x)
void* wmalloc(uint32_t);
void  wfree(void*);

/* return nr of memory block assigned by wmalloc - not freed*/
int32_t wmblkcnt(void);

/*
 * Priority of test function (smaller is higher).
 * So, fundamental/basic test should have small priy value.
 * For example,
 *   Unit testing for '+' operation should have smaller pri value then
 *     one for calculating distance between two points,
 *     beause, calculating distance requires '+' operation.
 *   Reason is "It is more difficult to debug error in complext operation
 *     than in simple/basic operation".
 */
enum wtestpri {
	/* for initialization */
	TESTPRI_INIT        = 0,
	/* for basic operation */
	TESTPRI_OPERATION,
	/* for basic functions - not for complex function */
	TESTPRI_FUNCTION,
	/* for basic unit test - complex functions etc. */
	TESTPRI_UNIT,
	/* for module */
	TESTPRI_MODULE,
	/* for subsystem */
	TESTPRI_SUBSYSTEM,
	/* for layer */
	TESTPRI_LAYER,
	/* for whole system */
	TESTPRI_SYSTEM,
	/* for testing via user-level interface */
	TESTPRI_USER_ACTION,
	TESTPRI_NR /* number of pri */
};

/*
 * returned value of '(*fn)(void)' is memory count compensation.
 * Ex.
 *  Test function may want to keep some memory block alive.
 *  But, test FW is unhappy with it.
 *  So, test func. should notify this compensation to FW. with return value.
 */
void wregister_tstfn(int32_t (*)(void), const char*, enum wtestpri);

#define TESTFN(fn, mod, pri)						\
	static void __tst_##fn(void) __attribute__ ((constructor));	\
	static void __tst_##fn(void) {					\
		wregister_tstfn(&fn, #mod, pri);			\
	}

#else /* CONFIG_TEST_EXECUTABLE */

#define DECL_EXTERN_UT(x)
#define EXTERN_UT          static

#include <malloc.h>

#define wassert(x)
#define wmalloc(x) malloc(x)
#define wfree(x)   free(x)

#define TESTFN(fn, mod, pri)

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

static inline int32_t _round_off(float x) {
	return (x > 0)? (int32_t)(x + .5f): (int32_t)(x - .5f);
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
dbg_tpf_check_start(uint32_t cat);

void
dbg_tpf_check_end(uint32_t cat);

void
dbg_tpf_print(uint32_t cat);

void
dbg_tpf_init(void);

#else /* CONFIG_DBG_STATISTICS */

#define dbg_tpf_check_start(x)
#define dbg_tpf_check_end(x)
#define dbg_tpf_print(x)
#define dbg_tpf_init()

#endif /* CONFIG_DBG_STATISTICS */

#endif /* _COMMOn_H_ */
