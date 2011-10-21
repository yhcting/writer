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

// ====================== Interface Description ========================
// One touch
//	-> Draw
// Two touch
//	-> Zoom in/out
// Three touch.
//	-> move

package com.yhc.writer;

import java.io.IOException;
import java.util.LinkedList;
import java.util.ListIterator;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

class WBoard extends View {

	/******************************
	 * members
	 ******************************/
	private static final	String	_TAG  		= "WBoard";
	private static final	Paint	_default_paint 	= new Paint();

	private enum _State {
		PEN	(WBStatePen.class.getName()),
		ERASE	(WBStateErase.class.getName());

		private WBStateI	_inst;
		private String          _clsname;

		_State(String clsname) {
			_clsname = clsname;
			_inst = null;
		}

		void init(WBoard board) {
			Class[] ctorarg = new Class[1];
			try {
				ctorarg[0] = WBoard.class;
				_inst = (WBStateI)Class.forName(_clsname).getConstructor(ctorarg).newInstance(board);
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

	private	WBStateI	_state		= null;
	private WSheet 		_sheet 		= null;

	private final int	_bgcolor	= Color.WHITE;
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
	// 6 is optimized value for dualcore -- see jni sources!
	private static final int _DIVISION_SIZE_FACTOR = 6;

	// Key values!!!!!!
	private static final String _KEY_AR	= "key_ar";
	private static final String _KEY_RAR	= "key_rar";
	private static final String _KEY_ZOV	= "key_zov";
	private static final String _KEY_STATE	= "key_state";

    /******************************
     * Types
     ******************************/

	// Listeners..
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
     * Local Functions
     **************************/
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

	/**************************
	 * Overriding.
	 **************************/
	@Override
	public boolean onTouchEvent(MotionEvent me) {
		return _state.onTouch(me);
	}

	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);

		if (0 != _sw && 0 != _sh) {
			Rect clip = canvas.getClipBounds();
			canvas.drawBitmap(_pixels,
					canvas.getWidth() * clip.top + clip.left,
					canvas.getWidth(),
					clip.left,
					clip.top,
					clip.right - clip.left,
					clip.bottom - clip.top,
					false,
					_default_paint);

			// state specific drawing!
			_state.draw(canvas);
		}
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			if (_rar.isEmpty()) {
				WDev.wassert(null == _pixels);
				// This should not be called more than once!!!
				_allocateScreenCanvas(getWidth(), getHeight());
				initUiState();
			} else if (null == _pixels) {
				// usually from restore....
				_allocateScreenCanvas(_rar.width(),
							_rar.height());
				moveActiveRegionTo(this, _ar.left, _ar.top);
			}
		}
	}

