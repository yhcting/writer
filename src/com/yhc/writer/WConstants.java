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

class WConstants {
	static final String XMLNS = "http://schemas.android.com/apk/res/com.yhc.writer";
	static final String PREF_NAME = "writerPrefs";

	static final byte LIMIT_THICK	= 5;	// thick is "radius"!! 5 thick means
						// "11 pixels"!! Actuall thick
						// can be "LIMIT_THICK-1"

	static final int ACTIVE_ICON_ALPHA	= 175;
	static final int INACTIVE_ICON_ALPHA	= 50;

	// Default board size
	static final int DEFAULT_BOARD_SIZE	= 10; // 10 times of LCD screen.
	static final int MIN_BOARD_SIZE		= 2;
	static final int MAX_BOARD_SIZE		= 50;

	// zoom out value!!!
	static final float ZOOM_MIN_VALUE	= 1;
	static final float ZOOM_MAX_VALUE	= 4; // 400 % zoom out - min is 100%(original size)
}