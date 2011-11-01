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

#include "config.h"

#include <stdio.h>
#include <malloc.h>

#include "jni.h"
#include "common.h"
#include "d2d.h"

/*
 * NOTE !!!
 * If array is passed as copy..... performance can be great problem..!!!
 * (In Eclair, memory is pinned.!! - so Ok)
 */

/*
 * Class:     com_yhc_writer_D2d
 * Method:    _nativeFill
 * Signature: ([IIIIIIII)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_D2d__1nativeFill(JNIEnv* env, jclass jclazz,
				     jintArray jarr, jint w, jint h,
				     jint color,
				     jint l, jint t, jint r, jint b) {
	jint* pixels = wjni_get_int_array_direct(env, jarr);
	fill_rect(pixels, w, h, color, l, t, r, b);
	wjni_release_int_array_direct(env, jarr, pixels);
}

/*
 * Class:     com_yhc_writer_D2d
 * Method:    _nativeCopy
 * Signature: ([I[IIIIIIIIIII)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_D2d__1nativeCopy(JNIEnv* env, jclass jclazz,
				     jintArray jarr_dst, jintArray jarr_src,
				     jint dw, jint dh, jint dl, jint dt,
				     jint sw, jint sh, jint sl, jint st,
				     jint width, jint height) {

	jint* dst_pixels = wjni_get_int_array_direct(env, jarr_dst);
	jint* src_pixels = wjni_get_int_array_direct(env, jarr_src);
	copy_rect(dst_pixels, src_pixels,
		  dw, dh, dl, dt,
		  sw, sh, sl, st,
		  width, height);
	wjni_release_int_array_direct(env, jarr_dst, dst_pixels);
	wjni_release_int_array_direct(env, jarr_src, src_pixels);
}

/*
 * Class:     com_yhc_writer_D2d
 * Method:    _nativeDrawLine
 * Signature: ([IIIIBIIII)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_D2d__1nativeDrawLine(JNIEnv* env, jclass jclazz,
					   jintArray jarr, jint w, jint h,
					   jint color, jbyte thick,
					   jint x0, jint y0,
					   jint x1, jint y1) {

	jint* pixels = wjni_get_int_array_direct(env, jarr);
	draw_line(pixels, w, h, color, thick, x0, y0, x1, y1);
	wjni_release_int_array_direct(env, jarr, pixels);
}
