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

#include <stdbool.h>
#include <stdint.h>

#include "common.h"
#include "list.h"
#include "gtype.h"
#include "div.h"

struct ucmd;
struct cstk;

struct wsheet {
	struct ucmd*      ucmd; /* current active user command */
	struct cstk*      hstk; /* history stack */
	struct cstk*      undostk; /* undo stack */
	uint32_t          divW, divH, colN, rowN;
	struct div**      divs; /* divs[row][col] */
};


DECL_EXTERN_UT(void
wsys_deinit(void);)

struct wsheet*
wsheet_create(void);

void
wsheet_init(struct wsheet* wsh,
	    int32_t divW, int32_t divH, int32_t colN, int32_t rowN);

void
wsheet_destroy(struct wsheet* wsh);

/*
 * Find div which (x, y) belongs to
 * @return: if (x, y) is not in sheet, NULL is returned.
 */
struct div*
wsheet_find_div(struct wsheet* wsh, int32_t x, int32_t y);

void
wsheet_cutout_lines(struct wsheet* wsh,
		    int32_t l, int32_t t, int32_t r, int32_t b);

/*
 * @nrpts : Number of points (NOT size of pts.)
 *          So, in correct case, size of pts / 2 == nrpts
 */
void
wsheet_add_curve(struct wsheet* wsh,
		 const int32_t* pts, uint16_t nrpts,
		 uint8_t  thick,
		 uint16_t color);

#if 0
/*
 * @return : false (object is out of sheet - not added.) otherwise true.
 */
bool
wsheet_add_obj(struct wsheet* wsh,
	       uint16_t type, void* data,
	       int32_t l, int32_t t, int32_t r, int32_t b);

void
wsheet_del_obj(struct wsheet* wsh, struct obj* o);
#endif

/* this functions is externed only for unit test */

/*
 * NOTE!!!:
 *   Lines that is included in the *rectangle will be returned.
 *   *rectange includes left/top line, but excludes right/bottom line.
 * returned line object should not be modified!!
 * out_keep : all list link should be preserved.
 * out_new : all list link should be deleted.
 *
 * 'r' and 'b' is open.
 */
DECL_EXTERN_UT(void
	       wsheet_find_lines_draw(const struct wsheet* wsh, struct list_link* out,
				      int32_t l, int32_t t, int32_t r, int32_t b);)

void
wsheet_clean(struct wsheet* wsh);

/*
 * 'r' and 'b' is open
 */
void
wsheet_draw(struct wsheet* wsh,
	    int32_t* pixels,
	    int32_t w, int32_t h,
	    int32_t ox, int32_t oy,
	    int32_t l, int32_t t, int32_t r, int32_t b,
	    float zf);


void
wsheet_ucmd_start(struct wsheet* wsh, int ucmdty);

void
wsheet_ucmd_end(struct wsheet* wsh);

static inline void
wsheet_set_ucmd(struct wsheet* wsh, struct ucmd* uc) {
	/*
	 * wsh->ucmd can be set at only following 2 cases.
	 *    NULL -> not NULL
	 *    not NULL -> NULL
	 * So, NULL->NULL and, not NULL -> not NULL are not allowed.
	 */
	wassert((!wsh->ucmd && uc) || (wsh->ucmd && !uc));
	wsh->ucmd = uc;
}

#endif /* _WSHEEt_h_ */
