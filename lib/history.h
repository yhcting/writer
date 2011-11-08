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

#include "wsheet.h"
/*
 * All are not-locked!
 * So, NOT mt-safe
 */

/*
 * initialize history structure
 */
void
his_init(struct wsheet*);

void
his_deinit(struct wsheet*);

void
his_clean(struct wsheet*);

void
his_add(struct wsheet*, struct ucmd*);

/*
 * @return : 1 (nothing happened), 0 (success).
 */
int32_t
his_undo(struct wsheet*);

/*
 * @return : 1 (nothing happened), 0 (success).
 */
int32_t
his_redo(struct wsheet*);

int32_t
his_sz(struct wsheet*);

int32_t
his_undosz(struct wsheet*);

#endif /* _HISTORy_h_ */
