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
#include "wsheet.h"

#include <stdio.h>
#include <assert.h>

static const int    _DIV_W  = 20;
static const int    _DIV_H  = 10;
static const int    _COL_N  = 10;
static const int    _ROW_N  = 10;

static void
_line_sanity_check(struct wsheet* wsh) {
	struct list_link   lout;
	struct node*       n;
	struct line*       ln;
	int	           minx, maxx, miny, maxy, li, ri, ti, bi;

	list_init_link(&lout);
	wsheet_find_lines(wsh, &lout,
			  0, 0,
			  wsh->divW * wsh->colN,
			  wsh->divH * wsh->rowN);
	list_foreach_item(n, &lout, struct node, lk) {
		ln = n->ln;
		minx = MIN(ln->x0, ln->x1);
		maxx = MAX(ln->x0, ln->x1);
		miny = MIN(ln->y0, ln->y1);
		maxy = MAX(ln->y0, ln->y1);
		li = minx/wsh->divW;
		ri = (0 == maxx % wsh->divW)?
			maxx / wsh->divW - 1:
			maxx / wsh->divW;
		ti = miny / wsh->divH;
		bi = (0 == maxy % wsh->divH)?
			maxy / wsh->divH - 1:
			maxy / wsh->divH;
	}
	wlist_clean(&lout);
}

