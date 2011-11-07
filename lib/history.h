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

#ifndef _HISTORy_h_
#define _HISTORy_h_

#include <stdint.h>

/*
 * All are not-locked!
 * So, NOT mt-safe
 */

/*
 * initialize history structure
 */
void
his_init(void);

void
his_deinit(void);

void
his_clean(void);

void
his_add(struct ucmd*);

void
his_undo(void);

void
his_redo(void);

int32_t
his_sz(void);

int32_t
his_undosz(void);

#endif /* _HISTORy_h_ */
