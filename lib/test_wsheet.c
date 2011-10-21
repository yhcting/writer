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
#include "test_values.h"
#include "gtype.h"
#include "g2d.h"

#include <stdio.h>
#include <assert.h>


static const int32_t    _DIV_W  = 20;
static const int32_t    _DIV_H  = 10;
static const int32_t    _COL_N  = 10;
static const int32_t    _ROW_N  = 10;

static void
_line_sanity_check(struct wsheet* wsh) {
	; /* do nothing at this moment */
}

static void
_add_line(struct wsheet* wsh,
	  int32_t x0, int32_t y0, int32_t x1, int32_t y1,
	  uint8_t thick, uint16_t color) {
	int32_t pts[4];
	pts[0] = x0;
	pts[1] = y0;
	pts[2] = x1;
	pts[3] = y1;
	wsheet_add_curve(wsh, pts, 2, thick, color);
}

#define _divlsz(r, c)  div_nr_lines(&wsh->divs[r][c])

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
	_add_line(wsh, 1, 1, 2, 2, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}


	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W-1, 2, 0, 1, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	wsheet_clean(wsh);
	_add_line(wsh, 1, 0, 2, _DIV_H-1, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W-1, _DIV_H-1, 0, 0, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	/***************
	 * line some/whole part is out of sheet.
	 ***************/
	wsheet_clean(wsh);
	_add_line(wsh, -5, 1, 3, 1, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	wsheet_clean(wsh);
	_add_line(wsh, -5, -3, 3, 2, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	wsheet_clean(wsh);
	_add_line(wsh, -5, 1, -2, 1, 1, 0);
	if (!(0 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	wsheet_clean(wsh);
	_add_line(wsh, -1, 5, 5, -1, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	wsheet_clean(wsh);
	_add_line(wsh, -1, 15, 5, -1, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 1 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	wsheet_clean(wsh);
	_add_line(wsh, -1, 19, 21, -1, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 1 == _divlsz(0, 1)
	      && 1 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	/***************
	 * extends over two division
	 ***************/
	wsheet_clean(wsh);
	_add_line(wsh, 1, 1, _DIV_W + 1, 2, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 1 == _divlsz(0, 1)
	      && 0 == _divlsz(0, 2)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(0, 2),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W, 2, 1, 1, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(2, 0)
	      && 1 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(1, 0),
		       _divlsz(2, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 1));
		wassert(0);
	}

	wsheet_clean(wsh);
	_add_line(wsh, 2, 1, 1, _DIV_H + 1, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 1 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}

	wsheet_clean(wsh);
	/* _DIV_W >= 2*_DIV_H */
	_add_line(wsh, _DIV_W, _DIV_H, _DIV_W*2, _DIV_H*2, 1, 0);
	if (!(0 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 1 == _divlsz(1, 1)
	      && 0 == _divlsz(2, 1)
	      && 0 == _divlsz(1, 2)
	      && 0 == _divlsz(2, 2)
	      && 0 == _divlsz(3, 2)
	      && 0 == _divlsz(2, 3)
	      && 0 == _divlsz(3, 3)
	      )) {
		printf("(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1),
		       _divlsz(2, 1),
		       _divlsz(1, 2),
		       _divlsz(2, 2),
		       _divlsz(3, 2),
		       _divlsz(2, 3),
		       _divlsz(3, 3));
		wassert(0);
	}

	/***************
	 * extends over three division
	 ***************/
	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W*2+1, 2, 1, 1, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 1 == _divlsz(0, 1)
	      && 1 == _divlsz(0, 2)
	      && 0 == _divlsz(0, 3)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(2, 0)
	      && 0 == _divlsz(3, 0)
	      )) {
		printf("(%d, %d, %d, %d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(0, 2),
		       _divlsz(0, 3),
		       _divlsz(1, 0),
		       _divlsz(2, 0),
		       _divlsz(3, 0));
		wassert(0);
	}
	_line_sanity_check(wsh);



	wsheet_clean(wsh);
	_add_line(wsh, 2, _DIV_H*2+1, 1, 1, 1, 0);
	wassert(1 == _divlsz(0, 0)
		&& 0 == _divlsz(0, 1)
		&& 0 == _divlsz(0, 2)
		&& 0 == _divlsz(0, 3)
		&& 1 == _divlsz(1, 0)
		&& 1 == _divlsz(2, 0)
		&& 0 == _divlsz(3, 0)
		);
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W-1, 1, _DIV_W*2-1, _DIV_H+1, 1, 0);
	wassert(1 == _divlsz(0, 0)
		&& 1 == _divlsz(0, 1)
		&& 0 == _divlsz(1, 0)
		&& 1 == _divlsz(1, 1)
		);
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, 1, _DIV_H-1, _DIV_W+1, _DIV_H+1, 1, 0);
	wassert(1 == _divlsz(0, 0)
		&& 0 == _divlsz(0, 1)
		&& 1 == _divlsz(1, 0)
		&& 1 == _divlsz(1, 1)
		);
	_line_sanity_check(wsh);

	/***************
	 * extends over several division
	 ***************/
	wsheet_clean(wsh);
	_add_line(wsh, 1, _DIV_H-1, _DIV_W*3+1, _DIV_H*2+1, 1, 0);
	wassert(1 == _divlsz(0, 0)
		&& 0 == _divlsz(0, 1)
		&& 0 == _divlsz(0, 2)
		&& 0 == _divlsz(0, 3)
		&& 1 == _divlsz(1, 0)
		&& 1 == _divlsz(1, 1)
		&& 1 == _divlsz(1, 2)
		&& 0 == _divlsz(1, 3)
		&& 0 == _divlsz(2, 0)
		&& 0 == _divlsz(2, 1)
		&& 1 == _divlsz(2, 2)
		&& 1 == _divlsz(2, 3)
		&& 0 == _divlsz(3, 0)
		&& 0 == _divlsz(3, 1)
		&& 0 == _divlsz(3, 2)
		&& 0 == _divlsz(3, 3)
		);
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W-1, 1, _DIV_W*2+1, _DIV_H*6-1, 1, 0);
	wassert(1 == _divlsz(0, 0)
		&& 0 == _divlsz(1, 0)
		&& 0 == _divlsz(2, 0)
		&& 1 == _divlsz(0, 1)
		&& 1 == _divlsz(1, 1)
		&& 1 == _divlsz(2, 1)
		&& 1 == _divlsz(3, 1)
		&& 1 == _divlsz(4, 1)
		&& 1 == _divlsz(5, 1)
		&& 0 == _divlsz(6, 1)
		&& 0 == _divlsz(0, 2)
		&& 0 == _divlsz(1, 2)
		&& 0 == _divlsz(3, 2)
		&& 0 == _divlsz(4, 2)
		&& 1 == _divlsz(5, 2)
		);
	_line_sanity_check(wsh);


	wsheet_clean(wsh);
	_add_line(wsh, 1, _DIV_H*4-1, _DIV_W*2+1, 1, 1, 0);
	wassert(0 == _divlsz(0, 0)
		&& 1 == _divlsz(3, 0)
		&& 1 == _divlsz(2, 0)
		&& 0 == _divlsz(1, 0)
		&& 0 == _divlsz(3, 1)
		&& 1 == _divlsz(2, 1)
		&& 1 == _divlsz(1, 1)
		&& 1 == _divlsz(0, 1)
		&& 1 == _divlsz(0, 2)
		&& 0 == _divlsz(1, 2)
		);
	_line_sanity_check(wsh);


	/***************
	 * Parellel line!!
	 ***************/
	wsheet_clean(wsh);
	_add_line(wsh, 0, 1, 0, 2, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, 1, 0, 3, 0, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W, 1, _DIV_W, 5, 1, 0);
	if (!(0 == _divlsz(0, 0)
	      && 1 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, 1, _DIV_H, 5, _DIV_H, 1, 0);
	if (!(0 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 1 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, 0, 0, _DIV_W-1, 0, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, 0, 0, 0, _DIV_H-1, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}
	_line_sanity_check(wsh);


	wsheet_clean(wsh);
	_add_line(wsh, 0, 5, 0, _DIV_H+5, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 1 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W, 5, _DIV_W, _DIV_H+5, 1, 0);
	if (!(0 == _divlsz(0, 0)
	      && 1 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 1 == _divlsz(1, 1)
	      )) {
		printf("(%d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1));
		wassert(0);
	}
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, 0, 5, 0, _DIV_H*2+5, 1, 0);
	if (!(1 == _divlsz(0, 0)
	      && 0 == _divlsz(0, 1)
	      && 1 == _divlsz(1, 0)
	      && 0 == _divlsz(1, 1)
	      && 1 == _divlsz(2, 0)
	      && 0 == _divlsz(2, 1)
	      )) {
		printf("(%d, %d, %d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1),
		       _divlsz(2, 0),
		       _divlsz(2, 1));
		wassert(0);
	}
	_line_sanity_check(wsh);

	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W, 5, _DIV_W, _DIV_H*2+5, 1, 0);
	if (!(0 == _divlsz(0, 0)
	      && 1 == _divlsz(0, 1)
	      && 0 == _divlsz(1, 0)
	      && 1 == _divlsz(1, 1)
	      && 0 == _divlsz(2, 0)
	      && 1 == _divlsz(2, 1)
	      )) {
		printf("(%d, %d, %d, %d, %d, %d)\n",
		       _divlsz(0, 0),
		       _divlsz(0, 1),
		       _divlsz(1, 0),
		       _divlsz(1, 1),
		       _divlsz(2, 0),
		       _divlsz(2, 1));
		wassert(0);
	}
	_line_sanity_check(wsh);

	printf("passed\n");
	return 0;
}

static int
_nr_lines_draw(struct list_link* hd) {
	int                cnt = 0;
	struct lines_draw* ld;
	lines_draw_foreach(ld, hd)
		cnt += list_size(&ld->lns);
	return cnt;
}

static int
_test_find_lines(struct wsheet* wsh) {
	struct list_link lns;

	printf(" + finding lines => ");
	fflush(stdout);

	wsheet_clean(wsh);
	_add_line(wsh, 1, 1, _DIV_W-1, 1, 1, 0);
	_add_line(wsh, 1, 3, _DIV_W-1, _DIV_H-1, 1, 0);
	_add_line(wsh, 1, _DIV_H-1, _DIV_W-1, _DIV_H-2, 1, 0);

	list_init_link(&lns);
	wsheet_find_lines_draw(wsh, &lns, 0, 0, 1, 1);
	wassert(0 == _nr_lines_draw(&lns));

	lines_draw_free_list_deep(&lns);
	wsheet_find_lines_draw(wsh, &lns, 0, 0, 2, 2);
	wassert(1 == _nr_lines_draw(&lns));

	lines_draw_free_list_deep(&lns);
	wsheet_find_lines_draw(wsh, &lns, 0, 0, 4, 4);
	wassert(2 == _nr_lines_draw(&lns));

	lines_draw_free_list_deep(&lns);
	wsheet_find_lines_draw(wsh, &lns, 1, 1, 2, 2);
	wassert(1 == _nr_lines_draw(&lns));

	lines_draw_free_list_deep(&lns);
	wsheet_find_lines_draw(wsh, &lns, 2, 2, 4, 4);
	wassert(1 == _nr_lines_draw(&lns));

	lines_draw_free_list_deep(&lns);
	wsheet_find_lines_draw(wsh, &lns, 3, 0, 4, _DIV_H);
	wassert(3 == _nr_lines_draw(&lns));

	lines_draw_free_list_deep(&lns);

	printf("passed\n");

	return 0;
}

static int
_test_cutout_lines(struct wsheet* wsh) {
	printf(" + cutting out lines => ");

	wsheet_clean(wsh);
	_add_line(wsh, 1, 1, 3, 3, 1, 0);
	wsheet_cutout_lines(wsh, 0, 0, _DIV_W, _DIV_H);
	wassert(0 == _divlsz(0, 0));

	wsheet_clean(wsh);
	_add_line(wsh, 0, 0, 1, 1, 1, 0);
	wsheet_cutout_lines(wsh, 0, 0, _DIV_W, _DIV_H);
	wassert(0 == _divlsz(0, 0));

	wsheet_clean(wsh);
	_add_line(wsh, 3, 3, _DIV_W, _DIV_H, 1, 0);
	wsheet_cutout_lines(wsh, 0, 0, _DIV_W, _DIV_H);
	wassert(0 == _divlsz(0, 0)
		&& 0 == _divlsz(0, 1)
		&& 0 == _divlsz(1, 0)
		&& 0 == _divlsz(1, 1)
		);

	wsheet_clean(wsh);
	_add_line(wsh, 1, _DIV_H, _DIV_W-1, _DIV_H, 1, 0);
	wsheet_cutout_lines(wsh, 0, 0, _DIV_W, _DIV_H);
	wassert(0 == _divlsz(0, 0)
		&& 1 == _divlsz(1, 0)
		);

	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W, 1, _DIV_W, _DIV_H-1, 1, 0);
	wsheet_cutout_lines(wsh, 0, 0, _DIV_W, _DIV_H);
	wassert(0 == _divlsz(0, 0)
		&& 1 == _divlsz(0, 1)
		);


	wsheet_clean(wsh);
	_add_line(wsh, 1, 1, _DIV_W, _DIV_H, 1, 0);
	wsheet_cutout_lines(wsh, 1, 1, _DIV_W, _DIV_H);
	wassert(0 == _divlsz(0, 0)
		&& 0 == _divlsz(0, 1)
		&& 0 == _divlsz(1, 0)
		&& 0 == _divlsz(1, 1)
		);

	wsheet_clean(wsh);
	_add_line(wsh, 1, 1, _DIV_W-2, 1, 1, 0);
	wsheet_cutout_lines(wsh, 2, 0, _DIV_W-1, 3);
	wassert(1 == _divlsz(0, 0));

	wsheet_clean(wsh);
	_add_line(wsh, 2, 1, _DIV_W-1, 1, 1, 0);
	wsheet_cutout_lines(wsh, 1, 0, _DIV_W-2, 3);
	wassert(1 == _divlsz(0, 0));

	wsheet_clean(wsh);
	_add_line(wsh, 1, 1, _DIV_W-1, 1, 1, 0);
	wsheet_cutout_lines(wsh, 2, 0, _DIV_W-2, 3);
	wassert(2 == _divlsz(0, 0));


	wsheet_clean(wsh);
	_add_line(wsh, 1, 1, 1, _DIV_H-2, 1, 0);
	wsheet_cutout_lines(wsh, 0, 2, 3, _DIV_H-1);
	wassert(1 == _divlsz(0, 0));

	wsheet_clean(wsh);
	_add_line(wsh, 1, 2, 1, _DIV_H-1, 1, 0);
	wsheet_cutout_lines(wsh, 0, 1, 3, _DIV_H-2);
	wassert(1 == _divlsz(0, 0));

	wsheet_clean(wsh);
	_add_line(wsh, 1, 1, 1, _DIV_H-1, 1, 0);
	wsheet_cutout_lines(wsh, 0, 2, 3, _DIV_H-2);
	wassert(2 == _divlsz(0, 0));

	wsheet_clean(wsh);
	_add_line(wsh, 2, 2, _DIV_W-2, _DIV_H-2, 1, 0);
	wsheet_cutout_lines(wsh, 0, 0, _DIV_W-4, _DIV_H-3);
	wassert(1 == _divlsz(0, 0));

	wsheet_clean(wsh);
	_add_line(wsh, 2, 2, _DIV_W-2, _DIV_H-2, 1, 0);
	wsheet_cutout_lines(wsh, 0, 0, _DIV_W-1, _DIV_H-3);
	wassert(1 == _divlsz(0, 0));

	wsheet_clean(wsh);
	_add_line(wsh, 2, 2, _DIV_W-2, _DIV_H-2, 1, 0);
	wsheet_cutout_lines(wsh, 3, 3, _DIV_W-3, _DIV_H-3);
	wassert(2 == _divlsz(0, 0));

	wsheet_clean(wsh);
	_add_line(wsh, 2, 2, _DIV_W*2-2, 3, 1, 0);
	wsheet_cutout_lines(wsh, 0, 0, _DIV_W*2-4, _DIV_H*2-3);
	wassert(0 == _divlsz(0, 0)
		&& 1 == _divlsz(0, 1)
		&& 0 == _divlsz(0, 2)
		);

	wsheet_clean(wsh);
	_add_line(wsh, 2, 2, 3, _DIV_H*2-2, 1, 0);
	wsheet_cutout_lines(wsh, 0, 0, _DIV_W-1, _DIV_H*2-3);
	wassert(0 == _divlsz(0, 0)
		&& 1 == _divlsz(1, 0)
		);

	wsheet_clean(wsh);
	_add_line(wsh, _DIV_W-1, 1, _DIV_W*2+2, _DIV_H*3-2, 1, 0);
	wsheet_cutout_lines(wsh, 3, 4, _DIV_W*2, _DIV_H*2+1);
	wassert(1 == _divlsz(0, 0)
		&& 1 == _divlsz(0, 1)
		&& 0 == _divlsz(1, 0)
		&& 0 == _divlsz(1, 1)
		&& 0 == _divlsz(1, 2)
		&& 1 == _divlsz(2, 1)
		&& 1 == _divlsz(2, 2)
		);

	wsheet_clean(wsh);
	_add_line(wsh, 1, 1, 2, 1, 1, 0);
	_add_line(wsh, 1, 1, 1, _DIV_H-1, 1, 0);
	wsheet_cutout_lines(wsh, 0, 0, 3, 3);
	wassert(1 == _divlsz(0, 0));

	printf("passed\n");

	return 0;
}

#undef _divlsz

static void
_test_draw() {
	struct wsheet*     wsh;
	struct list_link   hd;
	struct lines_draw* ld;
	struct linend*     lnd;
	const int32_t*     pi;

	wsh = wsheet_create();
	wsheet_init(wsh, 30, 20, 1, 1);

	list_init_link(&hd);
	wsheet_add_curve(wsh, _pts0, arrsz(_pts0)/2, 0, 0);
	wsheet_find_lines_draw(wsh, &hd, 0, 0, 30, 20);
	pi = _pts0;
	lines_draw_foreach(ld, &hd) {
		linend_foreach(lnd, &ld->lns) {
			wassert(lnd->ln.p0.x == *pi++);
			wassert(lnd->ln.p0.y == *pi++);
		}
	}
	lines_draw_free_list_deep(&hd);

	wsheet_destroy(wsh);
}


static int
_test_wsheet(void)
{
	struct wsheet* wsh;
	wsh = wsheet_create();
	wsheet_init(wsh, 20, 10, 10, 10);
	_test_add(wsh);
	_test_find_lines(wsh);
	_test_cutout_lines(wsh);
	wsheet_destroy(wsh);

	_test_draw();

	wsys_deinit();
	return 0;
}

TESTFN(_test_wsheet, WSHEET, TESTPRI_SUBSYSTEM)

#endif /* CONFIG_TEST_EXECUTABLE */
