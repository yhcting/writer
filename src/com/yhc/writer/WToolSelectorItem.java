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
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.widget.FrameLayout;
import android.widget.ImageView;

class WToolSelectorItem extends FrameLayout implements WSelector.Item {
	private 	boolean 	_bTouched = false;
	private final	ImageView	_iv;
	private final 	String		_clsname;

	// set as public to access 'XML' of 'res'
	public WToolSelectorItem(Context context, AttributeSet attrs) {
		super(context, attrs);
		_clsname = attrs.getAttributeValue(WConstants.XMLNS, "myclass");

		Integer icon = null;
		try {
			icon = (Integer)Class.forName(_clsname).getMethod("getIcon").invoke(null);
		} catch (Exception e) {
			WDev.log(1, e.getMessage());
			WDev.wassert(false);
		}

		_iv = new ImageView(context);
		_iv.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
		_iv.setImageResource(icon.intValue());
		_iv.setAlpha(WConstants.INACTIVE_ICON_ALPHA);
		addView(_iv);
		setClickable(true);
	}


	String getStateClass() {
		return _clsname;
	}

	@Override
	public boolean isContain(int x, int y) {
		return new Rect(getLeft(), getTop(), getRight(), getBottom()).contains(x, y);
	}

	@Override
	public Drawable detail() {
		return null;
	}

	@Override
	public void itemTouched(boolean bTouched) {
		_bTouched = bTouched;
		if (_bTouched)
			_iv.setAlpha(WConstants.ACTIVE_ICON_ALPHA);
		else
			_iv.setAlpha(WConstants.INACTIVE_ICON_ALPHA);
		_iv.invalidate();
	}
}