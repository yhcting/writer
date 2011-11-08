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

import java.util.LinkedList;


class WBStatePen {
	static final int ACT_INVALID	= -1;
	static final int ACT_CURVE	= 0;
	static final int ACT_ZMV	= 1;

	private WBoard		_board = null;
	private byte		_thick = 1;
	private int		_color = 0xff000000;
	private int		_act =	ACT_INVALID;

	private boolean         _b_first_move = true;
	private LinkedList<G2d.Point> _points	= new LinkedList<G2d.Point>();

	WBStatePen(WBoard board) {
		WDev.wassert(null != board);
		_board = board;
	}

	void thick(byte thick) {
		_thick = thick;
	}

	void color(int color) {
		_color = color;
	}

	byte thick() {
		return _thick;
	}

	int color() {
		return _color;
	}

	void onActionStart(int act) {
		WDev.wassert(ACT_INVALID <= act && act <= ACT_ZMV);
		_act = act;
		switch(act) {
		case ACT_CURVE:
			_board.curveStart();
		break;
		case ACT_ZMV:
			//_board.zmvStart();
		break;
		default:
			WDev.wassert(false);
		}
		_b_first_move = true;
	}

	void onActionEnd() {
		if (_points.size() > 1)
			_board.updateCurve(_points, thick(), color());

		_points.clear();
		_b_first_move = true;
		switch(_act) {
		case ACT_CURVE:
			_board.curveEnd();
		break;
		case ACT_ZMV:
			//_board.zmvEnd();
		break;
		default:
			WDev.wassert(false);
		}
		_act = ACT_INVALID;
	}

	void onActionLine(int x0, int y0, int x1, int y1) {
		// Draw with Pen.
		if (x0 == x1 && y0 == y1)
			return; // Invalid 'Move'

		if (_b_first_move)
			_points.addLast(new G2d.Point(x0, y0));
		_points.addLast(new G2d.Point(x1, y1));
		_b_first_move = false;

		_board.drawLine(x0, y0, x1, y1, thick(), color());
		_board.invalidateBoard(WUtil.min(x1, x0),
					WUtil.min(y1, y0),
					WUtil.max(x1, x0),
					WUtil.max(y1, y0));

	}

	void onActionZoom(float ratio) {
		_board.zoom(ratio);
	}

	void onActionMove(int dx, int dy) {
		_board.moveActiveRegion(this, dx, dy);
	}
}
