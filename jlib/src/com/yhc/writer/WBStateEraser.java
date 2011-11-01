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

import com.yhc.writer.G2d.Rect;

class WBStateEraser {
	private WBoard		_board = null;

	private int		_ox, _oy;
	private int		_x, _y;
	private final Rect	_moveTrack = new Rect();

	WBStateEraser(WBoard board) {
		WDev.wassert(null != board);
		_board = board;
	}

	Rect track() {
		return _moveTrack;
	}

	void onActionStart(int x, int y) {
		_ox = _x = x;
		_oy = _y = y;
		_moveTrack.setEmpty();
	}

	void onActionEnd() {
		_board.invalidateBoard(_moveTrack.l, _moveTrack.t,
					_moveTrack.r, _moveTrack.b);
		_moveTrack.set(WUtil.min(_ox, _x),
				WUtil.min(_oy, _y),
				WUtil.max(_ox, _x),
				WUtil.max(_oy, _y));
		_board.invalidateBoard(_moveTrack.l, _moveTrack.t,
					_moveTrack.r, _moveTrack.b);
		// we should assume maximum pen thickness to cover all kind of line in this rect.
		_board.fill(_moveTrack.l, _moveTrack.t, _moveTrack.r, _moveTrack.b,
				_board.bgcolor(), WConstants.LIMIT_THICK);
		_board.cutoutSheet(_moveTrack.l, _moveTrack.t,
					_moveTrack.r, _moveTrack.b);
		_board.drawSheet(_moveTrack.l, _moveTrack.t,
					_moveTrack.r, _moveTrack.b);
		_moveTrack.setEmpty();
	}

	void onActionMove(int x, int y) {
		_x = x;
		_y = y;
		_board.invalidateBoard(_moveTrack.l, _moveTrack.t,
					_moveTrack.r, _moveTrack.b);
		_moveTrack.set(WUtil.min(_ox, _x),
				WUtil.min(_oy, _y),
				WUtil.max(_ox, _x),
				WUtil.max(_oy, _y));
		_board.invalidateBoard(_moveTrack.l, _moveTrack.t,
					_moveTrack.r, _moveTrack.b);
	}
}
