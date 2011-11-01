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

import java.io.IOException;
import java.util.LinkedList;
import java.util.ListIterator;

import com.yhc.writer.G2d.Rect;


// PI : Platform Independent.
class WBoard {
	// platform dependent operation.
	interface Plat {
		void invalidatePlatBoard();
		void invalidatePlatBoard(Rect r);
	}


	Plat                    _platboard      = null;
	private WSheet 		_sheet 		= null;
	private	WBStateI	_state		= null;

	private final int	_bgcolor	= 0xffffffff;
	private int[]		_pixels		= null; // just reference

	private int		_sw		= 0,
				_sh		= 0; // screen width, screen height

	// Followings represents UI state..!!
	private Rect		_ar		= new Rect();	// active rect.
	private Rect		_rar		= new Rect();	// root active rect...
	private float		_zov		= 1.0f;		// zoom out value

	// To secure memory space (to avoid continuous 'new' operation)
	private final Rect	_tmpR		= new Rect();

	/*******************************************
	 * Pre-allocated object - to prevent too frequent allocation!! All
	 * should be used with very short life cycle - ex. in one function
	 * scope!!
	 *******************************************/
	private final Rect _rectA	= new Rect();

	// listeners
	private OnActiveRegionMoved_Listener	_activie_region_moved_listener = null;
	private OnStateChanged_Listener		_state_changed_listener = null;

	/******************************
	 * Constants
	 ******************************/

	/******************************
	 * Types
	 ******************************/
	private enum _State {
		PEN,
		ERASE;

		private WBStateI	_inst;
		private String          _clsname;

		_State() {
			_inst = null;
		}

		void init(Object boardplat, String clsname) {
			_clsname = clsname;
			Class[] ctorarg = new Class[1];
			try {
				ctorarg[0] = Object.class;
				_inst = (WBStateI)Class.forName(_clsname).getConstructor(ctorarg).newInstance(boardplat);
			} catch (Exception e) {
				WDev.log(1, e.getMessage());
				WDev.wassert(false);
			}
		}

		String clsname() {
			return _clsname;
		}

		WBStateI getInstance() {
			return _inst;
		}
	}

	/**************************
	 * Listeners
	 **************************/
	interface OnActiveRegionMoved_Listener {
		/**
		 *
		 * @param trigger_owner
		 */
		void onMoved(Object trigger_owner,
				float nleft,	float ntop, float nright, float nbottom);
	}

	interface OnStateChanged_Listener {
		void onChanged(Object trigger_owner, WBStateI state);
	}

	/**************************
	 * Functions
	 **************************/
	WBoard() {}
	WBoard(Plat platboard) {}

	private void _allocateScreenCanvas(int width, int height) {
		_sw = width;
		_sh = height;
		WDev.wassert(null == _pixels);
		_pixels = new int[_sw * _sh];
	}

	// zoom factor
	private float _zf() {
		return 1.0f / _zov;
	}

	private Rect _compensateThickness(Rect r, byte thick) {
		WUtil.expand(r, thick, thick);
		WUtil.adjust(r, _sheet.boundary());
		return r;
	}

	// c2s : canvas to sheet
	private int _c2s(int v) {
		return WUtil.roundOff(_zov * v);
	}

	// s2c : sheet to canvas
	private int _s2c(int v) {
		return WUtil.roundOff(v / _zov);
	}

	void destroy() {
		_sheet.destroy();
	}


	int sheetWidth() {
		return _sheet.width();
	}

	int sheetHeight() {
		return _sheet.height();
	}

	// register functions...
	void registerListener(OnActiveRegionMoved_Listener listener) {
		_activie_region_moved_listener = listener;
	}

	void registerListener(OnStateChanged_Listener listener) {
		_state_changed_listener = listener;
	}

	// screen width
	int sw() {
		return _sw;
	}

	int sh() {
		return _sh;
	}

	Rect ar() {
		return _ar;
	}

	void ar(Rect ar) {
		_ar.set(ar);
	}

	Rect rar() {
		return _rar;
	}

	void rar(Rect rar) {
		_rar.set(rar);
	}

	float zov() {
		//return WUtil.round_off(_zov * WConstants.ZOOM_INIT_VALUE);
		return _zov;
	}

	void zov(float zov) {
		_zov = zov;
	}

	int[] pixels() {
		return _pixels;
	}

