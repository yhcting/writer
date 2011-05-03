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
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.widget.FrameLayout;

class WThickSelectorItem extends FrameLayout implements WSelector.Item {
	private static final int _COLOR = Color.WHITE;

	private		boolean _bTouched = false;
	private final	byte	_index;
	private final	Paint	_paint = new Paint();

	// set as public to access 'XML' of 'res'
	public WThickSelectorItem(Context context, AttributeSet attrs) {
		super(context, attrs);
		_index = (byte)attrs.getAttributeIntValue(WConstants.XMLNS, "index", -1);
		WDev.wassert(_index >= 0);

		WLineView lv = new WLineView(context);
		lv.setLayoutParams(new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
		addView(lv);

		setClickable(true);

		lv.color(_COLOR).thick(_index);

		_paint.setStrokeWidth(0); // 1pixel
		_paint.setColor(_COLOR);
	}

	public boolean isContain(int x, int y) {
		return new Rect(getLeft(), getTop(), getRight(), getBottom()).contains(x, y);
	}

	public Drawable detail() {
		return null;
	}

	public void itemTouched(boolean bTouched) {
		_bTouched = bTouched;
		if (_bTouched)
			setBackgroundColor(Color.MAGENTA);
		else
			setBackgroundColor(Color.TRANSPARENT);
	}

	byte thick() {
		return _index;
	}

	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);
	}
}