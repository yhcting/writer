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

import android.content.Context;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.RelativeLayout;

/*********************
 * ASSUMPTION!!!
 *   "selector_item_root" is direct child of selector!!!
 */

class WSelector extends RelativeLayout {
	/******************************
	 * Constants
	 ******************************/
	private final int _MODE_TAB	= 0;
	private final int _MODE_SLIDE	= 1;

	/******************************
	 * members
	 ******************************/
	private View _cover	= null;
	private View _ov	= null; // original view (original pressed view) - '_pv' before touch down action
	private View _pv	= null; // pressed view
	private OnSelected_Listener	_selected_listener = null;
	private OnChanged_Listener	_changed_listener = null;
	private ViewGroup		_item_root = null;

	private final View.OnTouchListener _mode_tab = new View.OnTouchListener() {
		public boolean onTouch(View view, MotionEvent me) {
			View v = _find((int) me.getX(), (int) me.getY());
			// Nothing changed if nothing is touched.

			switch (me.getAction()) {
			case MotionEvent.ACTION_DOWN: {
				if (null == v)
					return false;

				_touch(_ov, false);
				_touch(_pv, false);
				_ov = _pv = v;
				_touch(v, true);
				if (null != _selected_listener)
					_selected_listener.onSelect(v);
			} break;
			// 'break' is missing intentionally!!
			case MotionEvent.ACTION_MOVE: {
				; // do nothing at this mode.
			} break;

			case MotionEvent.ACTION_UP: {
				; // do nothing at this mode
			} break;

			default:
				;
			}
			return true;
		}
	};

	private final View.OnTouchListener _mode_slide = new View.OnTouchListener() {
		public boolean onTouch(View view, MotionEvent me) {
			View v = _find((int) me.getX(), (int) me.getY());

			switch (me.getAction()) {
			case MotionEvent.ACTION_DOWN: {
				if (null == v)
					return true;

				_ov = _pv;
				_touch(_pv, false);
				_pv = null;
			}
				// 'break' is missing intentionally!!
			case MotionEvent.ACTION_MOVE: {
				if (null == v)
					return true;

				if (v != _pv) {
					_touch(_pv, false);
					_touch(v, true);
					View from = _pv;
					_pv = v;
					if (null != _changed_listener)
						_changed_listener.onChange(from, v);
				}
			} break;

			case MotionEvent.ACTION_UP: {
				// _pv is selected one !!!
				if (null != _selected_listener)
					_selected_listener.onSelect(_pv);
			} break;

			default:
				;
			}
			return true;
		}
	};

	private View.OnTouchListener _mode = _mode_slide;

	/******************************
	 * Types
	 ******************************/
	interface Item {
		/**
		 * check whether this item contains point(x,y). [NOTE!] (x,y)
		 * should be based on parent's coordinate.
		 *
		 * @return
		 */
		boolean isContain(int x, int y);

		// get item detail view
		Drawable detail();

		void itemTouched(boolean bTouched);
	}

	// on select is called
	interface OnSelected_Listener {
		void onSelect(View v);
	}

	/**
	 * from, to can be null
	 */
	interface OnChanged_Listener {
		void onChange(View from, View to);
	}

	/**************************
	 * Local Functions
	 **************************/
	/**
	 * _Find View that contains (x,y)
	 */
	private View _find(int x, int y) {
		// See assumption above!!!!
		int dx = _cover.getLeft() - _item_root.getLeft();
		int dy = _cover.getTop() - _item_root.getTop();

		// x, y is relative value based on cover..
		// So, we should compensate it!
		x += dx;
		y += dy;

		int N = _item_root.getChildCount();
		View v;
		Rect r = new Rect();
		for (int i = N - 1; i >= 0; i--) {
			v = _item_root.getChildAt(i);
			WDev.wassert(null != v);
			if (_cover == v)
				continue; // ignore ghost view

			r.set(v.getLeft(), v.getTop(), v.getRight(), v.getBottom());
			if (r.contains(x, y)) {
				if (v.isClickable()) {
					if (v instanceof Item)
						return ((Item) v).isContain(x, y) ? v : null;
					else
						return v;
				} else
					return null;
			}
		}
		return null;
	}

	/**
	 * _Find View index.
	 */
	private int _find(View in) {
		if (null == in)
			return -1;

		int N = _item_root.getChildCount();
		View v;
		for (int i = N - 1; i >= 0; i--) {
			v = _item_root.getChildAt(i);
			if (in == v)
				return i;
		}
		return -1;
	}

	private void _touch(View v, boolean bValue) {
		if (null != v && v instanceof Item) {
			((Item) v).itemTouched(bValue);
			v.invalidate();
		}
	}

	/**************************
	 * Overriding.
	 **************************/

	/**************************
	 * APIs.
	 **************************/

	// set as public to access 'XML' of 'res'
	public WSelector(Context context, AttributeSet attrs) {
		super(context, attrs);

		int mode = attrs.getAttributeIntValue(WConstants.XMLNS, "selector_mode", -1);
		switch (mode) {
		case _MODE_TAB: {
			_mode = _mode_tab;
		} break;
		case _MODE_SLIDE: {
			_mode = _mode_slide;
		} break;
		default:
			_mode = _mode_tab;
		}
	}

	/**
	 * @return
	 */
	boolean init() {
		_cover = new View(getContext());
		_cover.setLayoutParams(new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
		_cover.setOnTouchListener(_mode);
		addView(_cover);

		_item_root = (ViewGroup) findViewById(R.id.selector_item_root);
		WDev.wassert(null != _cover && null != _item_root);
		return (null != _item_root) && (null != _cover);
	}

	OnSelected_Listener register_onSelected(OnSelected_Listener listener) {
		OnSelected_Listener sv = _selected_listener;
		_selected_listener = listener;
		return sv;
	}

	OnChanged_Listener register_onChanged(OnChanged_Listener listener) {
		OnChanged_Listener sv = _changed_listener;
		_changed_listener = listener;
		return sv;
	}

	/**
	 * @return : pressed item before touch down action.(original pressed
	 *         item before "ACTION")
	 */
	boolean isIn(View item) {
		return (0 > _find(item))? false: true;
	}

	View originalItem() {
		return _ov;
	}

	View pressed() {
		return _pv;
	}

	View detail(View item) {
		if (null == item || !isIn(item))
			return null;
		else {
			if (item instanceof Item && null != ((Item) item).detail()) {
				ImageView iv = new ImageView(getContext());
				iv.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,
						LayoutParams.WRAP_CONTENT));
				iv.setImageDrawable(((Item) item).detail());
				return iv;
			} else
				return null;
		}
	}

	int	getItemNR() {
		return _item_root.getChildCount();
	}

	Item getItem(int i) {
		return (Item)_item_root.getChildAt(i);
	}

	boolean touch(int at) {
		View v = _item_root.getChildAt(at);
		return (null != v)? touch(v): false;
	}

	boolean touch(View item) {
		if (!isIn(item))
			return false;
		else if (item != _pv) {
			_touch(_pv, false);
			_pv = item;
			_touch(_pv, true);
		}
		return true;
	}

	boolean select(int at) {
		View v = _item_root.getChildAt(at);
		return (null != v)? select(v): false;
	}

	boolean select(View item) {
		if (touch(item)) {
			WDev.wassert(item == _pv);
			if (null != _pv && null != _selected_listener)
				_selected_listener.onSelect(_pv);
			return true;
		} else
			return false;
	}
}