	/**************************
	 * APIs.
	 **************************/
	// set as public to access 'XML' of 'res'
	public WBoard(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	void initBoard() {
		for (_State s : _State.values())
			s.init(this);
		_state = _State.PEN.getInstance();
	}

	void destroy() {
		_sheet.destroy();
	}

	WBoard penColor(int color) {
		((WBStatePen)_State.PEN.getInstance()).color(color);
		return this;
	}

	WBoard penThick(byte thick) {
		((WBStatePen)_State.PEN.getInstance()).thick(thick);
		return this;
	}

	WBStateI state() {
		return _state;
	}

	int penColor() {
		return ((WBStatePen)_State.PEN.getInstance()).color();
	}

	byte penThick() {
		return ((WBStatePen)_State.PEN.getInstance()).thick();
	}

	int sheetWidth() {
		return _sheet.width();
	}

	int sheetHeight() {
		return _sheet.height();
	}

	// register functions...
	WBoard registerListener(OnActiveRegionMoved_Listener listener) {
		_activie_region_moved_listener = listener;
		return this;
	}

	WBoard registerListener(OnStateChanged_Listener listener) {
		_state_changed_listener = listener;
		return this;
	}


	// screen width
	int sw() {
		return _sw;
	}

	int sh() {
		return _sh;
	}

	int bgcolor() {
		return _bgcolor;
	}

	// -------------------------
	// Selecting tool
	// --------------------------
	void setState(Object trigger_owner, String clsname) {
		for (_State s: _State.values()) {
			if (clsname.equals(s.clsname())) {
				setState(trigger_owner, s.getInstance());
				return;
			}
		}
		WDev.wassert(false);
	}

	void setState(Object trigger_owner, WBStateI state) {
		if (_state != state) {
			_state = state;
			if (null != _state_changed_listener) {
				_state_changed_listener.onChanged(trigger_owner, state);
			}
		}
	}

	void initUiState() {
		_ar.set(0, 0, _sw, _sh);
		_rar.set(_ar);
		_zov = 1.0f;
		setState(this, _State.PEN.getInstance());
		moveActiveRegionTo(this, 0, 0);
	}

	float getZoomOutValue() {
		//return WUtil.round_off(_zov * WConstants.ZOOM_INIT_VALUE);
		return _zov;
	}

	/**
	 * Zoom in/out based on center of viewing rectangle!
	 *
	 * @param zov	: zoom value (value based on current value.)
	 */
	boolean zoom(float zov) {
		zov = _zov * zov;
		if (zov < WConstants.ZOOM_MIN_VALUE) zov = WConstants.ZOOM_MIN_VALUE;
		if (zov > WConstants.ZOOM_MAX_VALUE) zov = WConstants.ZOOM_MAX_VALUE;



		Rect r = _rectA;

		r.set(_rar);
		WUtil.expand(r, zov);

		if (r.width() > _sheet.width()) {
			return false;
		} else {
			_zov = zov;
			_ar.set(r);

			WUtil.adjust(_ar, _sheet.boundary());

			float rw = _ar.width() / 2.0f;
			float rh = _ar.height() / 2.0f;
			_rar.left = WUtil.roundOff(_ar.left + rw - rw / _zov);
			_rar.top = WUtil.roundOff(_ar.top + rh - rh / _zov);
			_rar.right = _rar.left + _sw;
			_rar.bottom = _rar.top + _sh;

			moveActiveRegionTo(this, _ar.left, _ar.top);

			return true;
		}
	}

	void getInvalidateArea(Rect outR, int left, int top, int right, int bottom, byte thick) {
		outR.set(left, top, right, bottom);
		_compensateThickness(outR, thick);
	}

	void invalidateBoard(int left, int top, int right, int bottom) {
		getInvalidateArea(_tmpR, left, top, right, bottom, (byte)_s2c(WConstants.LIMIT_THICK));
		invalidate(_tmpR);
	}

	/*
	void invalidateBoard(int left, int top, int right, int bottom, byte thick) {
		getInvalidateArea(_tmpR, left, top, right, bottom, (byte)_s2c(thick));
		invalidate(_tmpR);
	}
	*/

	void cutoutSheet(int left, int top, int right, int bottom) {
		_sheet.cutout(_ar.left + _c2s(left), _ar.top + _c2s(top),
				_ar.left + _c2s(right), _ar.top + _c2s(bottom));
	}

	void createNew(int width, int height) {
		// Internally, we use "unsigned short" to save coordinate!!!
		WDev.wassert(width * _DIVISION_SIZE_FACTOR < 0x0000ffff && height * _DIVISION_SIZE_FACTOR < 0x0000ffff);

		if (null != _sheet) {
			_sheet.destroy();
		}

		_sheet = WSheet.createWSheet();

		// NOTE
		// Writer supports only "Landscape" mode!!!
		// get display size to set appropriate division size.
		DisplayMetrics dm = new DisplayMetrics();
		((WindowManager) getContext()
					.getSystemService(Context.WINDOW_SERVICE))
					.getDefaultDisplay()
					.getMetrics(dm);

		int laxis, saxis;
		if (dm.widthPixels > dm.heightPixels) {
			laxis = dm.widthPixels;
			saxis = dm.heightPixels;
		} else {
			laxis = dm.heightPixels;
			saxis = dm.widthPixels;
		}

		_sheet.init(laxis / _DIVISION_SIZE_FACTOR,
			saxis / _DIVISION_SIZE_FACTOR,
			width * _DIVISION_SIZE_FACTOR,
			height * _DIVISION_SIZE_FACTOR);
	}

	void saveState(Bundle out) {
		out.putParcelable(_KEY_AR, _ar);
		out.putParcelable(_KEY_RAR, _rar);
		out.putFloat(_KEY_ZOV, _zov);
		out.putString(_KEY_STATE, _state.name());
	}

	void loadState(Bundle in) {
		_ar = in.getParcelable(_KEY_AR);
		_rar = in.getParcelable(_KEY_RAR);
		_zov = in.getFloat(_KEY_ZOV);
		String name = in.getString(_KEY_STATE);

		_state = _State.PEN.getInstance(); // default
		for (_State s : _State.values())
			if (name.equals(s.name()))
				_state = s.getInstance();
	}

	void saveSheet(String fpath) throws IOException {
		_sheet.save(fpath);
	}

	void loadSheet(String fpath) throws IOException {
		if (null != _sheet) {
			_sheet.destroy();
		}

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

		moveActiveRegionTo(this, tmpR.left, tmpR.top);
	}

	void moveActiveRegion(Object trigger_owner, int dx, int dy) {
		Rect tmpR = _rectA;
		tmpR.set(_ar);
		tmpR.offset(_c2s(dx), _c2s(dy));
		WUtil.adjust(tmpR, _sheet.boundary());

		moveActiveRegionTo(this, tmpR.left, tmpR.top);
	}

	void moveActiveRegionTo(Object trigger_owner, int left, int top) {
		int dx = left - _ar.left;
		int dy = top - _ar.top;

		// we just re-calculate all!!!
		Rect tmpR = _rectA;
		// till now, we only consider "l, t" value.
		D2d.fill(_pixels, _sw, _sh, _bgcolor, 0, 0, _sw, _sh); // erase all.
		tmpR.set(left, top, _ar.right + dx, _ar.bottom + dy);
		_compensateThickness(tmpR, WConstants.LIMIT_THICK);
		_sheet.draw(_pixels,
				_sw, _sh,
				left, top,
				tmpR.left, tmpR.top,
				tmpR.right, tmpR.bottom,
				_zf());

		_ar.offset(dx, dy);
		_rar.offset(dx, dy);
		invalidate();
		if (null != _activie_region_moved_listener) {
			_activie_region_moved_listener.onMoved(
					trigger_owner,
					_ar.left / (float) _sheet.width(),
					_ar.top / (float) _sheet.height(),
					_ar.right / (float) _sheet.width(),
					_ar.bottom / (float) _sheet.height());
		}
	}

	void updateCurve(LinkedList<G2d.Point> points, byte thick, int color) {
		if (!points.isEmpty()) {
			// Change from board coordinate to sheet coordinate
			ListIterator<G2d.Point> iter = points.listIterator();
			G2d.Point               pt;
			while (iter.hasNext()) {
				pt = iter.next();
				pt.x = _c2s(pt.x) + _ar.left;
				pt.y = _c2s(pt.y) + _ar.top;
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
				_ar.left,
				_ar.top,
				_ar.left + _c2s(_tmpR.left),
				_ar.top + _c2s(_tmpR.top),
				_ar.left + _c2s(_tmpR.right),
				_ar.top + _c2s(_tmpR.bottom),
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
			_tmpR.left, _tmpR.top,
			_tmpR.right, _tmpR.bottom);
	}

}
