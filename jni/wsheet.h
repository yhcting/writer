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

#ifndef _WSHEEt_h_
#define _WSHEEt_h_

#include "gtype.h"
#include "div.h"

void
wsys_deinit(void);

struct wsheet*
wsheet_create(void);

void
wsheet_init(struct wsheet* wsh, int divW, int divH, int colN, int rowN);

void
wsheet_destroy(struct wsheet* wsh);

void
wsheet_cutout(struct wsheet* wsh, int l, int t, int r, int b);

void
wsheet_add(struct wsheet* wsh,
	   int x0, int y0,
	   int x1, int y1,
	   char thick,
	   unsigned short color);

/*
 * NOTE!!!:
 *   Lines that is included in the *rectangle will be returned.
 *   *rectange includes left/top line, but excludes right/bottom line.
 * returned line object should not be modified!!
 * out_keep : all list link should be preserved.
 * out_new : all list link should be deleted.
 */
void
wsheet_find_lines(struct wsheet* wsh, struct list_link* out,
		  int l, int t, int r, int b);



static inline void
wsheet_clean(struct wsheet* wsh) {
	int i,j;
	for (i = 0; i < wsh->rowN; i++)
		for (j = 0; j < wsh->colN; j++)
			div_clean(&wsh->divs[i][j]);
}

void
wsheet_draw(struct wsheet* wsh,
	    int* pixels,
	    int w, int h,
	    int ox, int oy,
	    int l, int t, int r, int b,
	    float zf);


#endif /* _WSHEEt_h_ */
