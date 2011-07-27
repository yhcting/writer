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

#include "common.h"

int
g2d_splitX(int* out_intersecty,
	   int x0, int y0,
	   int x1, int y1,
	   int x,
	   int yt, int yb) {
	if (x0 == x1)
		return 0;
	else {
		float s = (float)(x - x0) / (float)(x1 - x0);
		if (0 < s && s < 1) {
			int y = _round_off(s * (float)(y1 - y0) + (float)y0);
			if (yt >= y || y >= yb)
				return 0;
			*out_intersecty=y;
			return 1;
		} else
			return 0;
	}
}


int
g2d_splitY(int* out_intersectx,
	   int x0, int y0,
	   int x1, int y1,
	   int y,
	   int xl, int xr) {
	if (y0 == y1)
		return 0;
	else {
		float s = (float)(y - y0) / (float)(y1 - y0);
		if (0 < s && s < 1) {
			int x = _round_off(s * (float)(x1 - x0) + (float)x0);
			if (xl >= x || x >= xr)
				return 0;
			*out_intersectx = x;
			return 1;
		} else
			return 0;
	}
}

/*============================= TEST START =================================*/

#ifdef CONFIG_TEST_EXECUTABLE

#include <assert.h>

void test_g2d(void)
{
	int out;

	ASSERT(0 == g2d_splitX(&out, 1, 1, 5, 1, 1, 0, 5));
	ASSERT(0 == g2d_splitX(&out, 1, 1, 5, 1, 5, 0, 5));
	ASSERT(0 == g2d_splitY(&out, 1, 1, 5, 1, 1, 0, 5));
	ASSERT(0 == g2d_splitX(&out, 1, 1, 5, 1, 0, 0, 5));
	ASSERT(0 == g2d_splitX(&out, 1, 1, 5, 3, 0, 0, 5));
	ASSERT(1 == g2d_splitX(&out, 1, 1, 5, 3, 2, 0, 5));
	ASSERT(0 == g2d_splitX(&out, 1, 1, 5, 3, 5, 0, 3));
	ASSERT(0 == g2d_splitY(&out, 1, 1, 5, 3, 3, 0, 6));
	ASSERT(1 == g2d_splitY(&out, 1, 1, 5, 3, 2, 0, 6));
}

#endif /* CONFIG_TEST_EXECUTABLE */
