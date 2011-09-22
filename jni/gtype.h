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


#ifndef _GTYPe_h_
#define _GTYPe_h_

#include <stdint.h>
#include "list.h"

/*
 * Top two priority!!!
 *     Saving memory! Increasing speed!
 *     (Number of lines can be reachs to 1,000,000!! We should consider this!)
 */


/*
 * 'left' and 'top' is included at rect.
 * But, 'right' and 'bottom' is not included.
 * rect = { (x, y) | x is [l, r), y is [t, b) }
 */
struct rect {
	int32_t l, t, r, b;
};

struct div {
	struct list_link   lns;  /* lines */
	struct list_link   objs; /* objects */
	struct rect        boundary;
};

/*
 * (x0, y0) is closed. (x1, y1) is open
 */
struct line {
	int32_t     x0, y0, x1, y1;
	uint16_t    color; /* 16bit color */
	uint8_t	    alpha; /* alpha value of color (not used yet) */
	uint8_t	    thick; /* 1~255 is enough! */

	struct div*         div;
	struct list_link*   divlk;
};

/*
 * all objects except for line segment.
 */
struct obj {
	struct rect         extent;
	uint16_t            ty;   /* object type */
	int16_t             ref;  /* reference count */
	void*               priv; /* object private data */
};

struct node {
	struct list_link   lk;
	void*              v;  /* value of this node */
};

struct wsheet {
	uint32_t          divW, divH, colN, rowN;
	struct div**      divs; /* divs[row][col] */
};


#endif /* _GTYPe_h_ */
