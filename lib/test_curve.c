/*****************************************************************************
 *    Copyright (C) 2011 Younghyung Cho. <yhcting77@gmail.com>
 *
 *    This file is part of Writer.
 *
n *    This program is free software: you can redistribute it and/or modify
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
#include <stdint.h>

#include "common.h"
#include "curve.h"
#include "list.h"
#include "test_values.h"

/*
 * @pts   : x0 | y0 | x1 | y1 | ...
 * @nrpts : # of points == (# of items of 'pts') / 2
 */
static struct curve*
_create_curve(const int32_t* pts, int32_t nrpts) {
	struct curve* crv = crv_create(nrpts);
	struct point* cp = crv->pts;
	const int32_t *p, *pend;

	crv->color = 0;
	crv->thick = 0;

	p = pts;
	pend = p + nrpts * 2;
	while (p < pend) {
		cp->x = *p++;
		cp->y = *p++;
		cp++;
	}
	return crv;
}

static void
_test_curve_split_set1(int32_t set1_index, int32_t insz, int32_t outsz) {
	struct curve*    crv;
	struct list_link in;
	struct list_link out;

	list_init_link(&in);
	list_init_link(&out);

	crv = _create_curve(_set1.lns[set1_index], 2);
	crv_split(crv, &in, &out,
		  _set1.l, _set1.t, _set1.r, _set1.b);
	if (insz != list_size(&in)
	    || outsz != list_size(&out)) {
		printf("  [ERR] %d: in(%d), out(%d)\n",
		       set1_index, list_size(&in), list_size(&out));
		wassert(0);
	}
	crv_list_free(&in);
	crv_list_free(&out);
	crv_destroy(crv);
}

static void
_test_curve_split_pts(const int32_t* pts, int32_t nrpts,
		      const struct rect* r,
		      int32_t insz, int32_t outsz) {
	struct curve*    crv;
	struct list_link in;
	struct list_link out;

	list_init_link(&in);
	list_init_link(&out);

	crv = _create_curve(pts, nrpts);
	crv_split(crv, &in, &out, r->l, r->t, r->r, r->b);
	if (insz != list_size(&in)
	    || outsz != list_size(&out)) {
		printf("  [ERR] in(%d), out(%d)\n",
		       list_size(&in), list_size(&out));
		printf("-------- in ---------\n");
		crv_foreach(crv, &in)
			crv_dump(crv);
		printf("\n-------- out ---------\n");
		crv_foreach(crv, &out)
			crv_dump(crv);
		wassert(0);
	}
	crv_list_free(&in);
	crv_list_free(&out);
	crv_destroy(crv);
}

static int
_test_curve() {
	struct rect r;

	_test_curve_split_set1(0, 1, 1);
	_test_curve_split_set1(1, 0, 1);
	_test_curve_split_set1(2, 0, 1);
	_test_curve_split_set1(3, 1, 1);
	_test_curve_split_set1(4, 0, 1);
	_test_curve_split_set1(5, 1, 1);
	_test_curve_split_set1(6, 1, 1);
	_test_curve_split_set1(7, 1, 2);
	_test_curve_split_set1(8, 1, 2);
	_test_curve_split_set1(9, 1, 1);
	_test_curve_split_set1(10, 1, 1);
	_test_curve_split_set1(11, 1, 2);
	_test_curve_split_set1(12, 1, 2);
	_test_curve_split_set1(13, 1, 0);
	_test_curve_split_set1(14, 1, 0);
	_test_curve_split_set1(15, 1, 1);
	_test_curve_split_set1(16, 1, 1);
	_test_curve_split_set1(17, 1, 2);
	_test_curve_split_set1(18, 1, 2);
	_test_curve_split_set1(19, 1, 1);
	_test_curve_split_set1(20, 1, 2);
	_test_curve_split_set1(21, 1, 0);
	_test_curve_split_set1(22, 1, 1);
	_test_curve_split_set1(23, 1, 1);
	_test_curve_split_set1(24, 1, 2);
	_test_curve_split_set1(25, 1, 1);
	_test_curve_split_set1(26, 1, 2);

	rect_set(&r, 10, 10, 20, 20);

	_test_curve_split_pts(_pts0, sizeof(_pts0) / sizeof(_pts0[0]) / 2, &r,
			      1, 2);

	_test_curve_split_pts(_pts1, sizeof(_pts1) / sizeof(_pts1[0]) / 2, &r,
			      1, 2);

	_test_curve_split_pts(_pts3, sizeof(_pts3) / sizeof(_pts3[0]) / 2, &r,
			      1, 2);

	{ /* Just for Scope */
		/*
		 *            +--------+
		 *            |        |
		 *        *   |        |
		 *            |        |
		 *            +--------*
		 *                             *
		 */
		static const int32_t _tp[] = {
			5, 15,
			20, 20,
			25, 23,
		};
		_test_curve_split_pts(_tp, arrsz(_tp) / 2, &r,
				      1, 2);

	}

	{ /* Just for Scope */
		static const int32_t _tp[] = {
			1, 1,
			4, 2,
		};
		rect_set(&r, 2, 1, 3, 8);
		_test_curve_split_pts(_tp, arrsz(_tp) / 2, &r,
				      1, 2);

	}

	{ /* Just for Scope */
		static const int32_t _tp[] = {
			0, 0,
			1, 1,
		};
		rect_set(&r, 0, 0, 10, 10);
		_test_curve_split_pts(_tp, arrsz(_tp) / 2, &r,
				      1, 0);

	}

	return 0;
}

TESTFN(_test_curve, CURVE, TESTPRI_UNIT)

#endif /* CONFIG_TEST_EXECUTABLE */
