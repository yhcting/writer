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
	unsigned long long tmp; /* tmp for start time */
	unsigned long long t;   /* average time */
	unsigned int       cnt; /* measure count */
} _tpfs[DBG_PERF_NR];

static unsigned long long
_utime(void) {
	struct timeval tv;
	if (gettimeofday(&tv, NULL))
		assert(0);
	return (unsigned long long)(tv.tv_sec * 1000000)
		+ (unsigned long long)tv.tv_usec;
}

void
dbg_tpf_check_start(int cat) {
	_tpfs[cat].tmp = _utime();
}

/*
 * NOTE
 *     : Be careful to avoid 'overflow'!
 */
void
dbg_tpf_check_end(int cat) {
	unsigned long long t = _utime();
	struct _tpf*       p = &_tpfs[cat];
	/*p->t = (t - p->tmp) / (p->cnt + 1) + p->cnt / (p->cnt + 1) * p->t;*/
	p->t = (p->t * p->cnt + (t - p->tmp)) / (p->cnt + 1);
	p->cnt++;
}

void
dbg_tpf_print(int cat) {
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
	int i;
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
	int             (*fn)(void);
	const char*       modname;
	struct list_link  lk;
};

static list_decl_head(_tstfnl);
static int _memblk = 0;


void
wregister_tstfn(int (*fn)(void), const char* mod) {
	/* malloc should be used instead of wmalloc */
	struct _tstfn* n = malloc(sizeof(*n));
	n->fn = fn;
	n->modname = mod;
	list_add_last(&_tstfnl, &n->lk);
}

void*
wmalloc(unsigned int sz) {
	_memblk++;
	return malloc(sz);
}

void
wfree(void* p) {
	_memblk--;
	free(p);
}

int
wmblkcnt(void) {
	return _memblk;
}


int
main()
{
	int               sv, compen;
	struct _tstfn*    p;

	list_foreach_item(p, &_tstfnl, struct _tstfn, lk) {
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
		printf("<< PASSED [%s] >>\n", p->modname);
	}

	if (wmblkcnt())
		printf("Unbalanced memory!\n"
		       "    balance : %d\n",
		       wmblkcnt());

#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_print(DBG_PERF_FILL_RECT);
	dbg_tpf_print(DBG_PERF_FIND_LINE);
	dbg_tpf_print(DBG_PERF_DRAW_LINE);
#endif /* CONFIG_DBG_STATISTICS */
	printf("=== TEST SUCCESS!!! ===\n");
	return 0;
}
#endif /* CONFIG_TEST_EXECUTABLE */
