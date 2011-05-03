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
import android.os.Handler;
import android.os.Message;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

class WView extends View {
	private static final int _SPEED_TOUCH_INTERVAL = 50; // milliseconds.

	private static final int _MSG_SPEED_TOUCH = 0;

	private boolean _b_speed_touch = false;

	private final Handler _speed_touch = new Handler() {
		// NOTE !!!
		// Should I consider synchronization?????
		// (Let's consider it later!!!!!!)
		// TODO : synchronization???
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case _MSG_SPEED_TOUCH: {
				WView.this._b_speed_touch = false;
			} break;
			}
		}
	};

	WView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	protected boolean onWSpeedTouch() {
		return false;
	}

	@Override
	public boolean onTouchEvent(MotionEvent me) {
		boolean ret = false;
		switch (me.getAction()) {
		case MotionEvent.ACTION_DOWN: {
			Message msg = _speed_touch.obtainMessage(_MSG_SPEED_TOUCH);
			_b_speed_touch = true;
			_speed_touch.sendMessageDelayed(msg, _SPEED_TOUCH_INTERVAL);
		} break;

		case MotionEvent.ACTION_MOVE: {
			// _b_speed_touch = false;
		} break;

		case MotionEvent.ACTION_UP: {
			_speed_touch.removeMessages(_SPEED_TOUCH_INTERVAL);
			if (_b_speed_touch) {
				ret = onWSpeedTouch();
				_b_speed_touch = false;
			}
		} break;
		}
		return ret;
	}
}
