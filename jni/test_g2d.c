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
#include <assert.h>
#include <stdint.h>

#include "common.h"
#include "g2d.h"
#include "test_values.h"

static void
_line_intersect(void) {
	/*
	 * Notation
	 * --------
	 *   T(top), B(bottom), L(left), R(right)
	 *   O(open point) C(close point)
	 *   < L, T: close / R, B: open >
	 *
	 */

	int32_t i0, i1;

	wassert(1 == line_intersectx(&i0, &i1,
				     1, 1,
				     5, 1,
				     1, 0, 5));
	wassert(0 == line_intersectx(&i0, &i1,
				     1, 1,
				     5, 1,
				     5, 0, 5));
	wassert(2 == line_intersecty(&i0, &i1,
				     1, 1,
				     5, 1,
				     1, 0, 5));
	wassert(0 == line_intersecty(&i0, &i1,
				     1, 1,
				     5, 1,
				     0, 0, 5));
	wassert(0 == line_intersectx(&i0, &i1,
				     1, 1,
				     5, 3,
				     0, 0, 5));
	wassert(1 == line_intersectx(&i0, &i1,
				     1, 1,
				     5, 3,
				     2, 0, 5));
	wassert(0 == line_intersectx(&i0, &i1,
				     1, 1,
				     5, 3,
				     5, 0, 3));
	wassert(0 == line_intersecty(&i0, &i1,
				     1, 1,
				     5, 3,
				     3, 0, 6));
	wassert(1 == line_intersecty(&i0, &i1,
				     1, 1,
				     5, 3,
				     2, 0, 6));
	wassert(1 == line_intersecty(&i0, &i1,
				     0, 18,
				     21, -1,
				     9, INT32_MIN, INT32_MAX));

	wassert(1 == line_intersecty(&i0, &i1,
				     0, 9,
				     21, -1,
				     0, 0, 20));

	wassert(1 == line_intersecty(&i0, &i1,
				     -5, -3,
				     3, 2,
				     0, -21, 1)
		&& i0 >= -21 && i0 < 1);



	/*
	 *
	 *      ------C-------
	 *
	 *               O
	 *
	 */
	wassert(1 == line_intersecty(&i0, &i1,
				     5, 10,
				     8, 15,
				     10, 0, 10));

	/*
	 *
	 *      ------O-------
	 *
	 *               C
	 *
	 */
	wassert(0 == line_intersecty(&i0, &i1,
				     8, 15,
				     5, 10,
				     10, 0, 10));

	/****************************
	 *
	 * Test Parallel
	 * (Due to open/close concept, handling parallel is not easy!)
	 *
	 ****************************/

	/*
	 *         T(3,0)
	 *         |  C(5,1)
	 *         |  |
	 *    (3,4)B  |
	 *            O(5,5)
	 */
	wassert(0 == line_intersectx(&i0, &i1,
				     5, 1,
				     5, 5,
				     3, 0, 4));

	/*
	 *         T(3,0)
	 *         |  C(5,1)
	 *         |  |
	 *         |  |
	 *         |  O(5,5)
	 *         |
	 *         B(3,10)
	 */
	wassert(0 == line_intersectx(&i0, &i1,
				     5, 1,
				     5, 5,
				     3, 0, 10));

	/*
	 * (1,5)          (10,5)
	 *   L---------------R
	 *          C-----O
	 *        (5,7) (7,7)
	 */
	wassert(0 == line_intersecty(&i0, &i1,
				     1, 5,
				     10, 5,
				     7, 5, 7));

	/*
	 *       (1,5)  (4,5)
	 *          C-----O  L---------R
	 *                 (6,5)      (9,5)
	 */
	wassert(0 == line_intersecty(&i0, &i1,
				     1, 5,
				     4, 5,
				     5, 6, 9));

	/*
	 *       (1,5)   (5,5)
	 *          C------OL--------R
	 *                         (10,5)
	 */
	wassert(0 == line_intersecty(&i0, &i1,
				     1, 5,
				     5, 5,
				     5, 5, 10));

	/*
	 *       (1,5)      (6,5)
	 *          C------L--O------R
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     1, 5,
				     6, 5,
				     5, 5, 10)
		&& 5 == i0 && 6 == i1);

	/*
	 *       (1,5)          (8,5)
	 *          C------L------O--R
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     1, 5,
				     8, 5,
				     5, 5, 10)
		&& 5 == i0 && 8 == i1);

	/*
	 *       (1,5)
	 *          C------L---------OR
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     1, 5,
				     10, 5,
				     5, 5, 10)
		&& 5 == i0 && 10 == i1);

	/*
	 *       (1,5)                  (13,5)
	 *          C------L---------R----O
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     1, 5,
				     13, 5,
				     5, 5, 10)
		&& 5 == i0 && 10 == i1);


	/*
	 *                     (7,5)
	 *                 CL----O----R
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     5, 5,
				     7, 5,
				     5, 5, 10)
		&& 5 == i0 && 7 == i1);

	/*
	 *                 CL---------OR
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     5, 5,
				     10, 5,
				     5, 5, 10)
		&& 5 == i0 && 10 == i1);

	/*
	 *                              (13,5)
	 *                 CL---------R----O
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     5, 5,
				     13, 5,
				     5, 5, 10)
		&& 5 == i0 && 10 == i1);

	/*
	 *                  (6,5)(8,5)
	 *                 L--C---O--R
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     6, 5,
				     8, 5,
				     5, 5, 10)
		&& 6 == i0 && 8 == i1);

	/*
	 *                  (6,5)(8,5)
	 *                 L--O---C--R
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     8, 5,
				     6, 5,
				     5, 5, 10)
		&& 8 == i0 && 6 == i1);

	/*
	 *                       (8,5)
	 *                 LO------C--R
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     8, 5,
				     5, 5,
				     5, 5, 10)
		&& 8 == i0 && 5 == i1);

	/*
	 *          (2,5)        (8,5)
	 *            O----L-------C--R
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     8, 5,
				     2, 5,
				     5, 5, 10)
		&& 8 == i0 && 4 == i1);

	/*
	 *                   (7,5)
	 *                 L---O-----CR
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     10, 5,
				     7, 5,
				     5, 5, 10)
		&& 9 == i0 && 7 == i1);

	/*
	 *          (2,5)
	 *            O----L---------CR
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     10, 5,
				     2, 5,
				     5, 5, 10)
		&& 9 == i0 && 4 == i1);

	/*
	 *         (2,5)               (13,5)
	 *           O----L---------R----C
	 *               (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     13, 5,
				     2, 5,
				     5, 5, 10)
		&& 9 == i0 && 4 == i1);

	/*
	 *      (4,5)             (13,5)
	 *        O--L---------R----C
	 *         (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     13, 5,
				     4, 5,
				     5, 5, 10)
		&& 9 == i0 && 4 == i1);

	/*
	 *                        (13,5)
	 *           OL---------R----C
	 *         (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     13, 5,
				     5, 5,
				     5, 5, 10)
		&& 9 == i0 && 5 == i1);

	/*
	 *              (7,5)     (13,5)
	 *           L----O----R----C
	 *         (5,5)     (10,5)
	 */
	wassert(2 == line_intersecty(&i0, &i1,
				     13, 5,
				     7, 5,
				     5, 5, 10)
		&& 9 == i0 && 7 == i1);

	/*
	 *                (9,5)   (13,5)
	 *           L------O--R----C
	 *         (5,5)    (10,5)
	 */
	wassert(0 == line_intersecty(&i0, &i1,
				     13, 5,
				     9, 5,
				     5, 5, 10));

	/*
	 *                       (13,5)
	 *           L--------OR----C
	 *         (5,5)    (10,5)
	 */
	wassert(0 == line_intersecty(&i0, &i1,
				     13, 5,
				     10, 5,
				     5, 5, 10));

	/*
	 *                        (12,5)  (15,5)
	 *           L--------R----O--------C
	 *         (5,5)    (10,5)
	 */
	wassert(0 == line_intersecty(&i0, &i1,
				     15, 5,
				     12, 5,
				     5, 5, 10));
}

