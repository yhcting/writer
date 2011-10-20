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

	void addCurve(LinkedList<G2d.Point> points, byte thick, int color) {
		WDev.wassert(thick < WConstants.LIMIT_THICK);

		// Array format
		// [x0][y0][x1][y1]...
		ListIterator<G2d.Point> iter = points.listIterator();
		G2d.Point               pt;
		int                     i = 0;
		int[]                   arr = new int[points.size() * 2];
		while (iter.hasNext()) {
			pt = iter.next();
			arr[2*i] = pt.x;
			arr[2*i + 1] = pt.y;
			i++;
		}
		_nativeAddCurve(_native_sheet, arr, thick, rgb32to16(color));
		arr = null; // explicitly notify that array is not used anymore
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

	private static native void _nativeAddCurve(long native_sheet, int[] pts, byte thick, short color);

	private static native void _nativeDraw(long native_sheet, int[] pixels,
						int w, int h, int ox, int oy,
						int l, int t, int r, int b,
						float zf);

	private static native int _nativeSave(long native_sheet, String filepath);

	private static native int _nativeLoad(long native_sheet, String filepath);

}