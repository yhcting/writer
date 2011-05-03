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
import android.graphics.Color;
import android.util.Log;
import android.view.MotionEvent;

class WBStatePen implements WBStateI {
	private static final String	_TAG	= "WBStatePen";
	private static final int	_ICON	= R.drawable.pen;
	// UI values
	private static final int	_AINR	= 3; // Action Index NR;


	private WBoard		_board = null;
	private byte		_thick = 4;
	private int		_color = Color.BLACK; // black

	/*
	 *  Why this flags is required!
	 *  We cannot know which point is motioned in multi-touch.
	 *  So, changing between multi-points-motions
	 *  (ex. 2-point-motion -> 3-points-motion -> 2-points-motion)
	 *    may cause unexpected action.
	 *  To avoid this, motions only from first 'down' to first 'up', are regarded as 'VALID'
	 */
	private boolean		_valid_action = false;
	private int		_nr_downedpt = 0; // number of downed-point.
	private int[]		_px, _py; // previous position
	private int[]		_x,  _y;  // current event position - to avoid too-frequent-'new' operation.

	// set public to access dynamically through 'Class' class
	public static int getIcon() {
		return _ICON;
	}

	@Override
	public String name() {
		return this.getClass().getName();
	}

	// set public to access dynamically through 'Class' class
	public WBStatePen(WBoard board) {
		WDev.wassert(null != board);
		_board = board;

		// pre-allocated _x, _y is used to avoid frequent 'new' operation
		_x = new int[_AINR];
		_y = new int[_AINR];

		// To store previous value of point.
		_px= new int[_AINR];
		_py= new int[_AINR];
	}

	WBStateI thick(byte thick) {
		_thick = thick;
		return this;
	}

	WBStateI color(int color) {
		_color = color;
		return this;
	}

	byte thick() {
		return _thick;
	}

	int color() {
		return _color;
	}

	@Override
	public boolean onTouch(MotionEvent me) {
		int ai = me.getActionIndex();
		//Log.d(_TAG, "AI : " + ai);

		switch (me.getAction()) {
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_POINTER_DOWN:
		case MotionEvent.ACTION_POINTER_2_DOWN:
		case MotionEvent.ACTION_POINTER_3_DOWN: {
			Log.d(_TAG, "AI(" + ai + ") DOWN");
			if (0 == _nr_downedpt) {
				// This is first down action!
				// So, from now on, actions are valid.
				_valid_action = true;
			}
			_nr_downedpt++;

			if (ai >= _AINR)
				break; // Ignore

			if (_nr_downedpt > _AINR) {
				// Something wrong in motion event.
				// Fix it up!
				_nr_downedpt = _AINR;
			}

			_x[ai] = (int)me.getX(ai);
			_y[ai] = (int)me.getY(ai);
			if (_x[ai] < 0 || _y[ai] < 0)
				break; // Ignore for unexpected value.
			_px[ai] = _x[ai];
			_py[ai] = _y[ai];
		} break;

		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_POINTER_UP:
		case MotionEvent.ACTION_POINTER_2_UP:
		case MotionEvent.ACTION_POINTER_3_UP: {
			Log.d(_TAG, "AI(" + ai + ") UP");
			_nr_downedpt--;
			if (_nr_downedpt < 0) {
				// Something wrong in motion event.
				// Fix it up!
				_nr_downedpt = 0;
			}
			_valid_action = false;
		} break;

		case MotionEvent.ACTION_MOVE: {

			if (!_valid_action) break; // ignore invalid move action

			// Max is _AINR
			int ptnr = (me.getPointerCount() < _AINR)? me.getPointerCount(): _AINR;

			//Log.d(_TAG, "PTNR(" + ptnr + ") MOVE");
			// Get current (x,y) value.
			for (int i=0; i < ptnr; i++) {
				_x[i] = (int)me.getX(i);
				_y[i] = (int)me.getY(i);
			}

			switch (me.getPointerCount()) {
			case 1: {
				// Draw with Pen.
				if (_x[0] == _px[0] && _y[0] == _py[0])
					break; // Invalid 'Move'

				_board.addLine(_px[0], _py[0], _x[0], _y[0], thick(), color());
				_board.drawLine(_px[0], _py[0], _x[0], _y[0], thick(), color());
				_board.invalidateBoard(WUtil.min(_x[0], _px[0]),
							WUtil.min(_y[0], _py[0]),
							WUtil.max(_x[0], _px[0]),
							WUtil.max(_y[0], _py[0]));
			} break;

			case 2: {
				// calcurate distance^2
				double d1 = (_px[1] - _px[0]) * (_px[1] - _px[0])
						+ (_py[1] - _py[0]) * (_py[1] - _py[0]);
				double d2 = (_x[1] - _x[0]) * (_x[1] - _x[0])
						+ (_y[1] - _y[0]) * (_y[1] - _y[0]);

				d1 = java.lang.Math.sqrt(d1);
				d2 = java.lang.Math.sqrt(d2);
				_board.zoom((float)(d1/d2));
			} break;

			// Over 3
			default:{
				// Move
				int dx, dy;
				dx = WUtil.averageDelta(_px, _x, ptnr);
				dy = WUtil.averageDelta(_py, _y, ptnr);
				_board.moveActiveRegion(this, -dx, -dy);
			}
			}

			for (int i=0; i < ptnr; i++) {
				_px[i] = _x[i];
				_py[i] = _y[i];
			}
		} break;

		default:
			Log.d(_TAG, "Motion Event : " + me.getAction());
			;
		}
		return true;
	}

	@Override
	public void draw(Canvas canvas) {
	}
}