static int
_test_add(struct wsheet* wsh) {
	struct list_link lout;

	printf(" + adding lines => ");
	fflush(stdout);
	list_init_link(&lout);
	/***************
	 * extends over only one division
	 ***************/
	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 1, 2, 2, 1, 0);

	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[1][1].lns)
		);


	wsheet_clean(wsh);
	wsheet_add(wsh, _DIV_W-1, 2, 0, 1, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[1][1].lns)
		);

	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 0, 2, _DIV_H-1, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[1][1].lns)
		);

	wsheet_clean(wsh);
	wsheet_add(wsh, _DIV_W-1, _DIV_H-1, 0, 0, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[1][1].lns)
		);


	/***************
	 * extends over two division
	 ***************/
	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 1, _DIV_W + 1, 2, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 1 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[0][2].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[1][1].lns)
		);

	wsheet_clean(wsh);
	wsheet_add(wsh, _DIV_W, 2, 1, 1, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[2][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[1][1].lns)
		);

	wsheet_clean(wsh);
	wsheet_add(wsh, 2, 1, 1, _DIV_H + 1, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 1 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[1][1].lns)
		);

	wsheet_clean(wsh);
	/* _DIV_W >= 2*_DIV_H */
	wsheet_add(wsh, _DIV_W, _DIV_H, _DIV_W*2, _DIV_H*2, 1, 0);
	wassert(0 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 1 == list_size(&wsh->divs[1][1].lns)
		&& 0 == list_size(&wsh->divs[2][1].lns)
		&& 0 == list_size(&wsh->divs[1][2].lns)
		&& 0 == list_size(&wsh->divs[2][2].lns)
		&& 0 == list_size(&wsh->divs[3][2].lns)
		&& 0 == list_size(&wsh->divs[2][3].lns)
		&& 0 == list_size(&wsh->divs[3][3].lns)
		);

	/***************
	 * extends over three division
	 ***************/
	wsheet_clean(wsh);
	wsheet_add(wsh, _DIV_W*2+1, 2, 1, 1, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 1 == list_size(&wsh->divs[0][1].lns)
		&& 1 == list_size(&wsh->divs[0][2].lns)
		&& 0 == list_size(&wsh->divs[0][3].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[2][0].lns)
		&& 0 == list_size(&wsh->divs[3][0].lns)
		);
	_line_sanity_check(wsh);



	wsheet_clean(wsh);
	wsheet_add(wsh, 2, _DIV_H*2+1, 1, 1, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[0][2].lns)
		&& 0 == list_size(&wsh->divs[0][3].lns)
		&& 1 == list_size(&wsh->divs[1][0].lns)
		&& 1 == list_size(&wsh->divs[2][0].lns)
		&& 0 == list_size(&wsh->divs[3][0].lns)
		);
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	wsheet_add(wsh, _DIV_W-1, 1, _DIV_W*2-1, _DIV_H+1, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 1 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 1 == list_size(&wsh->divs[1][1].lns)
		);
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	wsheet_add(wsh, 1, _DIV_H-1, _DIV_W+1, _DIV_H+1, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 1 == list_size(&wsh->divs[1][0].lns)
		&& 1 == list_size(&wsh->divs[1][1].lns)
		);
	_line_sanity_check(wsh);

	/***************
	 * extends over several division
	 ***************/
	wsheet_clean(wsh);
	wsheet_add(wsh, 1, _DIV_H-1, _DIV_W*3+1, _DIV_H*2+1, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[0][2].lns)
		&& 0 == list_size(&wsh->divs[0][3].lns)
		&& 1 == list_size(&wsh->divs[1][0].lns)
		&& 1 == list_size(&wsh->divs[1][1].lns)
		&& 1 == list_size(&wsh->divs[1][2].lns)
		&& 0 == list_size(&wsh->divs[1][3].lns)
		&& 0 == list_size(&wsh->divs[2][0].lns)
		&& 0 == list_size(&wsh->divs[2][1].lns)
		&& 1 == list_size(&wsh->divs[2][2].lns)
		&& 1 == list_size(&wsh->divs[2][3].lns)
		&& 0 == list_size(&wsh->divs[3][0].lns)
		&& 0 == list_size(&wsh->divs[3][1].lns)
		&& 0 == list_size(&wsh->divs[3][2].lns)
		&& 0 == list_size(&wsh->divs[3][3].lns)
		);
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	wsheet_add(wsh, _DIV_W-1, 1, _DIV_W*2+1, _DIV_H*6-1, 1, 0);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[2][0].lns)
		&& 1 == list_size(&wsh->divs[0][1].lns)
		&& 1 == list_size(&wsh->divs[1][1].lns)
		&& 1 == list_size(&wsh->divs[2][1].lns)
		&& 1 == list_size(&wsh->divs[3][1].lns)
		&& 1 == list_size(&wsh->divs[4][1].lns)
		&& 1 == list_size(&wsh->divs[5][1].lns)
		&& 0 == list_size(&wsh->divs[6][1].lns)
		&& 0 == list_size(&wsh->divs[0][2].lns)
		&& 0 == list_size(&wsh->divs[1][2].lns)
		&& 0 == list_size(&wsh->divs[3][2].lns)
		&& 0 == list_size(&wsh->divs[4][2].lns)
		&& 1 == list_size(&wsh->divs[5][2].lns)
		);
	_line_sanity_check(wsh);


	wsheet_clean(wsh);
	wsheet_add(wsh, 1, _DIV_H*4-1, _DIV_W*2+1, 1, 1, 0);
	wassert(0 == list_size(&wsh->divs[0][0].lns)
		&& 1 == list_size(&wsh->divs[3][0].lns)
		&& 1 == list_size(&wsh->divs[2][0].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[3][1].lns)
		&& 1 == list_size(&wsh->divs[2][1].lns)
		&& 1 == list_size(&wsh->divs[1][1].lns)
		&& 1 == list_size(&wsh->divs[0][1].lns)
		&& 1 == list_size(&wsh->divs[0][2].lns)
		&& 0 == list_size(&wsh->divs[1][2].lns)
		);
	_line_sanity_check(wsh);

	printf("passed\n");
	return 0;
}

