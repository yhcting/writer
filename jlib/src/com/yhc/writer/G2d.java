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

class G2d {
	static class Rect {
		int l, t, r, b;

		Rect() {
			set(0, 0, 0, 0);
		}

		Rect(Rect r) {
			set(r.l, r.t, r.b, r.b);
		}

		Rect(int l, int t, int r, int b) {
			set(l, t, r, b);
		}

		void set(int al, int at, int ar, int ab) {
			l = al;
			t = at;
			r = ar;
			b = ab;
		}

		void set(Rect r) {
			set(r.l, r.t, r.r, r.b);
		}

		void setEmpty() {
			set(0, 0, 0, 0);
		}

		void offset(int dx, int dy) {
			set(l + dx, t + dy, r + dx, b + dy);
		}

		boolean isEmpty() {
			return l >= r && t >= b;
		}

		int width() {
			return r - l;
		}

		int height() {
			return b - t;
		}

	}

	static class RectF {
		float l, t, r, b;

		RectF() {
			set(0, 0, 0, 0);
		}

		RectF(RectF r) {
			set(r.l, r.t, r.b, r.b);
		}

		RectF(float l, float t, float r, float b) {
			set(l, t, r, b);
		}

		void set(float al, float at, float ar, float ab) {
			l = al;
			t = at;
			r = ar;
			b = ab;
		}

		void set(RectF r) {
			set(r.l, r.t, r.r, r.b);
		}

		void setEmpty() {
			set(0, 0, 0, 0);
		}

		void offset(float dx, float dy) {
			set(l + dx, t + dy, r + dx, b + dy);
		}

		boolean isEmpty() {
			return l >= r && t >= b;
		}

		float width() {
			return r - l;
		}

		float height() {
			return b - t;
		}
	}

	static class Line {
		int x0, y0, x1, y1, color;
		byte thick;

		Line(int ax0, int ay0, int ax1, int ay1,
			byte athick, int acolor) {
			x0 = ax0; y0 = ay0;
			x1 = ax1; y1 = ay1;
			color = acolor; thick = athick;
		}
	}

	static class Point {
		int x, y;

		Point(int ax, int ay) {
			x = ax;
			y = ay;
		}
	}
}
