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

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.Bundle;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

import com.yhc.writer.G2d.Rect;

class WBoardPlat extends View implements WBoard.Plat {

	/******************************
	 * members
	 ******************************/
	private static final	String	_TAG  		= "WBoard";
	private static final	Paint	_default_paint 	= new Paint();
	// 6 is optimized value for dualcore -- see jni sources!
	private static final 	int 	_DIVISION_SIZE_FACTOR = 6;

	// Key values!!!!!!
	private static final 	String 	_KEY_AR		= "key_ar";
	private static final 	String 	_KEY_RAR	= "key_rar";
	private static final 	String 	_KEY_ZOV	= "key_zov";
	private static final 	String 	_KEY_STATE	= "key_state";

	private WBoard          _bpi; // Board Platform Independent
	private OnBoardPlat_Listener _bpl;
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

	// Initialization of board is done.
	interface OnBoardPlat_Listener {
		void onInitialized();
	}

	/**************************
	 * Implements interfaces.
	 **************************/
	public void invalidatePlatBoard() {
		invalidate();
	}

	public void invalidatePlatBoard(Rect r) {
		invalidate(WAL.convertFrom(r));
	}

	public void invalidateBoard(int left, int top, int right, int bottom) {
		_bpi.invalidateBoard(left, top, right, bottom);
	}
	/**************************
	 * Overriding.
	 **************************/
	@Override
	public boolean onTouchEvent(MotionEvent me) {
		return _bpi.onTouch(me);
	}

	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);

		if (0 != _bpi.sw() && 0 != _bpi.sh()) {
			android.graphics.Rect clip = canvas.getClipBounds();
			canvas.drawBitmap(_bpi.pixels(),
					canvas.getWidth() * clip.top + clip.left,
					canvas.getWidth(),
					clip.left,
					clip.top,
					clip.right - clip.left,
					clip.bottom - clip.top,
					false,
					_default_paint);

			_bpi.onDraw(canvas);
		}
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			_bpi.initScreen(getWidth(), getHeight());
			_bpl.onInitialized();
		}
	}

	/**************************
	 * APIs.
	 **************************/
	// set as public to access 'XML' of 'res'
	public WBoardPlat(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	void createNew(int width, int height) {
		// Internally, we use "unsigned short" to save coordinate!!!
		WDev.wassert(width * _DIVISION_SIZE_FACTOR < 0x0000ffff && height * _DIVISION_SIZE_FACTOR < 0x0000ffff);
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

		_bpi.createNew(laxis / _DIVISION_SIZE_FACTOR,
				saxis / _DIVISION_SIZE_FACTOR,
				width * _DIVISION_SIZE_FACTOR,
				height * _DIVISION_SIZE_FACTOR);
	}

	void destroy() {
		_bpi.destroy();
	}

	void initBoard(OnBoardPlat_Listener l) {
		_bpi = new WBoard();
		_bpi.init(this, WBStatePenPlat.class.getName(), WBStateEraserPlat.class.getName());
		_bpl = l;
	}


	// ================================
	// !!! Delegation !!!
	// ================================

	WBoard bpi() {
		return _bpi;
	}

	void registerListener(final OnActiveRegionMoved_Listener listener) {
		_bpi.registerListener(new WBoard.OnActiveRegionMoved_Listener() {
			@Override
			public void onMoved(Object trigger_owner, float nleft, float ntop, float nright, float nbottom) {
				listener.onMoved(trigger_owner, nleft, ntop, nright, nbottom);
			}
		});
	}

	void registerListener(final OnStateChanged_Listener listener) {
		_bpi.registerListener(new WBoard.OnStateChanged_Listener() {
			@Override
			public void onChanged(Object trigger_owner, WBStateI state) {
				listener.onChanged(trigger_owner, state);
			}
		});
	}

	int sheetWidth() {
		return _bpi.sheetWidth();
	}

	int sheetHeight() {
		return _bpi.sheetHeight();
	}
	void penColor(int color) {
		((WBStatePenPlat)_bpi.penState()).color(color);
	}

	void penThick(byte thick) {
		((WBStatePenPlat)_bpi.penState()).thick(thick);
	}

	int bgcolor() {
		return _bpi.bgcolor();
	}
	int penColor() {
		return ((WBStatePenPlat)_bpi.penState()).color();
	}

	byte penThick() {
		return ((WBStatePenPlat)_bpi.penState()).thick();
	}

	boolean zoom(float zov) {
		return _bpi.zoom(zov);
	}

	WBStateI state() {
		return _bpi.state();
	}

	void initUiState() {
		_bpi.initUiState();
	}

	void setState(Object trigger_owner, String statecls) {
		_bpi.setState(trigger_owner, statecls);
	}

	void moveActiveRegion(Object trigger_owner, int dx, int dy) {
		_bpi.moveActiveRegion(trigger_owner, dx, dy);
	}

	void moveActiveRegionTo(Object trigger_owner, int left, int top) {
		_bpi.moveActiveRegionTo(trigger_owner, left, top);
	}

	void updateCurve(LinkedList<G2d.Point> points, byte thick, int color) {
		_bpi.updateCurve(points, thick, color);
	}

	void drawLine(int x0, int y0, int x1, int y1, byte thick, int color) {
		_bpi.drawLine(x0, y0, x1, y1, thick, color);
	}

	void fill(int left, int top, int right, int bottom, int bgcolor, byte thick){
		_bpi.fill(left, top, right, bottom, bgcolor, thick);
	}

	void drawSheet(int left, int top, int right, int bottom) {
		_bpi.drawSheet(left, top, right, bottom);
	}

	void cutoutSheet(int left, int top, int right, int bottom) {
		_bpi.cutoutSheet(left, top, right, bottom);
	}

	void saveSheet(String fpath) throws IOException {
		_bpi.saveSheet(fpath);
	}

	void loadSheet(String fpath) throws IOException {
		_bpi.loadSheet(fpath);
	}

	// =================================
	// !!! Platform Specific !!!
	// =================================

	void saveState(Bundle out) {
		out.putParcelable(_KEY_AR, WAL.convertFrom(_bpi.ar()));
		out.putParcelable(_KEY_RAR, WAL.convertFrom(_bpi.rar()));
		out.putFloat(_KEY_ZOV, _bpi.zov());
		out.putString(_KEY_STATE, _bpi.state().name());
	}

	void loadState(Bundle in) {
		android.graphics.Rect r = new android.graphics.Rect();
		r = in.getParcelable(_KEY_AR);
		_bpi.ar(WAL.convertTo(r));
		r = in.getParcelable(_KEY_RAR);
		_bpi.rar(WAL.convertTo(r));
		_bpi.zov(in.getFloat(_KEY_ZOV));
		_bpi.state(in.getString(_KEY_STATE));
	}
}
