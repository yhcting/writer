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
import com.yhc.writer.G2d.RectF;

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
		WDev.wassert(nr.width() < (r - l) && nr.height() < (b - t));
		int w = nr.width();
		int h = nr.height();
		if (nr.l < l) {
			nr.l = l;
			nr.r = l + w;
		}

		if (nr.t < t) {
			nr.t = t;
			nr.b = t + h;
		}

		if (nr.r > r) {
			nr.r = r;
			nr.l = r - w;
		}

		if (nr.b > b) {
			nr.b = b;
			nr.t = b - h;
		}
		WDev.wassert(nr.l >= l && nr.t >= t && nr.r <= r && nr.b <= b);
	}

	/**
	 *
	 * @param nr	: normalized rectangle (in/out)
	 * @return
	 */
	static final void adjust(Rect nr, Rect boundary) {
		WDev.wassert(nr.width() <= boundary.width() && nr.height() <= boundary.height());
		adjust(nr, boundary.l, boundary.t, boundary.r, boundary.b);
	}

	static final void expand(Rect r, int dx, int dy) {
		r.l -= dx;
		r.r += dx;
		r.t -= dy;
		r.b += dy;
	}

	static final void expand(Rect r, float ratio) {
		float wf = (r.width()) * ratio;
		float hf = (r.height()) * ratio;
		float ox = ((r.l + r.r)) / 2.0f;
		float oy = ((r.t + r.b)) / 2.0f;
		r.l	= WUtil.roundOff(ox - wf / 2.0f);
		r.t	= WUtil.roundOff(oy - hf / 2.0f);
		r.r	= r.l + WUtil.roundOff(wf);
		r.b= r.t + WUtil.roundOff(hf);
	}

	static final int roundOff(float v) {
		return (v > 0) ? (int) (v + .5f) : (int) (v - .5f);
	}

	/**
	 *
	 * @param nr	: normalized rect
	 */
	static final void adjust(RectF nr) {
		float w = nr.width();
		float h = nr.height();
		if (nr.l < 0) {
			nr.l = 0;
			nr.r = w;
		}

		if (nr.t < 0) {
			nr.t = 0;
			nr.b = h;
		}

		if (nr.r > 1.0f) {
			nr.r = 1.0f;
			nr.l = nr.r - w;
		}

		if (nr.b > 1.0f) {
			nr.b = 1.0f;
			nr.t = nr.b - h;
		}
		WDev.wassert(nr.l >=0 && nr.t >= 0 && nr.r <= 1.0f && nr.b <= 1.0f);
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