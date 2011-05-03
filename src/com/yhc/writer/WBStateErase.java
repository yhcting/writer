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

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.view.MotionEvent;

class WBStateErase implements WBStateI {
	private static final String	_TAG	= "WBStateErase";
	private static final int	_ICON	= R.drawable.eraser;

	private WBoard		_board = null;
	private int		_ox, _oy;
	private int		_x, _y;
	private final Rect	_moveTrack = new Rect();
	private final Paint	_p = new Paint();

	// set public to access dynamically through 'Class' class
	public static int getIcon() {
		return _ICON;
	}

	// set public to access dynamically through 'Class' class
	public WBStateErase(WBoard board) {
		WDev.wassert(null != board);
		_board = board;
		_p.setStrokeWidth(0);
		_p.setStyle(Paint.Style.STROKE);
	}

	@Override
	public String name() {
		return this.getClass().getName();
	}

	@Override
	public boolean onTouch(MotionEvent me) {
		int x = (int) me.getX(), y = (int) me.getY();

		if (x < 0 || y < 0) {
			return true;
		} // ignore unexpected input!

		switch (me.getAction()) {
		case MotionEvent.ACTION_DOWN: {
			_ox = _x = x;
			_oy = _y = y;
			_moveTrack.setEmpty();
		} break;

		case MotionEvent.ACTION_MOVE: {
			_x = x;
			_y = y;
			_board.invalidateBoard(_moveTrack.left, _moveTrack.top,
						_moveTrack.right, _moveTrack.bottom);
			_moveTrack.set(WUtil.min(_ox, _x),
					WUtil.min(_oy, _y),
					WUtil.max(_ox, _x),
					WUtil.max(_oy, _y));
			_board.invalidateBoard(_moveTrack.left, _moveTrack.top,
						_moveTrack.right, _moveTrack.bottom);
		} break;

		case MotionEvent.ACTION_UP: {
			_board.invalidateBoard(_moveTrack.left, _moveTrack.top,
					_moveTrack.right, _moveTrack.bottom);
			_moveTrack.set(WUtil.min(_ox, _x),
					WUtil.min(_oy, _y),
					WUtil.max(_ox, _x),
					WUtil.max(_oy, _y));
			_board.invalidateBoard(_moveTrack.left, _moveTrack.top,
					_moveTrack.right, _moveTrack.bottom);
			// we should assume maximum pen thickness to cover all kind of line in this rect.
			_board.fill(_moveTrack.left, _moveTrack.top, _moveTrack.right, _moveTrack.bottom,
					_board.bgcolor(), WConstants.LIMIT_THICK);
			_board.cutoutSheet(_moveTrack.left, _moveTrack.top,
					_moveTrack.right, _moveTrack.bottom);
			_board.drawSheet(_moveTrack.left, _moveTrack.top, _moveTrack.right, _moveTrack.bottom);
			_moveTrack.setEmpty();
		} break;

		default:
			;
		}
		return true;
	}

	@Override
	public void draw(Canvas canvas) {
		if (!_moveTrack.isEmpty()) {
			_p.setColor(_board.bgcolor() ^ 0x00ffffff); // invert
			canvas.drawRect(_moveTrack, _p);
		}
	}


}
