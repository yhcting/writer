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

#ifndef _D2d_h_
#define _D2d_h_

#include <stdint.h>

void
copy_rect(int32_t* dst, const int32_t* src,
	  int32_t dw, int32_t dh, int32_t dl, int32_t dt,
	  int32_t sw, int32_t sh, int32_t sl, int32_t st,
	  int32_t width, int32_t height);

void
fill_rect(int32_t* pixels, int32_t w, int32_t h,
	  int32_t color,
	  int32_t l, int32_t t, int32_t r, int32_t b);

void
draw_line(int32_t* pixels, int32_t w, int32_t h,
	  int32_t color, uint8_t thick,
	  int32_t x0, int32_t y0, int32_t x1, int32_t y1);


static inline int
_rbg16to32(int16_t color) {
	return 0xff000000
		| ((color & 0xf800) << 8)
		| ((color & 0x07e0) << 5)
		| ((color & 0x001f) << 3);
}

#endif /* _D2d_h_ */
