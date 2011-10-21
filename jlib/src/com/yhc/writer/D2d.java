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

package com.yhc.writer;

class D2d {

	static void fill(int[] pixels, int w, int h, int color, int l, int t, int r, int b) {
		_nativeFill(pixels, w, h, color, l, t, r, b);
	}

	static void copy(int[] dst, int[] src,
			int dw, int dh, int dl, int dt,
			int sw, int sh, int sl, int st,
			int width, int height) {
		_nativeCopy(dst, src,
				dw, dh, dl, dt,
				sw, sh, sl, st,
				width, height);
	}

	static void drawLine(int[] pixels, int w, int h,
				int color, byte thick,
				int x0, int y0, int x1, int y1) {
		//WDev.wassert(thick < WConstants.LIMIT_THICK);
		_nativeDrawLine(pixels, w, h, color, thick, x0, y0, x1, y1);
	}

	//====== Natives ======
	private static native void _nativeFill(int[] pixels, int w, int h, int color,
						int l, int t, int r, int b);
	private static native void _nativeCopy(int[] dst, int[] src,
						int dw, int dh, int dl, int dt,
						int sw, int sh, int sl, int st,
						int width, int height);
	private static native void _nativeDrawLine(int[] pixels, int w, int h,
							int color, byte thick,
							int x0, int y0, int x1, int y1);

}