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
#include "wsheet.h"

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeCreateWsheet
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeCreateWsheet(JNIEnv* env, jclass jclazz) {
	return (jint)wsheet_create();
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeInitWsheet
 * Signature: (IIIII)I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeInitWsheet(JNIEnv* env, jclass jclazz,
					      jint sheet,
					      jint divW, jint divH,
					      jint colN, jint rowN) {
#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_init();
#endif /* CONFIG_DBG_STATISTICS */

	wsheet_init((struct wsheet*)sheet, divW, divH, colN, rowN);
	return 1;
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeDestroyWsheet
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_WSheet__1nativeDestroyWsheet(JNIEnv* env, jclass jclazz,
						 jint sheet) {
	wsheet_destroy((struct wsheet*)sheet);
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeWidth
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeWidth(JNIEnv* env, jclass jclazz,
					 jint sheet) {
	return ((struct wsheet*)sheet)->divW * ((struct wsheet*)sheet)->colN;
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeHeight
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeHeight(JNIEnv* env, jclass jclazz,
					  jint sheet) {
	return ((struct wsheet*)sheet)->divH * ((struct wsheet*)sheet)->rowN;
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeCutout
 * Signature: (IIIII)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_WSheet__1nativeCutout(JNIEnv* env, jclass jclazz,
					    jint sheet,
					    jint l, jint t, jint r, jint b) {
#ifdef CONFIG_DBG_STATISTICS
	dbg_tpf_print(DBG_PERF_FILL_RECT);
	dbg_tpf_print(DBG_PERF_FIND_LINE);
	dbg_tpf_print(DBG_PERF_DRAW_LINE);
#endif /* CONFIG_DBG_STATISTICS */
	wsheet_cutout((struct wsheet*)sheet, l, t, r, b);
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeAdd
 * Signature: (IIIIICS)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_WSheet__1nativeAdd(JNIEnv* env, jclass jclazz,
				       jint sheet,
				       jint x0, jint y0,
				       jint x1, jint y1,
				       jbyte thick, jshort color) {
	wsheet_add((struct wsheet*)sheet, x0, y0, x1, y1, thick, color);
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeDraw
 * Signature: (I[IIIIIIIIIF)V
 */
JNIEXPORT void JNICALL
Java_com_yhc_writer_WSheet__1nativeDraw(JNIEnv* env, jclass jclazz,
					jint sheet,
					jintArray jarr,
					jint w, jint h,
					jint ox, jint oy,
					jint l, jint t, jint r, jint b,
					jfloat zf) {

	jint* pixels = (*env)->GetIntArrayElements(env, jarr, NULL);

	wsheet_draw((struct wsheet*)sheet,
		    pixels, w, h, ox, oy, l, t, r, b, zf);

	(*env)->ReleaseIntArrayElements(env, jarr, pixels, JNI_ABORT);
}



/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeSave
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeSave(JNIEnv* env, jclass jclazz,
					jint sheet, jstring jpath) {

	int             ret  = 0;
	const char*     path = (*env)->GetStringUTFChars(env, jpath, NULL);
	FILE*           fp   = fopen(path, "w");
	struct wsheet*  wsh  = (struct wsheet*)sheet;
	struct node*    n;
	struct line*    ln;
	int	        i, j, sz;

	if (NULL == fp)
		goto bail;

#define _FW(vAL)  fwrite(&(vAL), sizeof(vAL), 1, fp)

	_FW(wsh->divW);
	_FW(wsh->divH);
	_FW(wsh->colN);
	_FW(wsh->rowN);

	for (i = 0; i < wsh->rowN; i++) {
		for (j = 0; j < wsh->colN; j++) {
			sz = list_size(&wsh->divs[i][j].lns);
			_FW(sz);
			list_foreach_item(n,
					  &wsh->divs[i][j].lns,
					  struct node,
					  lk) {
				ln = n->ln;
				_FW(ln->x0);
				_FW(ln->y0);
				_FW(ln->x1);
				_FW(ln->y1);
				_FW(ln->color);
				_FW(ln->thick);
				_FW(ln->alpha);
			}

		}
	}

#undef _FW

	ret = 1; // success

 bail:
	if (NULL != fp)
		fclose(fp);
	(*env)->ReleaseStringUTFChars(env, jpath, path);
	return ret;
}

/*
 * Class:     com_yhc_writer_WSheet
 * Method:    _nativeLoad
 * Signature: (ILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL
Java_com_yhc_writer_WSheet__1nativeLoad(JNIEnv* env, jclass jclazz,
					jint sheet, jstring jpath) {

	int             ret  = 0;
	const char*     path = (*env)->GetStringUTFChars(env, jpath, NULL);
	FILE*           fp   = fopen(path, "r");
	struct wsheet*  wsh  = (struct wsheet*)sheet;
	struct line*    ln;
	int	        i, j, k, sz;

	if (NULL == fp)
		goto bail;

#define _FR(vAL)  fread(&(vAL), sizeof(vAL), 1, fp)

	_FR(wsh->divW);
	_FR(wsh->divH);
	_FR(wsh->colN);
	_FR(wsh->rowN);

	wsheet_init(wsh, wsh->divW, wsh->divH, wsh->colN, wsh->rowN);

	for (i = 0; i < wsh->rowN; i++) {
		for (j = 0; j < wsh->colN; j++) {
			_FR(sz);
			for (k = 0; k < sz; k++) {
				ln = wmalloc(sizeof(*ln));
				_FR(ln->x0);
				_FR(ln->y0);
				_FR(ln->x1);
				_FR(ln->y1);
				_FR(ln->color);
				_FR(ln->thick);
				_FR(ln->alpha);
				div_add(&wsh->divs[i][j], ln);
			}
		}
	}

#undef _FR

	ret = 1; // success

 bail:
	if (NULL != fp)
		fclose(fp);
	(*env)->ReleaseStringUTFChars(env, jpath, path);
	return ret;
}