static void
_set1_lnset(struct line* l, int i) {
	line_set(l, _set1.lns[i][0], _set1.lns[i][1],
		 _set1.lns[i][2], _set1.lns[i][3]);
}

static void
_rect_intersect(void) {
	struct point ip0, ip1;
	struct rect  r;
	struct line  l;

	rect_set(&r, _set1.l, _set1.t, _set1.r, _set1.b);

	_set1_lnset(&l, 0);
	wassert(3 == rect_intersect_line5(&ip0, &ip1, &r, &l));

	_set1_lnset(&l, 1);
	wassert(0 == rect_intersect_line5(&ip0, &ip1, &r, &l));

	_set1_lnset(&l, 2);
	wassert(0 == rect_intersect_line5(&ip0, &ip1, &r, &l));

	_set1_lnset(&l, 3);
	wassert(3 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 10 == ip0.x && 10 == ip0.y && 15 == ip1.y);

	_set1_lnset(&l, 4);
	wassert(0 == rect_intersect_line5(&ip0, &ip1, &r, &l));

	_set1_lnset(&l, 5);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 10 == ip0.x && 15 == ip0.y);

	_set1_lnset(&l, 6);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 10 == ip0.x && 15 == ip0.y);

	_set1_lnset(&l, 7);
	wassert(2 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 10 == ip0.x && 15 == ip0.y
		&& 20 == ip1.x && 15 == ip1.y);

	_set1_lnset(&l, 8);
	wassert(2 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 19 == ip0.x && 15 == ip0.y
		&& 10 == ip1.x && 15 == ip1.y);

	_set1_lnset(&l, 9);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 15 == ip0.x && 10 == ip0.y);

	_set1_lnset(&l, 10);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 15 == ip0.x && 10 == ip0.y);

	_set1_lnset(&l, 11);
	wassert(2 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 15 == ip0.x && 10 == ip0.y
		&& 15 == ip1.x && 20 == ip1.y);

	_set1_lnset(&l, 12);
	wassert(2 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 15 == ip0.x && 19 == ip0.y
		&& 15 == ip1.x && 10 == ip1.y);

	_set1_lnset(&l, 13);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 10 == ip0.x && 12 == ip0.y);

	_set1_lnset(&l, 14);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 16 == ip0.x && 10 == ip0.y);

	_set1_lnset(&l, 15);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l));

	_set1_lnset(&l, 16);
	wassert(2 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 16 == ip0.x && 10 == ip0.y);

	_set1_lnset(&l, 17);
	wassert(2 == rect_intersect_line5(&ip0, &ip1, &r, &l));

	_set1_lnset(&l, 18);
	wassert(2 == rect_intersect_line5(&ip0, &ip1, &r, &l));

	_set1_lnset(&l, 19);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l));

	_set1_lnset(&l, 20);
	wassert(2 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 19 == ip0.x);

	_set1_lnset(&l, 21);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 10 == ip0.x && 15 == ip0.y);

	_set1_lnset(&l, 22);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 19 == ip0.y);

	_set1_lnset(&l, 23);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 19 == ip0.y);

	_set1_lnset(&l, 24);
	wassert(2 == rect_intersect_line5(&ip0, &ip1, &r, &l)
		&& 19 == ip0.y);

	_set1_lnset(&l, 25);
	wassert(1 == rect_intersect_line5(&ip0, &ip1, &r, &l));

	_set1_lnset(&l, 26);
	wassert(2 == rect_intersect_line5(&ip0, &ip1, &r, &l));
}

int
_test_g2d(void) {
	printf(" + line intersect => ");
	_line_intersect();
	printf("passed\n");

	printf(" + rect intersect => ");
	_rect_intersect();
	printf("passed\n");

	return 0;
}

TESTFN(_test_g2d, G2D, TESTPRI_FUNCTION)

#endif /* CONFIG_TEST_EXECUTABLE */
