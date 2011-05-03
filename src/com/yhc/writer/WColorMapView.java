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

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

class WColorMapView extends View {

	private static final int 	_RES = 256;
	private static final int[] 	_color_map = _createMap();

	// pre-allocated paint.
	private final Paint _paint = new Paint();

	private static int[] _createMap() {
		int[] map = new int[_RES * _RES];
		for (int i = 0; i < _RES * _RES; i++) {
			map[i] = 0xff000000 | ((i & 0xf800) << 8)
					| ((i & 0x07e0) << 5)
					| ((i & 0x001f) << 3);
		}
		return map;
	}

	public WColorMapView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	int color(int x, int y) {
		if (y < 0)
			y = 0;
		if (y >= _RES)
			y = _RES - 1;
		if (x < 0)
			x = 0;
		if (x >= _RES)
			x = _RES - 1;
		return _color_map[y * _RES + x];
	}

	@Override
	protected void onLayout(boolean changed, int l, int t, int r, int b) {
		super.onLayout(changed, l, t, r, b);
		WDev.wassert((r - l) == _RES && (b - t) == _RES);
	}

	@Override
	protected void onDraw(Canvas canvas) {
		canvas.drawBitmap(_color_map,
				0, _RES, 0, 0,
				_RES, _RES, false, _paint);
	}

}