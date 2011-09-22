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

#include <assert.h>

#include "common.h"
#include "g2d.h"

static int
_test_g2d(void) {
	int32_t out;

	wassert(1 == g2d_intersectX(&out, 1, 1, 5, 1, 1, 0, 5));
	wassert(0 == g2d_intersectX(&out, 1, 1, 5, 1, 5, 0, 5));
	wassert(2 == g2d_intersectY(&out, 1, 1, 5, 1, 1, 0, 5));
	wassert(0 == g2d_intersectY(&out, 1, 1, 5, 1, 0, 0, 5));
	wassert(0 == g2d_intersectX(&out, 1, 1, 5, 3, 0, 0, 5));
	wassert(1 == g2d_intersectX(&out, 1, 1, 5, 3, 2, 0, 5));
	wassert(0 == g2d_intersectX(&out, 1, 1, 5, 3, 5, 0, 3));
	wassert(0 == g2d_intersectY(&out, 1, 1, 5, 3, 3, 0, 6));
	wassert(1 == g2d_intersectY(&out, 1, 1, 5, 3, 2, 0, 6));

	return 0;
}

TESTFN(_test_g2d, G2D)

#endif /* CONFIG_TEST_EXECUTABLE */
