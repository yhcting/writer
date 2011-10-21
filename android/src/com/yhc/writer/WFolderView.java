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
import java.io.FilenameFilter;

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.ContextMenu;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.AdapterContextMenuInfo;

public class WFolderView extends ListActivity {
	private static final int _CONTEXT_MENU_DELETE = 0;

	private String _dir;
	private String _ext;
	private File[] _files;

	private static class _ExtFilter implements FilenameFilter {
		private final String _ext;

		_ExtFilter(String extention) {
			_ext = extention;
		}

		public boolean accept(File dir, String filename) {
			return filename.endsWith(_ext);
		}
	}

	private static class _FVAdapter extends BaseAdapter {
		private final Context _context;
		private final File[] _fs;

		_FVAdapter(Context context, final File[] files) {
			_context = context;
			_fs = files;
		}

		public int getCount() {
			return _fs.length;
		}

		public Object getItem(int position) {
			return _fs[position];
		}

		public long getItemId(int position) {
			return position;
		}

		public View getView(int position, View convertView, ViewGroup parent) {
			ViewGroup vg;
			if (convertView == null) {
				vg = (ViewGroup) LayoutInflater
							.from(_context)
							.inflate(R.layout.folder_view_item,
								parent,
								false);
			} else {
				vg = (ViewGroup) convertView;
			}
			int last_dot = _fs[position].getName().lastIndexOf(".");
			WDev.wassert(last_dot < _fs[position].getName().length());
			((TextView) vg.findViewById(R.id.text))
					.setText(_fs[position].getName().substring(0, last_dot));
			return vg;
		}
	}

	private File[] _fileList(String directory, String ext) {
		File dir = new File(directory);
		WDev.wassert(null != dir && dir.isDirectory());
		return dir.listFiles(new _ExtFilter(ext));
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		Intent i = getIntent();
		WDev.wassert(null != i);
		Bundle b = i.getExtras();

		_dir = i.getData().getPath();
		_ext = b.getString("extention");

		if (!_dir.endsWith("/")) {
			_dir = _dir + "/"; // directory name should be ended with "/"
		}

		_files = _fileList(_dir, _ext);

		ListView lv = getListView();
		// Inform the list we provide context menus for items
		lv.setOnCreateContextMenuListener(this);

		lv.setOnItemClickListener(new AdapterView.OnItemClickListener() {
			public void onItemClick(AdapterView<?> parent,
						View view,
						int position,
						long id) {
				WFolderView.this.setResult(
						RESULT_OK,
						new Intent().setData(Uri.parse("file://" + WFolderView.this._files[position].getPath())));
				WFolderView.this.finish();
			}
		});

		setListAdapter(new _FVAdapter(this, _files));
	}

	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		menu.add(0, _CONTEXT_MENU_DELETE, 0, R.string.delete);
	}

	@Override
	public boolean onContextItemSelected(MenuItem item) {
		AdapterContextMenuInfo info = (AdapterContextMenuInfo) item.getMenuInfo();
		switch (item.getItemId()) {
		case _CONTEXT_MENU_DELETE: {
			String basename = ((TextView) ((ViewGroup) info.targetView).findViewById(R.id.text)).getText().toString();
			File f = new File(_dir + basename + _ext);
			WDev.wassert(null != f);
			if (f.delete()) {
				_files = _fileList(_dir, _ext);
				setListAdapter(new _FVAdapter(this, _files));
			} else {
				Toast.makeText(this,
						R.string.delete_file_fails,
						Toast.LENGTH_LONG).show();
			}
		} return true;

		default:
			return super.onContextItemSelected(item);
		}
	}

}