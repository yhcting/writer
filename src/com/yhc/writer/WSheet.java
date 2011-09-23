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

/****************************************
 * NOTE:
 *   + Line splitting is not supported yet!!
 *     (So, some lines may be belongs to several division.)
 *       <= in android graphic framework, this can be handled smoothly..
 ****************************************/

package com.yhc.writer;

import java.io.IOException;
import java.util.LinkedList;
import java.util.ListIterator;

import android.graphics.Rect;

class WSheet {
	private static final int _MAX_V = 0x0000ffff; // internally, cooridnate value is treated as unsigned short!!

	// to support 64bit host machine.
	private final long _native_sheet;

	private final Rect _boundary = new Rect(0, 0, 0, 0);

	static {
		System.loadLibrary("writer-jni");
	}

	private WSheet() {
		_native_sheet = _nativeCreateWsheet();
	}

	private short rgb32to16(int color) {
		return (short) (((color & 0x00f80000) >> 8) | ((color & 0x0000fc00) >> 5) | ((color & 0x000000f8) >> 3));
	}

	/******************************
	 * APIs
	 ******************************/
	static WSheet createWSheet() {
		return new WSheet();
	}

	int init(int divW, int divH, int colN, int rowN) {
		WDev.wassert(divW * colN < _MAX_V && divH * rowN < _MAX_V);
		_boundary.set(0, 0, divW * colN, divH * rowN);
		return _nativeInitWsheet(_native_sheet, divW, divH, colN, rowN);
	}

	void destroy() {
		_nativeDestroyWsheet(_native_sheet);
	}

	int width() {
		return _boundary.right;
	}

	int height() {
		return _boundary.bottom;
	}

	Rect boundary() {
		return _boundary;
	}

	void cutout(int l, int t, int r, int b) {
		_nativeCutout(_native_sheet, l, t, r, b);
	}

	// @bend : is this end of line sequence? : 0(false), 1(true)
	void addLine(int x0, int y0, int x1, int y1, byte thick, int color, boolean bend) {
		// TODO : change 8888 -> 565
		WDev.wassert(thick < WConstants.LIMIT_THICK);
		_nativeAddLine(_native_sheet, x0, y0, x1, y1,
				thick, rgb32to16(color), bend);
	}

	void addLines(LinkedList<G2d.Line> lines) {
		// TODO
		ListIterator<G2d.Line> iter = lines.listIterator();
		G2d.Line               l;
		while (iter.hasNext()) {
			l = iter.next();
			addLine(l.x0, l.y0, l.x1, l.y1, l.thick, l.color, !iter.hasNext());
		}
	}

	// this is for performance!!!
	// (For better design, this sould not be here!!)
	// zf : zoom factor
	void draw(int[] pixels, int w, int h, int ox, int oy, int l, int t, int r, int b, float zf) {
		_nativeDraw(_native_sheet, pixels, w, h, ox, oy, l, t, r, b, zf);
	}

	void save(String filepath) throws IOException {
		if (0 == _nativeSave(_native_sheet, filepath)) {
			// fail to save
		}
	}

	void load(String filepath) throws IOException {
		if (0 == _nativeLoad(_native_sheet, filepath)) {
			// fail to load
		}
		_boundary.set(0, 0, _nativeWidth(_native_sheet), _nativeHeight(_native_sheet));
	}

	/******************************
	 * Native functions!!
	 ******************************/
	private static native long _nativeCreateWsheet();

	private static native int  _nativeInitWsheet(long native_sheet, int divW, int divH, int colN, int rowN);

	private static native void _nativeDestroyWsheet(long native_sheet);

	private static native int  _nativeWidth(long native_sheet);

	private static native int  _nativeHeight(long native_sheet);

	private static native void _nativeCutout(long native_sheet, int l, int t, int r, int b);

	private static native void _nativeAddLine(long native_sheet, int x0, int y0, int x1, int y1,
							byte thick, short color, boolean bend);

	private static native void _nativeDraw(long native_sheet, int[] pixels,
						int w, int h, int ox, int oy,
						int l, int t, int r, int b,
						float zf);

	private static native int _nativeSave(long native_sheet, String filepath);

	private static native int _nativeLoad(long native_sheet, String filepath);

}