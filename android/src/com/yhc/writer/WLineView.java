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

class WLineView extends View {
	private 	byte	_thick = 0;
	private final	Paint	_paint = new Paint();

	private void _init() {
		_paint.setStrokeWidth(0);
	}

	public WLineView(Context context, AttributeSet attrs) {
		super(context, attrs);
		_init();
	}

	public WLineView(Context context) {
		super(context);
		_init();
	}

	WLineView color(int color) {
		_paint.setColor(color);
		return this;
	}

	WLineView thick(byte thick) {
		_thick = thick;
		return this;
	}

	int color() {
		return _paint.getColor();
	}

	byte thick() {
		return _thick;
	}

	@Override
	protected void onDraw(Canvas canvas) {
		int sx = getWidth() / 5;// 20% - start x
		int ex = getWidth() - sx; // end x
		int my = getHeight() / 2; // middle y

		canvas.drawLine(sx, my, ex, my, _paint);
		for (int i = 1; i <= _thick; i++) {
			canvas.drawLine(sx, my - i, ex, my - i, _paint);
			canvas.drawLine(sx, my + i, ex, my + i, _paint);
		}

	}
}