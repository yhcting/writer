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

import java.io.File;
import java.io.IOException;

import android.app.Activity;
import android.app.Dialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

public class WriterMain extends Activity {
	/** Called when the activity is first created. */

	/******************************
	 * members
	 ******************************/

	/******************************
	 * Constants
	 ******************************/
	private static final String _WSHEET_DIRECTORY		= "/sdcard/wsheet/";
	private static final String _WSHEET_EXTENSION		= ".wst";
	private static final String _WSHEET_TEMP_SAVE_FILE	= "/tmp/" + ".__wsheet_tmp__";

	// Setting item's key name
	private static final String _PEN_COLOR		= "pen_color";
	private static final String _PEN_THICK		= "pen_thick";
	private static final String _BOARD_WIDTH	= "board_width";
	private static final String _BOARD_HEIGHT	= "board_height";

	private static final int _REQ_OPEN = 0;

	// runtime data
	private String _last_saved	= null;
	private int _wboard_size	= 0;
	private int _hboard_size	= 0;

	// pre-calculated/assigned. vaule.
	private WBoard		_board;

	/******************************
	 * Types
	 ******************************/

	/******************************
	 * Local Functions
	 **************************/
	private void _prepare() {
		File d = new File(_WSHEET_DIRECTORY);
		if (!d.exists()) {
			d.mkdirs();
		}
	}

	private void _saveSheet(String fpath) {
		try {
			_board.saveSheet(fpath);
		} catch (IOException e) {

		} catch (Exception e) {
			WDev.wassert(false);
		}
	}

	private void _loadSheet(String fpath) {
		try {
			_board.loadSheet(fpath);
		} catch (IOException e) {

		}
	}

