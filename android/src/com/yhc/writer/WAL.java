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

// WAL (Writer Abstraction Layer)
// This is abstraction layer between Android platform and java and native library.

package com.yhc.writer;

import android.graphics.Rect;

class WAL {
	static Rect convertFrom(G2d.Rect r) {
		return new Rect(r.l, r.t, r.r, r.b);
	}

	static G2d.Rect convertTo(Rect r) {
		return new G2d.Rect(r.left, r.top, r.right, r.bottom);
	}
}
