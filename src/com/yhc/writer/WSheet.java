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

import android.graphics.Rect;

class WSheet {
	private static final int _MAX_V = 0x0000ffff; // internally, cooridnate value is treated as unsigned short!!

	private final int _native_sheet;

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

	void add(int x0, int y0, int x1, int y1, byte thick, int color) {
		// TODO : change 8888 -> 565
		WDev.wassert(thick < WConstants.LIMIT_THICK);
		_nativeAdd(_native_sheet, x0, y0, x1, y1, thick, rgb32to16(color));
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
	private static native int _nativeCreateWsheet();

	private static native int _nativeInitWsheet(int native_sheet, int divW, int divH, int colN, int rowN);

	private static native void _nativeDestroyWsheet(int native_sheet);

	private static native int _nativeWidth(int native_sheet);

	private static native int _nativeHeight(int native_sheet);

	private static native void _nativeCutout(int native_sheet, int l, int t, int r, int b);

	private static native void _nativeAdd(int native_sheet, int x0, int y0, int x1, int y1, byte thick, short color);

	private static native void _nativeDraw(int native_sheet, int[] pixels,
						int w, int h, int ox, int oy,
						int l, int t, int r, int b,
						float zf);

	private static native int _nativeSave(int native_sheet, String filepath);

	private static native int _nativeLoad(int native_sheet, String filepath);

}