static int
_test_find_lines(struct wsheet* wsh) {
	struct list_link lns;

	printf(" + finding lines => ");
	fflush(stdout);

	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 1, _DIV_W-1, 1, 1, 0);
	wsheet_add(wsh, 1, 3, _DIV_W-1, _DIV_H-1, 1, 0);
	wsheet_add(wsh, 1, _DIV_H-1, _DIV_W-1, _DIV_H-2, 1, 0);

	list_init_link(&lns);
	wsheet_find_lines(wsh, &lns, 0, 0, 1, 1);
	wassert(0 == list_size(&lns));

	wlist_clean(&lns);
	wsheet_find_lines(wsh, &lns, 0, 0, 2, 2);
	wassert(1 == list_size(&lns));

	wlist_clean(&lns);
	wsheet_find_lines(wsh, &lns, 0, 0, 4, 4);
	wassert(2 == list_size(&lns));

	wlist_clean(&lns);
	wsheet_find_lines(wsh, &lns, 1, 1, 2, 2);
	wassert(1 == list_size(&lns));

	wlist_clean(&lns);
	wsheet_find_lines(wsh, &lns, 2, 2, 4, 4);
	wassert(1 == list_size(&lns));

	wlist_clean(&lns);
	wsheet_find_lines(wsh, &lns, 3, 0, 4, _DIV_H);
	wassert(3 == list_size(&lns));

	wlist_clean(&lns);

	printf("passed\n");
	return 0;
}