	int bgcolor() {
		return _bgcolor;
	}

	WBStateI state() {
		return _state;
	}

	void state(String name) {
		_state = _State.PEN.getInstance(); // default
		for (_State s : _State.values())
			if (name.equals(s.name()))
				_state = s.getInstance();
	}

	WBStateI penState() {
		return _State.PEN.getInstance();
	}

	WBStateI eraseState() {
		return _State.PEN.getInstance();
	}


	void initUiState() {
		_ar.set(0, 0, _sw, _sh);
		_rar.set(_ar);
		_zov = 1.0f;
		setState(this, _State.PEN.getInstance());
		moveActiveRegionTo(this, 0, 0);
	}

	void init(Plat platboard, String pencls, String erasercls) {
		_platboard = platboard;
		_State.PEN.init(platboard, pencls);
		_State.ERASE.init(platboard, erasercls);
		_state = _State.PEN.getInstance();
	}


	void initScreen(int width, int height) {
		if (_rar.isEmpty()) {
			WDev.wassert(null == _pixels);
			// This should not be called more than once!!!
			_allocateScreenCanvas(width, height);
			initUiState();
		} else if (null == _pixels) {
			// usually from restore....
			_allocateScreenCanvas(_rar.width(), _rar.height());
			moveActiveRegionTo(this, _ar.l, _ar.t);
		}
	}


	void setState(Object trigger_owner, String clsname) {
		for (_State s: _State.values())
			if (clsname.equals(s.clsname())) {
				setState(trigger_owner, s.getInstance());
				return;
			}
		WDev.wassert(false);
	}

	void setState(Object trigger_owner, WBStateI state) {
		if (_state != state) {
			_state = state;
			if (null != _state_changed_listener)
				_state_changed_listener.onChanged(trigger_owner, state);
		}
	}

	boolean onTouch(Object o) {
		return _state.onTouch(o);
	}

	void onDraw(Object o) {
		// state specific drawing!
		_state.draw(o);
	}


	/**
	 * Zoom in/out based on center of viewing rectangle!
	 *
	 * @param zov	: zoom value (value based on current value.)
	 */
	boolean zoom(float zov) {
		zov = _zov * zov;
		if (zov < WConstants.ZOOM_MIN_VALUE)
			zov = WConstants.ZOOM_MIN_VALUE;

		if (zov > WConstants.ZOOM_MAX_VALUE)
			zov = WConstants.ZOOM_MAX_VALUE;

		Rect r = _rectA;

		r.set(_rar);
		WUtil.expand(r, zov);

		if (r.width() > _sheet.width())
			return false;
		else {
			_zov = zov;
			_ar.set(r);

			WUtil.adjust(_ar, _sheet.boundary());

			float rw = _ar.width() / 2.0f;
			float rh = _ar.height() / 2.0f;
			_rar.l = WUtil.roundOff(_ar.l + rw - rw / _zov);
			_rar.t = WUtil.roundOff(_ar.t + rh - rh / _zov);
			_rar.r = _rar.l + _sw;
			_rar.b = _rar.t + _sh;

			moveActiveRegionTo(this, _ar.l, _ar.t);

			return true;
		}
	}

	void getInvalidateArea(Rect outR, int left, int top, int right, int bottom, byte thick) {
		outR.set(left, top, right, bottom);
		_compensateThickness(outR, thick);
	}

	void invalidateBoard(int left, int top, int right, int bottom) {
		getInvalidateArea(_tmpR, left, top, right, bottom, (byte)_s2c(WConstants.LIMIT_THICK));
		_platboard.invalidatePlatBoard(_tmpR);
	}

	void cutoutSheet(int left, int top, int right, int bottom) {
		_sheet.cutout(_ar.l + _c2s(left), _ar.t + _c2s(top),
				_ar.l + _c2s(right), _ar.t + _c2s(bottom));
	}

	void createNew(int divW, int divH, int colN, int rowN) {
		if (null != _sheet)
			_sheet.destroy();

		_sheet = WSheet.createWSheet();
		_sheet.init(divW, divH, colN, rowN);
	}


	void saveSheet(String fpath) throws IOException {
		_sheet.save(fpath);
	}

	void loadSheet(String fpath) throws IOException {
		if (null != _sheet)
			_sheet.destroy();

		_sheet = WSheet.createWSheet();
		_sheet.load(fpath);
	}

