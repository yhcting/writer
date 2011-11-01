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
import android.view.MotionEvent;



class WBStateEraserPlat implements WBStateI {
	private static final String	_TAG	= "WBStateEraser";
	private static final int        _ICON   = R.drawable.eraser;

	private WBStateEraser   _st = null;
	private WBoardPlat      _board = null;

	private final Paint	_p = new Paint();

	// This is used at WToolSectorItem.
	public static int getIcon() {
		return _ICON;
	}

	// set public to access dynamically through 'Class' class
	public WBStateEraserPlat(Object boardplat) {
		_board = (WBoardPlat)boardplat;
		_st = new WBStateEraser(_board.bpi());
		_p.setStrokeWidth(0);
		_p.setStyle(Paint.Style.STROKE);
	}

	@Override
	public String name() {
		return this.getClass().getName();
	}

	@Override
	public boolean onTouch(Object meo) {
		MotionEvent me = (MotionEvent)meo;
		int x = (int) me.getX(), y = (int) me.getY();

		if (x < 0 || y < 0)
			// ignore unexpected input!
			return true;

		switch (me.getAction()) {
		case MotionEvent.ACTION_DOWN: {
			_st.onActionStart(x, y);
		} break;

		case MotionEvent.ACTION_MOVE: {
			_st.onActionMove(x, y);
		} break;

		case MotionEvent.ACTION_UP: {
			_st.onActionEnd();
		} break;

		default:
			;
		}
		return true;
	}

	@Override
	public void draw(Object canvaso) {
		Canvas canvas = (Canvas)canvaso;
		if (!_st.track().isEmpty()) {
			_p.setColor(_board.bgcolor() ^ 0x00ffffff); // invert
			canvas.drawRect(WAL.convertFrom(_st.track()), _p);
		}
	}

}
