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

import android.util.Log;

class WDev {

	// Configuration values.
	static final boolean __DEBUG__	= true;
	static final boolean __TEST__	= false;

	private static final int _LOG_LEVEL = 1;

	static void wassert(boolean cond, String msg) {
		if (__DEBUG__) {
			if (!cond)
				throw new AssertionError(msg);
			// junit.framework.Assert.assertTrue(msg, cond);
		}
	}

	static void wassert(boolean cond) {
		wassert(cond, "");
	}

	static void log(int level, String string) {
		if (__DEBUG__ && _LOG_LEVEL <= level)
			Log.d("[Writer]", string);
	}

	/**
	 * This function is used only to increase readibility
	 */
	static void unused_parameter(Object o) {
	}
}