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

#ifdef CONFIG_TEST_EXECUTABLE
#include <stdio.h>

#include "wsheet.h"
#include "history.h"

#include "testpts.h"


#define _divlsz(r, c)  list_size(&wsh->divs[r][c].lns)

#define _addpts(pts)							\
	wsheet_add_curve(wsh, pts,					\
			 sizeof(pts) / sizeof((pts)[0]) / 2, 1, 0);

static void
_ucmd_curve() {
	struct wsheet* wsh;

	/* assumption of this test */
	wassert(2 == (1 << CONFIG_HISTORY_SZBITS));

	/**************
	 * TEST 1
	 **************/
	wsh = wsheet_create();
	wsheet_init(wsh, 10, 10, 5, 5);

	_addpts(_pts0);
	wassert(2 == _divlsz(0, 0)
		&& 2 == _divlsz(0, 1)
		&& 0 == _divlsz(0, 2)
		&& 0 == _divlsz(1, 0)
		&& 2 == _divlsz(1, 1)
		&& 1 == _divlsz(1, 2)
		);

	his_undo();
	wassert(0 == _divlsz(0, 0)
		&& 0 == _divlsz(0, 1)
		&& 0 == _divlsz(0, 2)
		&& 0 == _divlsz(1, 0)
		&& 0 == _divlsz(1, 1)
		&& 0 == _divlsz(1, 2)
		);

	his_redo();
	wassert(2 == _divlsz(0, 0)
		&& 2 == _divlsz(0, 1)
		&& 0 == _divlsz(0, 2)
		&& 0 == _divlsz(1, 0)
		&& 2 == _divlsz(1, 1)
		&& 1 == _divlsz(1, 2)
		);

	wsheet_destroy(wsh);
	wsys_deinit();



	/**************
	 * TEST 2
	 **************/
	wsh = wsheet_create();
	wsheet_init(wsh, 10, 10, 5, 5);

	/*
	 * History size of test is 2.
	 * So, last 'add' overwrites first history.
	 */
	_addpts(_pts0);
	_addpts(_pts1);
	_addpts(_pts2);

	his_undo();
	his_undo();
	wassert(0 == his_sz());
	his_undo(); /* this will do nothing */
	wassert(0 == his_sz() && 2 == his_undosz());

	wassert(2 == _divlsz(0, 0)
		&& 2 == _divlsz(0, 1)
		&& 0 == _divlsz(0, 2)
		&& 0 == _divlsz(1, 0)
		&& 2 == _divlsz(1, 1)
		&& 1 == _divlsz(1, 2)
		);

	wsheet_destroy(wsh);
	wsys_deinit();

}



static int
_test_history(void) {
	int32_t memsv;

#define __do(testname, func)			\
	printf(" + " testname " => ");		\
	memsv = wmblkcnt();			\
	func;					\
	wassert(memsv == wmblkcnt());		\
	printf("passed\n");

	__do("ucmd curve", _ucmd_curve());

	return 0;
}

#undef __addpts

#undef _divlsz

TESTFN(_test_history, HISTORY, TESTPRI_SUBSYSTEM)



#endif /* CONFIG_TEST_EXECUTABLE */

