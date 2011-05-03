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

import android.graphics.Rect;
import android.graphics.RectF;

class WUtil {
	static float min(float x, float y) {
		return (x > y) ? y : x;
	}

	static float max(float x, float y) {
		return (x > y) ? x : y;
	}

	static int min(int x, int y) {
		return (x > y) ? y : x;
	}

	static int max(int x, int y) {
		return (x > y) ? x : y;
	}

	static final void adjust(Rect nr, int l, int t, int r, int b) {
		if (nr.left < l)
			nr.left = l;

		if (nr.top < t)
			nr.top = t;

		if (nr.right > r)
			nr.right = r;

		if (nr.bottom > b)
			nr.bottom = b;
	}

	/**
	 *
	 * @param nr	: normalized rectangle (in/out)
	 * @return
	 */
	static final void adjust(Rect nr, Rect boundary) {
		WDev.wassert(nr.width() <= boundary.width() && nr.height() <= boundary.height());
		adjust(nr, boundary.left, boundary.top, boundary.right, boundary.bottom);
	}

	static final void expand(Rect r, int dx, int dy) {
		r.left -= dx;
		r.right += dx;
		r.top -= dy;
		r.bottom += dy;
	}

	static final void expand(Rect r, float ratio) {
		float wf = (r.width()) * ratio;
		float hf = (r.height()) * ratio;
		float ox = ((r.left + r.right)) / 2.0f;
		float oy = ((r.top + r.bottom)) / 2.0f;
		r.left	= WUtil.roundOff(ox - wf / 2.0f);
		r.top	= WUtil.roundOff(oy - hf / 2.0f);
		r.right	= r.left + WUtil.roundOff(wf);
		r.bottom= r.top + WUtil.roundOff(hf);
	}

	static final int roundOff(float v) {
		return (v > 0) ? (int) (v + .5f) : (int) (v - .5f);
	}

	/**
	 *
	 * @param nr	: normalized rect
	 */
	static final void adjust(RectF nr) {
		if (nr.left < 0)
			nr.left = 0;

		if (nr.top < 0)
			nr.top = 0;

		if (nr.right > 1.0f)
			nr.right = 1.0f;

		if (nr.bottom > 1.0f)
			nr.bottom = 1.0f;
	}
	/**
	 * @param ppt	: previous values
	 * @param pt	: current values
	 * @param nr	: number of values to consider.
	 * @return 	: average delta
	 */
	static final int averageDelta(int[] pv, int[] v, int nr) {
		int dsum = 0;
		for (int i=0; i < nr; i++)
			dsum += v[i] - pv[i];
		return dsum/nr;
	}
}