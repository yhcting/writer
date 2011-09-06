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

#ifndef _AHASh_h_
#define _AHASh_h_

#include <stdbool.h>
#include <stdint.h>

struct ahash*
ahash_create(void);

void
ahash_destroy(struct ahash* h);

uint32_t
ahash_sz(struct ahash* h);

struct ahash*
ahash_add(struct ahash* h, void* p);

struct ahash*
ahash_del(struct yahash* h, void* p);

bool
ahash_check(struct yahash* h, void* p);

#endif /* _AHASh_h_ */
