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

void
copy_rect(int* dst, const int* src,
	  int dw, int dh, int dl, int dt,
	  int sw, int sh, int sl, int st,
	  int width, int height);

void
fill_rect(int* pixels, int w, int h,
	  int color,
	  int l, int t, int r, int b);

void
draw_line(int* pixels, int w, int h,
	  int color, char thick,
	  int x0, int y0, int x1, int y1);


static inline int
_rbg16to32(short color) {
	return 0xff000000
		| ((color & 0xf800) << 8)
		| ((color & 0x07e0) << 5)
		| ((color & 0x001f) << 3);
}

#endif /* _D2d_h_ */
