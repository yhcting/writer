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

import android.util.Log;
import android.view.MotionEvent;

public class WBStatePenPlat implements WBStateI {
	private static final String	_TAG	= "WBStatePenPlat";
	private static final int        _ICON   = R.drawable.pen;

	// UI values
	private static final int	_AINR	= 3; // Action Index NR;

	private WBStatePen	_st = null;
	//  Why this flags is required!
	//  We cannot know which point is motioned in multi-touch.
	//  So, changing between multi-points-motions
	//  (ex. 2-point-motion -> 3-points-motion -> 2-points-motion)
	//    may cause unexpected action.
	//  To avoid this, motions only from first 'down' to first 'up', are regarded as 'VALID'
	private boolean		_valid_action = false;
	// To start to save points at first MOVE event (NOT first DOWN event!)
	private int		_nr_downedpt = 0; // number of downed-point.
	private int[]		_px, _py; // previous position
	private int[]		_x,  _y;  // current event position - to avoid too-frequent-'new' operation.

	// set public to access dynamically through 'Class' class
	// This is used at WToolSectorItem.
	public static int getIcon() {
		return _ICON;
	}

	// set public to access dynamically through 'Class' class
	public WBStatePenPlat(Object boardplat) {
		WBoardPlat wbp = (WBoardPlat)boardplat;
		_st = new WBStatePen(wbp.bpi());
		// pre-allocated _x, _y is used to avoid frequent 'new' operation
		_x = new int[_AINR];
		_y = new int[_AINR];

		// To store previous value of point.
		_px= new int[_AINR];
		_py= new int[_AINR];
	}

	void color(int color) {
		_st.color(color);
	}

	void thick(byte thick) {
		_st.thick(thick);
	}

	int color() {
		return _st.color();
	}

	byte thick() {
		return _st.thick();
	}

	@Override
	public String name() {
		return this.getClass().getName();
	}

	@Override
	public boolean onTouch(Object meo) {
		MotionEvent me = (MotionEvent)meo;
		int ai = me.getActionIndex();

		switch (me.getAction()) {
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_POINTER_DOWN:
		case MotionEvent.ACTION_POINTER_2_DOWN:
		case MotionEvent.ACTION_POINTER_3_DOWN: {
			int x, y;
			x = (int)me.getX(ai);
			y = (int)me.getY(ai);
			if (x < 0 || y < 0)
				break; // Ignore for unexpected value.

			if (ai >= _AINR)
				break; // Ignore

			if (0 == _nr_downedpt)
				// This is first down action!
				// So, from now on, actions are valid.
				_valid_action = true;

			_nr_downedpt++;

			if (_nr_downedpt > _AINR)
				// Something wrong in motion event.
				// Fix it up!
				_nr_downedpt = _AINR;

			_px[ai] = x;
			_py[ai] = y;
			_x[ai] = _y[ai] = WConstants.INVALID_COORD_VALUE;

			_st.onActionStart();
		} break;

		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_POINTER_UP:
		case MotionEvent.ACTION_POINTER_2_UP:
		case MotionEvent.ACTION_POINTER_3_UP: {
			_nr_downedpt--;
			if (_nr_downedpt < 0)
				// Something wrong in motion event.
				// Fix it up!
				_nr_downedpt = 0;

			_valid_action = false;
			_x[ai] = _y[ai] = WConstants.INVALID_COORD_VALUE;

			_st.onActionEnd();
		} break;

		case MotionEvent.ACTION_MOVE: {
			if (!_valid_action)
				break; // ignore invalid move action

			// Max is _AINR
			int ptnr = (me.getPointerCount() < _AINR)? me.getPointerCount(): _AINR;

			// Get current (x,y) value.
			for (int i=0; i < ptnr; i++) {
				_x[i] = (int)me.getX(i);
				_y[i] = (int)me.getY(i);
			}

			switch (me.getPointerCount()) {
			case 1: {
				_st.onActionLine(_px[0], _py[0], _x[0], _y[0]);

			} break;

			case 2: {
				// calcurate distance^2
				double d1 = (_px[1] - _px[0]) * (_px[1] - _px[0])
						+ (_py[1] - _py[0]) * (_py[1] - _py[0]);
				double d2 = (_x[1] - _x[0]) * (_x[1] - _x[0])
						+ (_y[1] - _y[0]) * (_y[1] - _y[0]);

				d1 = java.lang.Math.sqrt(d1);
				d2 = java.lang.Math.sqrt(d2);
				_st.onActionZoom((float)(d1/d2));
			} break;

			// Over 3
			default:{
				// Move
				int dx, dy;
				dx = WUtil.averageDelta(_px, _x, ptnr);
				dy = WUtil.averageDelta(_py, _y, ptnr);
				_st.onActionMove(-dx, -dy);
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
	public void draw(Object canvas) {
	}
}