	void shift(int vx, int vy) {
		// TODO : sliding effect is not implemented yet
		int absx = (vx > 0) ? vx : -vx;
		int absy = (vy > 0) ? vy : -vy;
		int ox = _sw / 4;
		int oy = _sh / 4;
		int dx, dy;

		if (0 == vx) {
			dx = 0;
			dy = (vy > 0) ? oy : -oy;
		} else if (0 == vy) {
			dy = 0;
			dx = (vx > 0) ? ox : -ox;
		} else {
			if (absy * ox > oy * absx) {
				dx = absx * oy / absy;
				dx = (vx > 0) ? dx : -dx;
				dy = (vy > 0) ? oy : -oy;
			} else {
				dy = absy * ox / absx;
				dy = (vy > 0) ? dy : -dy;
				dx = (vx > 0) ? ox : -ox;
			}
		}

		Rect tmpR = _rectA;

		tmpR.set(_ar);
		tmpR.offset(_c2s(dx), _c2s(dy));
		WUtil.adjust(tmpR, _sheet.boundary());

		moveActiveRegionTo(this, tmpR.l, tmpR.t);
	}

	void moveActiveRegion(Object trigger_owner, int dx, int dy) {
		Rect tmpR = _rectA;
		tmpR.set(_ar);
		tmpR.offset(_c2s(dx), _c2s(dy));
		WUtil.adjust(tmpR, _sheet.boundary());

		moveActiveRegionTo(this, tmpR.l, tmpR.t);
	}

	void moveActiveRegionTo(Object trigger_owner, int left, int top) {
		int dx = left - _ar.l;
		int dy = top - _ar.t;

		// we just re-calculate all!!!
		Rect tmpR = _rectA;
		// till now, we only consider "l, t" value.
		D2d.fill(_pixels, _sw, _sh, _bgcolor, 0, 0, _sw, _sh); // erase all.
		tmpR.set(left, top, _ar.r + dx, _ar.b + dy);
		_compensateThickness(tmpR, WConstants.LIMIT_THICK);
		_sheet.draw(_pixels,
				_sw, _sh,
				left, top,
				tmpR.l, tmpR.t,
				tmpR.r, tmpR.b,
				_zf());

		_ar.offset(dx, dy);
		_rar.offset(dx, dy);
		_platboard.invalidatePlatBoard();
		if (null != _activie_region_moved_listener)
			_activie_region_moved_listener.onMoved(
					trigger_owner,
					_ar.l / (float)_sheet.width(),
					_ar.t / (float)_sheet.height(),
					_ar.r / (float)_sheet.width(),
					_ar.b / (float)_sheet.height());
	}

	void updateCurve(LinkedList<G2d.Point> points, byte thick, int color) {
		if (!points.isEmpty()) {
			// Change from board coordinate to sheet coordinate
			ListIterator<G2d.Point> iter = points.listIterator();
			G2d.Point               pt;
			while (iter.hasNext()) {
				pt = iter.next();
				pt.x = _c2s(pt.x) + _ar.l;
				pt.y = _c2s(pt.y) + _ar.t;
			}

			// TODO!!!! ---
			_sheet.addCurve(points, thick, color);
		}
	}

	void drawSheet(int left, int top, int right, int bottom) {
		getInvalidateArea(_tmpR, left, top, right, bottom, WConstants.LIMIT_THICK);
		_sheet.draw(_pixels,
				_sw,
				_sh,
				_ar.l,
				_ar.t,
				_ar.l + _c2s(_tmpR.l),
				_ar.t + _c2s(_tmpR.t),
				_ar.l + _c2s(_tmpR.r),
				_ar.t + _c2s(_tmpR.b),
				_zf());
	}


	/*
	 * Draw interface
	 */
	void drawLine(int x0, int y0, int x1, int y1, byte thick, int color) {
		D2d.drawLine(_pixels,
				_sw, _sh,
				color,
				(byte)_s2c(thick),
				x0, y0,
				x1, y1);
	}

	void fill(int left, int top, int right, int bottom, int bgcolor, byte thick) {
		getInvalidateArea(_tmpR, left, top, right, bottom, thick);
		D2d.fill(_pixels,
			sw(), sh(),
			_bgcolor,
			_tmpR.l, _tmpR.t,
			_tmpR.r, _tmpR.b);
	}


}
