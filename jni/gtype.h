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

/*
 * Generic types
 */

#ifndef _GTYPe_h_
#define _GTYPe_h_

#include <stdint.h>

#include "list.h"

struct div;
struct node;

/*
 * 'left' and 'top' is included at rect.
 * But, 'right' and 'bottom' is not included.
 * rect = { (x, y) | x is [l, r), y is [t, b) }
 */
struct rect {
	int32_t l, t, r, b;
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

/*
 * segment of curve
 * head and tail should be at same division.
 * @head and @tail are pointer of division node.
 */
struct curve {
	struct list_link lk;
	struct node     *head, *tail;
};

struct curves {
	/* list of division nodes */
	struct list_link     divns;
	/* list of curve */
	struct list_link     crvs;
};

#endif /* _GTYPe_h_ */
