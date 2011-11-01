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
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import com.yhc.writer.G2d.Rect;
import com.yhc.writer.G2d.RectF;

class WMiniMap extends View {
	private int _sw = 0, _sh = 0;
	private final RectF _nar = new RectF(); // normalized active rect
	private final RectF _rectFA = new RectF(); // space to use to avoid too frequent new/GC.
	private final Paint _paint;
	private OnActiveRegionMoved_Listener _activie_region_moved_listener = null;

	interface OnActiveRegionMoved_Listener {
		/**
		 *
		 * @param trigger_owner
		 */
		void onMoved(Object trigger_owner, float nleft, float ntop);
	}

	// set as public to access 'XML' of 'res'
	public WMiniMap(Context context, AttributeSet attrs) {
		super(context, attrs);
		_paint = new Paint();
		_paint.setColor(Color.YELLOW);
		_paint.setAlpha(WConstants.ACTIVE_ICON_ALPHA);
		_paint.setStyle(Paint.Style.FILL);
	}

	private float _normalizeX(float x) {
		return x / _sw;
	}

	private float _normalizeY(float y) {
		return y / _sh;
	}

	void moveActiveRegionTo(Object trigger_owner, float nleft, float ntop, float nright, float nbottom) {
		WDev.wassert(nleft < nright && ntop < nbottom
				&& ntop >= 0 && nleft >= 0
				&& nright <= 1 && nbottom <= 1);
		_nar.set(nleft, ntop, nright, nbottom);
		invalidate();
		if (null != _activie_region_moved_listener)
			_activie_region_moved_listener.onMoved(trigger_owner, nleft, ntop);
	}

	/**
	 * register listener for changed by user(user only!)
	 *
	 * @param listener
	 * @return
	 */
	WMiniMap registerListener(OnActiveRegionMoved_Listener listener) {
		_activie_region_moved_listener = listener;
		return this;
	}

	/**
	 *
	 * @param nWidth
	 *                : normalized active width
	 * @return
	 */
	@Override
	protected void onLayout(boolean changed, int l, int t, int r, int b) {
		super.onLayout(changed, l, t, r, b);
		if (changed)
			if (0 == _sw && 0 == _sh) {
				// This should not be called more than once!!!
				_sw = r - l;
				_sh = b - t;
			}
	}

	@Override
	public boolean onTouchEvent(MotionEvent me) {
		switch (me.getAction()) {
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_MOVE: {
			float x = _normalizeX(me.getX()), y = _normalizeY(me.getY());
			float w = _nar.width(), h = _nar.height();
			RectF nr = _rectFA;
			nr.l = x - w / 2;
			nr.t = y - h / 2;
			nr.r = nr.l + w;
			nr.b = nr.t + h;
			WUtil.adjust(nr);
			WDev.wassert(nr.l < nr.r && nr.t < nr.b);
			if (null != _activie_region_moved_listener)
				_activie_region_moved_listener.onMoved(this, nr.l, nr.t);
			_nar.set(nr);
		} break;

		case MotionEvent.ACTION_UP: {
		} break;

		default:
		}
		return true;
	}

	@Override
	protected void onDraw(Canvas canvas) {
		Rect rect = new Rect((int) (_nar.l * _sw),
					(int) (_nar.t * _sh),
					(int) (_nar.r * _sw + 1),
					(int) (_nar.b * _sh + 1));
		canvas.drawRect(WAL.convertFrom(rect), _paint);
	}
}