	// ====================================
	// * Handling UI for pen select dialog
	// ====================================
	private void _onPenSelect() {
		final Dialog d = new Dialog(this);
		d.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
		d.setContentView(R.layout.pen_select_diag);

		final WSelector ws = (WSelector) d.findViewById(R.id.pen_selector);
		ws.init();

		ws.register_onChanged(new WSelector.OnChanged_Listener() {
			public void onChange(View from, View to) {
				if (null == to) {
					_touchTool(ws, from.getClass().getName());
					ws.touch(from);
				} else {
					((WLineView) d.findViewById(R.id.line_preview)).thick(((WThickSelectorItem)to).thick());
					((WLineView) d.findViewById(R.id.line_preview)).invalidate();
				}
			}
		});

		ws.register_onSelected(new WSelector.OnSelected_Listener() {
			public void onSelect(View v) {
				if (null != v) {
					((WLineView) d.findViewById(R.id.line_preview)).thick(((WThickSelectorItem) v).thick());
					((WLineView) d.findViewById(R.id.line_preview)).invalidate();
				}
			}
		});

		// =================================
		// * Setup color selector
		// =================================
		((WColorMapView) d.findViewById(R.id.colormap)).setOnTouchListener(new View.OnTouchListener() {
			public boolean onTouch(View v, MotionEvent event) {
				switch (event.getAction()) {
				case MotionEvent.ACTION_DOWN:
				case MotionEvent.ACTION_MOVE:
				case MotionEvent.ACTION_UP:
					int ex = (int) event.getX();
					int ey = (int) event.getY();

					if (ex < 0)
						ex = 0;

					if (ex >= v.getWidth())
						ex = v.getWidth() - 1;

					if (ey < 0)
						ey = 0;

					if (ey >= v.getHeight())
						ey = v.getHeight() - 1;

					WColorMapView wcmv = (WColorMapView) v;
					((WLineView) d.findViewById(R.id.line_preview))
							.color(wcmv.color((int) (event.getX()), (int) (event.getY())));
					((WLineView) d.findViewById(R.id.line_preview)).invalidate();
					break;
				}
				return true;
			}
		});

		{ // Just scope
			// reflect current state
			final WLineView wlv = ((WLineView) d.findViewById(R.id.line_preview));
			wlv.color(_board.penColor());
			wlv.thick(_board.penThick());
			wlv.invalidate();

			ws.touch(_board.penThick());
		}

		// =================================
		// * Button operation
		// =================================
		((Button) d.findViewById(R.id.cancel)).setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				// nothing happenned!!
				d.dismiss();
			}
		});
		((Button) d.findViewById(R.id.ok)).setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				WLineView wlv = (WLineView) (d.findViewById(R.id.line_preview));
				WriterMain.this._onPenSelected(wlv.color(), wlv.thick());
				d.dismiss();
			}
		});
		d.show();
	}

	// =================================
	// * Handling "open" dialog
	// =================================
	private void _onDiagOpen() {
		// see WFolderView to know about key...of extra bundle.
		Intent i = new Intent(Intent.ACTION_MAIN)
				.setData(Uri.parse("file://" + _WSHEET_DIRECTORY))
				.putExtra("extention", _WSHEET_EXTENSION)
				.setClassName(this, "com.yhc.writer.WFolderView");
		startActivityForResult(i, _REQ_OPEN);
	}

	// =================================
	// * Handling "Save" dialog
	// =================================
	private void _onDiagSave() {
		final Dialog d = new Dialog(this);
		d.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
		d.setContentView(R.layout.save_diag);
		((Button) d.findViewById(R.id.cancel)).setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				d.dismiss();
			}
		});

		if (null != _last_saved) {
			((EditText) d.findViewById(R.id.edit)).setText(_last_saved);
		}

		((Button) d.findViewById(R.id.ok)).setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				final EditText et = (EditText) d.findViewById(R.id.edit);
				if (et.getText().toString().length() <= 0) {
					Toast.makeText(WriterMain.this, R.string.empty_filename, Toast.LENGTH_SHORT)
						.show();
					return;
				}

				final String fname = _WSHEET_DIRECTORY + et.getText().toString() + _WSHEET_EXTENSION;
				File f = new File(fname);
				if (f.exists()) {
					// ===========================
					// Overwrite dialog!!!
					final Dialog owdiag = new Dialog(WriterMain.this);
					owdiag.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
					owdiag.setContentView(R.layout.overwrite_diag);

					((Button) owdiag.findViewById(R.id.cancel)).setOnClickListener(new View.OnClickListener() {
						public void onClick(View v) {
							owdiag.dismiss();
						}
					});

					((Button) owdiag.findViewById(R.id.ok)).setOnClickListener(new View.OnClickListener() {
						public void onClick(View v) {
							_saveSheet(fname);
							WriterMain.this._last_saved = et.getText().toString();
							owdiag.dismiss();
							d.dismiss();
						}
					});

					owdiag.show();
				} else {
					_saveSheet(fname);
					WriterMain.this._last_saved = et.getText().toString();
					d.dismiss();
				}

			}
		});

		d.show();
	}

	// =================================
	// * Handling "New" dialog
	// =================================
	private void _onDiagNew_checkSize(ImageButton plus, ImageButton minus, int size) {
		if (size >= WConstants.MAX_BOARD_SIZE) {
			plus.setClickable(false);
			plus.setAlpha(WConstants.INACTIVE_ICON_ALPHA);
			minus.setClickable(true);
			minus.setAlpha(255);
		} else if (size <= WConstants.MIN_BOARD_SIZE) {
			plus.setClickable(true);
			plus.setAlpha(255);
			minus.setClickable(false);
			minus.setAlpha(WConstants.INACTIVE_ICON_ALPHA);
		} else {
			plus.setClickable(true);
			plus.setAlpha(255);
			minus.setClickable(true);
			minus.setAlpha(255);
		}
	}

	private void _onDiagNew() {
		final Dialog d = new Dialog(this);
		d.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
		d.setContentView(R.layout.new_diag);

		// * Set listeners
		final TextView wtv = (TextView) d.findViewById(R.id.wtext);
		final TextView htv = (TextView) d.findViewById(R.id.htext);
		final ImageButton wplus = (ImageButton) d.findViewById(R.id.wplus);
		final ImageButton wminus = (ImageButton) d.findViewById(R.id.wminus);
		final ImageButton hplus = (ImageButton) d.findViewById(R.id.hplus);
		final ImageButton hminus = (ImageButton) d.findViewById(R.id.hminus);

		{ // Just scope
			// Get from preference!!
			SharedPreferences settings = getSharedPreferences(WConstants.PREF_NAME, 0);
			_wboard_size = settings.getInt(_BOARD_WIDTH, WConstants.DEFAULT_BOARD_SIZE);
			_hboard_size = settings.getInt(_BOARD_HEIGHT, WConstants.DEFAULT_BOARD_SIZE);
		} // Just scope

		wtv.setText(String.valueOf(_wboard_size));
		htv.setText(String.valueOf(_hboard_size));

		_onDiagNew_checkSize(wplus, wminus, _wboard_size);
		_onDiagNew_checkSize(hplus, hminus, _hboard_size);

		// =============================================
		// * Set increase/decrease button operation. - START
		wplus.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				_wboard_size++;
				wtv.setText(String.format("%02d", _wboard_size));
				_onDiagNew_checkSize(wplus, wminus, _wboard_size);
			}
		});

		wminus.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				_wboard_size--;
				wtv.setText(String.format("%02d", _wboard_size));
				_onDiagNew_checkSize(wplus, wminus, _wboard_size);
			}
		});

		hplus.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				_hboard_size++;
				htv.setText(String.format("%02d", _hboard_size));
				_onDiagNew_checkSize(hplus, hminus, _hboard_size);
			}
		});

		hminus.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				_hboard_size--;
				htv.setText(String.format("%02d", _hboard_size));
				_onDiagNew_checkSize(hplus, hminus, _hboard_size);
			}
		});
		// * Set increase/decrease button operation. - END
		// =============================================

		// * Set bottom button
		((Button) d.findViewById(R.id.cancel)).setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				d.dismiss();
			}
		});

		((Button) d.findViewById(R.id.ok)).setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				// Save current value as preference!!
				SharedPreferences settings = getSharedPreferences(WConstants.PREF_NAME, 0);
				SharedPreferences.Editor editor = settings.edit();
				editor.putInt(_BOARD_WIDTH, _wboard_size);
				editor.putInt(_BOARD_HEIGHT, _hboard_size);
				editor.commit();

				WriterMain.this._board.createNew(_wboard_size, _hboard_size);
				WriterMain.this._board.initUiState();

				d.dismiss();
			}
		});

		d.show();
	}

	private void _onPenSelected(int color, byte thick) {
		// NOTE :
		// thick starts from 0
		_board.penColor(color).penThick(thick);

	}

	private void _onResultOpen(Intent data) {

		// save opened file name as preferred.

		// retrieve base name without extension
		int last_slash = data.getData().getPath().lastIndexOf("/");
		WDev.wassert(-1 != last_slash);
		int last_dot = data.getData().getPath().lastIndexOf(".");
		WDev.wassert(-1 != last_dot && last_dot > last_slash);
		_last_saved = data.getData().getPath().substring(last_slash + 1, last_dot);

		// load sheet from it
		_loadSheet(data.getData().getPath());
		_board.initUiState();
	}

	private void _activate(ImageView v, boolean bActivate) {
		v.setClickable(bActivate);
		if (bActivate) {
			v.setAlpha(WConstants.ACTIVE_ICON_ALPHA);
		} else {
			v.setAlpha(WConstants.INACTIVE_ICON_ALPHA);
		}
		v.invalidate();
	}

	private void _touchTool(WSelector ws, String clsname) {
		int nr = ws.getItemNR();
		WToolSelectorItem item;
		for (int i = 0; i < nr; i++) {
			item = (WToolSelectorItem)ws.getItem(i);
			if (clsname.equals(item.getStateClass())) {
				ws.touch(i);
				break;
			}
		}
	}

	/**************************
	 * Overriding.
	 **************************/

	private void _createMinimap() {
		final WMiniMap minimap = (WMiniMap) findViewById(R.id.mini_map);
		final WSelector ws = (WSelector) findViewById(R.id.tool_selector);
		ws.init();

		// Set selector
		ws.register_onSelected(new WSelector.OnSelected_Listener() {
			public void onSelect(View v) {
				if (null != v) {
					_board.setState(WriterMain.this, ((WToolSelectorItem)v).getStateClass());
				}
			}
		});
		_touchTool(ws, WBStatePen.class.getName());

		// Listeners
		_board.registerListener(new WBoard.OnActiveRegionMoved_Listener() {
			public void onMoved(Object trigger_owner,
					float nleft,
					float ntop,
					float nright,
					float nbottom) {
				if (trigger_owner != WriterMain.this) {
					minimap.moveActiveRegionTo(WriterMain.this,
									nleft, ntop, nright, nbottom);
				}
			}
		});

		_board.registerListener(new WBoard.OnStateChanged_Listener() {
			public void onChanged(Object trigger_owner, WBStateI state) {
				if (trigger_owner != WriterMain.this) {
					_touchTool(ws, state.getClass().getName());
				}
			}
		});

		minimap.registerListener(new WMiniMap.OnActiveRegionMoved_Listener() {
			public void onMoved(Object trigger_owner, float nleft, float ntop) {
				if (trigger_owner != WriterMain.this) {
					_board.moveActiveRegionTo(
							WriterMain.this,
							(int) (nleft * _board.sheetWidth()),
							(int) (ntop * _board.sheetHeight()));
				}
			}
		});
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		//WDev.log(1, "onCreate");
		//if (null != savedInstanceState)
		//	WDev.log(1, "    => " + savedInstanceState.toString());

		// set Abstraction Layer for this platform to use shared java library.
		WPlatform.setAL(WAL.instance());

		_prepare();

		setContentView(R.layout.main);

		_board = (WBoard) findViewById(R.id.board);
		_board.initBoard();

		{ // Just scope - Board
			// TODO :
			//	If preference has invalid values...
			//	This routine may crash Writer...
			//	Any safe way????
			//	1st Step
			//		Check it whether this unexpected case can be happened.
			//	2nd Step
			//		Guard code for exceptional case.

			// Get from preference!!
			SharedPreferences settings = getSharedPreferences(WConstants.PREF_NAME, 0);
			// Create new board with preferred size.
			_board.createNew(settings.getInt(_BOARD_WIDTH, WConstants.DEFAULT_BOARD_SIZE),
					settings.getInt(_BOARD_HEIGHT, WConstants.DEFAULT_BOARD_SIZE));
		} // Just scope
		_createMinimap();

		// Image Buttons
		_activate((ImageButton) findViewById(R.id.pen_select_btn), true);

		// ===========================
		// Apply preference...
		{ // Just scope
			// Restore preferences
			SharedPreferences settings = getSharedPreferences(WConstants.PREF_NAME, 0);
			_board.penColor(settings.getInt(_PEN_COLOR, Color.BLACK))
					.penThick((byte)settings.getInt(_PEN_THICK, 0));
		} // Just scope

	}

	@Override
	protected void onStart() {
		super.onStart();
		//WDev.log(1, "onStart");
	}

	@Override
	protected void onRestart() {
		super.onRestart();
		//WDev.log(1, "onRestart");
	}

	@Override
	protected void onResume() {
		super.onResume();
		//WDev.log(1, "onResume");
	}

	@Override
	protected void onPause() {
		super.onPause();
		//WDev.log(1, "onPause");
	}

	@Override
	protected void onStop() {
		super.onStop();
		//WDev.log(1, "onStop");
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		//WDev.log(1, "onDestroy");
		{ // Just Scope
			SharedPreferences settings = getSharedPreferences(WConstants.PREF_NAME, 0);
			SharedPreferences.Editor editor = settings.edit();
			editor.putInt(_PEN_COLOR, _board.penColor());
			editor.putInt(_PEN_THICK, _board.penThick());
			editor.commit();
		}

		_board.destroy();
		_board = null;
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		//WDev.log(1, "onSaveInstanceState");
		_saveSheet(_WSHEET_TEMP_SAVE_FILE);
		_board.saveState(outState);
	}

	@Override
	protected void onRestoreInstanceState(Bundle savedInstanceState) {
		//WDev.log(1, "onRestoreInstanceState");
		_board.loadState(savedInstanceState);
		_loadSheet(_WSHEET_TEMP_SAVE_FILE);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.main_opt, menu);

		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		super.onOptionsItemSelected(item);

		switch (item.getItemId()) {
		case R.id.open: {
			_onDiagOpen();
		} break;

		case R.id.save: {
			_onDiagSave();
		} break;

		case R.id.newsheet: {
			_onDiagNew();
		} break;
		}
		return true;
	}

	public void onPenSelectorClicked(View v) {
		_onPenSelect();
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		if (RESULT_OK == resultCode) {
			switch (requestCode) {
			case _REQ_OPEN: {
				_onResultOpen(data);
			} break;
			}
		} else {
			; // ignore...
		}
	}

	/**************************
	 * APIs.
	 **************************/

}