static int
_test_cutout(struct wsheet* wsh) {
	printf(" + cutting out lines => ");

	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 1, 3, 3, 1, 0);
	wsheet_cutout(wsh, 0, 0, _DIV_W, _DIV_H);
	wassert(0 == list_size(&wsh->divs[0][0].lns));

	wsheet_clean(wsh);
	wsheet_add(wsh, 0, 0, 1, 1, 1, 0);
	wsheet_cutout(wsh, 0, 0, _DIV_W, _DIV_H);
	wassert(0 == list_size(&wsh->divs[0][0].lns));

	wsheet_clean(wsh);
	wsheet_add(wsh, 3, 3, _DIV_W, _DIV_H, 1, 0);
	wsheet_cutout(wsh, 0, 0, _DIV_W, _DIV_H);
	wassert(0 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[1][1].lns)
		);

	wsheet_clean(wsh);
	wsheet_add(wsh, 1, _DIV_H, _DIV_W-1, _DIV_H, 1, 0);
	wsheet_cutout(wsh, 0, 0, _DIV_W, _DIV_H);
	wassert(0 == list_size(&wsh->divs[0][0].lns)
		&& 1 == list_size(&wsh->divs[1][0].lns)
		);

	wsheet_clean(wsh);
	wsheet_add(wsh, _DIV_W, 1, _DIV_W, _DIV_H-1, 1, 0);
	wsheet_cutout(wsh, 0, 0, _DIV_W, _DIV_H);
	wassert(0 == list_size(&wsh->divs[0][0].lns)
		&& 1 == list_size(&wsh->divs[0][1].lns)
		);


	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 1, _DIV_W, _DIV_H, 1, 0);
	wsheet_cutout(wsh, 1, 1, _DIV_W, _DIV_H);
	wassert(0 == list_size(&wsh->divs[0][0].lns)
		&& 0 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[1][1].lns)
		);

	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 1, _DIV_W-2, 1, 1, 0);
	wsheet_cutout(wsh, 2, 0, _DIV_W-1, 3);
	wassert(1 == list_size(&wsh->divs[0][0].lns));

	wsheet_clean(wsh);
	wsheet_add(wsh, 2, 1, _DIV_W-1, 1, 1, 0);
	wsheet_cutout(wsh, 1, 0, _DIV_W-2, 3);
	wassert(1 == list_size(&wsh->divs[0][0].lns));

	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 1, _DIV_W-1, 1, 1, 0);
	wsheet_cutout(wsh, 2, 0, _DIV_W-2, 3);
	wassert(2 == list_size(&wsh->divs[0][0].lns));


	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 1, 1, _DIV_H-2, 1, 0);
	wsheet_cutout(wsh, 0, 2, 3, _DIV_H-1);
	wassert(1 == list_size(&wsh->divs[0][0].lns));

	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 2, 1, _DIV_H-1, 1, 0);
	wsheet_cutout(wsh, 0, 1, 3, _DIV_H-2);
	wassert(1 == list_size(&wsh->divs[0][0].lns));

	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 1, 1, _DIV_H-1, 1, 0);
	wsheet_cutout(wsh, 0, 2, 3, _DIV_H-2);
	wassert(2 == list_size(&wsh->divs[0][0].lns));

	wsheet_clean(wsh);
	wsheet_add(wsh, 2, 2, _DIV_W-2, _DIV_H-2, 1, 0);
	wsheet_cutout(wsh, 0, 0, _DIV_W-4, _DIV_H-3);
	wassert(1 == list_size(&wsh->divs[0][0].lns));

	wsheet_clean(wsh);
	wsheet_add(wsh, 2, 2, _DIV_W-2, _DIV_H-2, 1, 0);
	wsheet_cutout(wsh, 0, 0, _DIV_W-1, _DIV_H-3);
	wassert(1 == list_size(&wsh->divs[0][0].lns));

	wsheet_clean(wsh);
	wsheet_add(wsh, 2, 2, _DIV_W-2, _DIV_H-2, 1, 0);
	wsheet_cutout(wsh, 3, 3, _DIV_W-3, _DIV_H-3);
	wassert(2 == list_size(&wsh->divs[0][0].lns));

	wsheet_clean(wsh);
	wsheet_add(wsh, 2, 2, _DIV_W*2-2, 3, 1, 0);
	wsheet_cutout(wsh, 0, 0, _DIV_W*2-4, _DIV_H*2-3);
	wassert(0 == list_size(&wsh->divs[0][0].lns)
		&& 1 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[0][2].lns)
		);

	wsheet_clean(wsh);
	wsheet_add(wsh, 2, 2, 3, _DIV_H*2-2, 1, 0);
	wsheet_cutout(wsh, 0, 0, _DIV_W-1, _DIV_H*2-3);
	wassert(0 == list_size(&wsh->divs[0][0].lns)
		&& 1 == list_size(&wsh->divs[1][0].lns)
		);

	wsheet_clean(wsh);
	wsheet_add(wsh, _DIV_W-1, 1, _DIV_W*2+2, _DIV_H*3-2, 1, 0);
	wsheet_cutout(wsh, 3, 4, _DIV_W*2, _DIV_H*2+1);
	wassert(1 == list_size(&wsh->divs[0][0].lns)
		&& 1 == list_size(&wsh->divs[0][1].lns)
		&& 0 == list_size(&wsh->divs[1][0].lns)
		&& 0 == list_size(&wsh->divs[1][1].lns)
		&& 0 == list_size(&wsh->divs[1][2].lns)
		&& 1 == list_size(&wsh->divs[2][1].lns)
		&& 1 == list_size(&wsh->divs[2][2].lns)
		);

	wsheet_clean(wsh);
	wsheet_add(wsh, 1, 1, 2, 1, 1, 0);
	wsheet_add(wsh, 1, 1, 1, _DIV_H-1, 1, 0);
	wsheet_cutout(wsh, 0, 0, 3, 3);
	wassert(1 == list_size(&wsh->divs[0][0].lns));

	printf("passed\n");
	return 0;
}

static int
_test_wsheet(void)
{
	struct wsheet* wsh;
	wsh = wsheet_create();
	wsheet_init(wsh, 20, 10, 10, 10);
	_test_add(wsh);
	_test_find_lines(wsh);
	_test_cutout(wsh);

	wsheet_destroy(wsh);
	wsys_deinit();
	return 0;
}

TESTFN(_test_wsheet, WSHEET)

#endif /* CONFIG_TEST_EXECUTABLE */
