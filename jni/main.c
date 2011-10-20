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


#ifdef CONFIG_DBG_STATISTICS

/* This is for debugging for library */
#include <sys/time.h>
#include <assert.h>
#include <stdio.h>

#ifndef CONFIG_TEST_EXECUTABLE
#	include <android/log.h>
#endif /* CONFIG_TEST_EXECUTABLE */

#include "common.h"
#include "wsheet.h"

/*
 * tpf : Time PerFormance
 */
static struct _tpf {
	uint64_t tmp; /* tmp for start time */
	uint64_t t;   /* average time */
	uint32_t cnt; /* measure count */
} _tpfs[DBG_PERF_NR];

static uint64_t
_utime(void) {
	struct timeval tv;
	if (gettimeofday(&tv, NULL))
		assert(0);
	return (uint64_t)(tv.tv_sec * 1000000)
		+ (uint64_t)tv.tv_usec;
}

void
dbg_tpf_check_start(uint32_t cat) {
	_tpfs[cat].tmp = _utime();
}

/*
 * NOTE
 *     : Be careful to avoid 'overflow'!
 */
void
dbg_tpf_check_end(uint32_t cat) {
	uint64_t     t = _utime();
	struct _tpf* p = &_tpfs[cat];
	/*p->t = (t - p->tmp) / (p->cnt + 1) + p->cnt / (p->cnt + 1) * p->t;*/
	p->t = (p->t * p->cnt + (t - p->tmp)) / (p->cnt + 1);
	p->cnt++;
}

void
dbg_tpf_print(uint32_t cat) {
	char    msg[1024];
	sprintf(msg, "%lld : %d\n", _tpfs[cat].t, _tpfs[cat].cnt);
#ifdef CONFIG_TEST_EXECUTABLE
	printf("%s\n", msg);
#else /* CONFIG_TEST_EXECUTABLE */
	__android_log_print(ANDROID_LOG_ERROR, "WPERF", msg);
#endif /* CONFIG_TEST_EXECUTABLE */
}

void
dbg_tpf_init() {
	uint32_t i;
	for (i = 0; i < DBG_PERF_NR; i++) {
		_tpfs[i].t = 0;
		_tpfs[i].cnt = 0;
	}
}

#endif /* CONFIG_DBG_STATISTICS */



#ifdef CONFIG_TEST_EXECUTABLE

#include <stdio.h>
#include <malloc.h>
#include <assert.h>

#include "list.h"
#include "wsheet.h"

struct _tstfn {
	int32_t         (*fn)(void);
	const char*       modname;
	struct list_link  lk;
};

#define _INITFNL(pri) [pri] = {&_tstfnl[pri], &_tstfnl[pri]}
struct list_link _tstfnl[TESTPRI_NR] = {
	_INITFNL(TESTPRI_INIT),
	_INITFNL(TESTPRI_OPERATION),
	_INITFNL(TESTPRI_FUNCTION),
	_INITFNL(TESTPRI_UNIT),
	_INITFNL(TESTPRI_MODULE),
	_INITFNL(TESTPRI_SUBSYSTEM),
	_INITFNL(TESTPRI_LAYER),
	_INITFNL(TESTPRI_SYSTEM),
	_INITFNL(TESTPRI_USER_ACTION)
};
#undef _INITFNL

static int32_t _memblk = 0;

void
wregister_tstfn(int32_t (*fn)(void), const char* mod, enum wtestpri pri) {
	/* malloc should be used instead of wmalloc */
	struct _tstfn* n = malloc(sizeof(*n));
	n->fn = fn;
	n->modname = mod;
	list_add_last(&_tstfnl[pri], &n->lk);
}

void*
wmalloc(uint32_t sz) {
	_memblk++;
	return malloc(sz);
}

void
wfree(void* p) {
	_memblk--;
	free(p);
}

int32_t
wmblkcnt(void) {
	return _memblk;
}


int
main()
{
	int32_t           sv, compen;
	struct _tstfn*    p;
	int               i;

	for (i = 0; i < TESTPRI_NR; i++)
		list_foreach_item(p, &_tstfnl[i], struct _tstfn, lk) {
			printf("<< Test [%s] >>\n", p->modname);
			sv = wmblkcnt();
			compen = (*p->fn)();
			if (sv + compen != wmblkcnt()) {
				printf("Unbalanced memory at [%s]!\n"
				       "    balance : %d\n",
				       p->modname,
				       wmblkcnt() - sv - compen);
				wassert(0);
			}
			printf("  => PASSED [%s]\n", p->modname);
		}

	if (wmblkcnt())
		printf("Unbalanced memory!\n"
		       "    balance : %d\n",
		       wmblkcnt());

	dbg_tpf_print(DBG_PERF_FILL_RECT);
	dbg_tpf_print(DBG_PERF_FIND_LINE);
	dbg_tpf_print(DBG_PERF_DRAW_LINE);
	printf("=== TEST SUCCESS!!! ===\n");
	return 0;
}
#endif /* CONFIG_TEST_EXECUTABLE */
