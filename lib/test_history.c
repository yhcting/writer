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
#include "list.h"
#include "div.h"
#include "test_values.h"

static inline int
_crvnr(struct wsheet*, int, int) __attribute__ ((unused));
static inline int
_crvnr(struct wsheet* wsh, int row, int col) {
	return list_size(&wsh->divs[row][col].crvs);
}

static void
_check_div_sz11(struct wsheet*, int, int, int, int)
__attribute__ ((unused));
static void
_check_div_sz11(struct wsheet* wsh,
		int d00, int d01, int d10, int d11) {
	if(!(d00 == _crvnr(wsh, 0, 0)
	     && d01 == _crvnr(wsh, 0, 1)
	     && d10 == _crvnr(wsh, 1, 0)
	     && d11 == _crvnr(wsh, 1, 1))) {
		printf("======== div check error ========\n"
		       "    %d    %d\n"
		       "    %d    %d\n"
		       ,_crvnr(wsh, 0, 0), _crvnr(wsh, 0, 1)
		       ,_crvnr(wsh, 1, 0), _crvnr(wsh, 1, 1));
		wassert(0);
	}
}

static void
_check_div_sz12(struct wsheet*,	int, int, int, int, int, int)
__attribute__ ((unused));
static void
_check_div_sz12(struct wsheet* wsh,
		int d00, int d01, int d02,
		int d10, int d11, int d12) {
	if(!(d00 == _crvnr(wsh, 0, 0)
	     && d01 == _crvnr(wsh, 0, 1)
	     && d02 == _crvnr(wsh, 0, 2)
	     && d10 == _crvnr(wsh, 1, 0)
	     && d11 == _crvnr(wsh, 1, 1)
	     && d12 == _crvnr(wsh, 1, 2))) {
		printf("======== div check error ========\n"
		       "    %d    %d    %d\n"
		       "    %d    %d    %d\n"
		       ,_crvnr(wsh, 0, 0), _crvnr(wsh, 0, 1)
		       ,_crvnr(wsh, 0, 2), _crvnr(wsh, 1, 0)
		       ,_crvnr(wsh, 1, 1), _crvnr(wsh, 1, 2));
		wassert(0);
	}
}

static void
_check_div_sz22(struct wsheet*,	int, int, int, int, int, int, int, int, int)
__attribute__ ((unused));
static void
_check_div_sz22(struct wsheet* wsh,
		int d00, int d01, int d02,
		int d10, int d11, int d12,
		int d20, int d21, int d22) {
	if(!(d00 == _crvnr(wsh, 0, 0)
	     && d01 == _crvnr(wsh, 0, 1)
	     && d02 == _crvnr(wsh, 0, 2)
	     && d10 == _crvnr(wsh, 1, 0)
	     && d11 == _crvnr(wsh, 1, 1)
	     && d12 == _crvnr(wsh, 1, 2)
	     && d20 == _crvnr(wsh, 2, 0)
	     && d21 == _crvnr(wsh, 2, 1)
	     && d22 == _crvnr(wsh, 2, 2))) {
		printf("======== div check error ========\n"
		       "    %d    %d    %d\n"
		       "    %d    %d    %d\n"
		       "    %d    %d    %d\n"
		       ,_crvnr(wsh, 0, 0), _crvnr(wsh, 0, 1)
		       ,_crvnr(wsh, 0, 2), _crvnr(wsh, 1, 0)
		       ,_crvnr(wsh, 1, 1), _crvnr(wsh, 1, 2)
		       ,_crvnr(wsh, 2, 0), _crvnr(wsh, 2, 1)
		       ,_crvnr(wsh, 2, 2));
		wassert(0);
	}
}


static void
_ucmd_curve(struct wsheet* wsh) {
	wsheet_clean(wsh);
	his_clean();
	wsheet_add_curve(wsh, _pts0, arrsz(_pts0) / 2, 1, 0);
	_check_div_sz11(wsh, 1, 1, 0, 1);
	his_undo();
	_check_div_sz11(wsh, 0, 0, 0, 0);
	his_redo();
	_check_div_sz11(wsh, 1, 1, 0, 1);

}

static void
_ucmd_cutout(struct wsheet* wsh) {
	wsheet_clean(wsh);
	his_clean();
	wsheet_add_curve(wsh, _pts0, arrsz(_pts0) / 2, 1, 0);
	_check_div_sz11(wsh, 1, 1, 0, 1);
	wsheet_cutout_lines(wsh, 13, 2, 18, 9);
	_check_div_sz11(wsh, 1, 2, 0, 1);
	his_undo();
	_check_div_sz11(wsh, 1, 1, 0, 1);
	his_redo();
	_check_div_sz11(wsh, 1, 2, 0, 1);
	his_undo();
	his_undo();
	_check_div_sz11(wsh, 0, 0, 0, 0);
	his_undo();
	_check_div_sz11(wsh, 0, 0, 0, 0);
	his_redo();
	his_redo();
	his_redo();
	_check_div_sz11(wsh, 1, 2, 0, 1);

	/* complex user command */
	wsheet_clean(wsh);
	his_clean();
	wsheet_add_curve(wsh, _pts0, arrsz(_pts0) / 2, 1, 0);
	wsheet_add_curve(wsh, _pts2, arrsz(_pts2) / 2, 1, 0);
	_check_div_sz11(wsh, 2, 2, 0, 1);
	wsheet_cutout_lines(wsh, 13, 2, 18, 9);
	_check_div_sz11(wsh, 2, 4, 0, 1);
	his_undo();
	_check_div_sz11(wsh, 2, 2, 0, 1);
	his_undo();
	_check_div_sz11(wsh, 1, 1, 0, 1);
	wsheet_cutout_lines(wsh, 13, 2, 18, 9);
	_check_div_sz11(wsh, 1, 2, 0, 1);
	wsheet_add_curve(wsh, _pts2, arrsz(_pts2) / 2, 1, 0);
	_check_div_sz11(wsh, 2, 3, 0, 1);
	his_redo();
	_check_div_sz11(wsh, 2, 3, 0, 1);
	his_undo();
	_check_div_sz11(wsh, 1, 2, 0, 1);
	his_redo();
	_check_div_sz11(wsh, 2, 3, 0, 1);
	wsheet_cutout_lines(wsh, 4, 1, 8, 8);
	_check_div_sz11(wsh, 3, 3, 0, 1);
	wsheet_cutout_lines(wsh, 2, 1, 3, 8);
	_check_div_sz11(wsh, 4, 3, 0, 1);
}

static int
_test_history(void) {
	struct wsheet* wsh;
	wsh = wsheet_create();
	wsheet_init(wsh, 10, 10, 4, 4);

#define __do(testname, func)						\
	printf(" + " testname " => ");					\
	func;								\
	printf("passed\n");

	__do("ucmd curve",   _ucmd_curve(wsh));
	__do("ucmd cutout",  _ucmd_cutout(wsh));

	wsheet_destroy(wsh);
	wsys_deinit();
	return 0;
}

#undef __addpts

#undef _divlsz

TESTFN(_test_history, HISTORY, TESTPRI_SUBSYSTEM)



#endif /* CONFIG_TEST_EXECUTABLE */

