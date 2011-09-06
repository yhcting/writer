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

#include <stdint.h>

#include "gtype.h"
#include "div.h"

void
wsys_deinit(void);

struct wsheet*
wsheet_create(void);

void
wsheet_init(struct wsheet* wsh,
	    int32_t divW, int32_t divH, int32_t colN, int32_t rowN);

void
wsheet_destroy(struct wsheet* wsh);

void
wsheet_cutout(struct wsheet* wsh,
	      int32_t l, int32_t t, int32_t r, int32_t b);

void
wsheet_add(struct wsheet* wsh,
	   int32_t x0, int32_t y0,
	   int32_t x1, int32_t y1,
	   uint8_t thick,
	   uint16_t color);

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
		  int32_t l, int32_t t, int32_t r, int32_t b);



static inline void
wsheet_clean(struct wsheet* wsh) {
	int32_t i,j;
	for (i = 0; i < wsh->rowN; i++)
		for (j = 0; j < wsh->colN; j++)
			div_clean(&wsh->divs[i][j]);
}

void
wsheet_draw(struct wsheet* wsh,
	    int32_t* pixels,
	    int32_t w, int32_t h,
	    int32_t ox, int32_t oy,
	    int32_t l, int32_t t, int32_t r, int32_t b,
	    float zf);


#endif /* _WSHEEt_h_ */
