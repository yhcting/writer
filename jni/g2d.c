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

#include "common.h"
#include "g2d.h"

int
g2d_intersectX(int32_t* out_intersecty,
	       int32_t x0, int32_t y0,
	       int32_t x1, int32_t y1,
	       int32_t x,
	       int32_t yt, int32_t yb) {
	wassert(yt < yb);
	if (x0 == x1) {
		/* parallel */
		if (x0 == x &&
		    ((yt <= y0 && yb > y0)
		     || (yt < y1 && yb > y1)))
			return 2;
		else
			return 0;
	} else {
		float s = (float)(x - x0) / (float)(x1 - x0);
		if (0 <= s && s < 1) {
			int32_t y =
				_round_off(s * (float)(y1 - y0) + (float)y0);
			if ((x == x1 && y == y1)
			    || yt >= y || y >= yb)
				return 0;
			*out_intersecty=y;
			return 1;
		} else
			return 0;
	}
}


int
g2d_intersectY(int32_t* out_intersectx,
	       int32_t x0, int32_t y0,
	       int32_t x1, int32_t y1,
	       int32_t y,
	       int32_t xl, int32_t xr) {
	wassert(xl < xr);
	if (y0 == y1) {
		/* parallel */
		if (y0 == y &&
		    ((xl <= x0 && xr > x0)
		     || (xl < x1 && xr > x1)))
			return 2;
		else
			return 0;
	} else {
		float s = (float)(y - y0) / (float)(y1 - y0);
		if (0 <= s && s < 1) {
			int32_t x =
				_round_off(s * (float)(x1 - x0) + (float)x0);
			if ((y == y1 && x == x1)
			    || xl >= x || x >= xr)
				return 0;
			*out_intersectx = x;
			return 1;
		} else
			return 0;
	}
}
