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

#include "common.h"
#include "ucmd.h"
#include "cstack.h"
#include "history.h"

static struct cstk* _hstk;     /* history stack */
static struct cstk* _undostk;  /* undo stack */

void
his_init(void) {
	wassert(!_hstk && !_undostk);
	_hstk = cstk_create(CONFIG_HISTORY_SZBITS,
			    (void(*)(void*))&ucmd_free);
	_undostk = cstk_create(CONFIG_HISTORY_SZBITS,
			       (void(*)(void*))&ucmd_free);
}

void
his_deinit(void) {
	cstk_destroy(_hstk);
	cstk_destroy(_undostk);
	_hstk = _undostk = NULL;
}

void
his_add(struct ucmd* uc) {
	wassert(UCMD_ST_DONE == uc->state);
	cstk_push(_hstk, uc);
	/* clean 'undo'ed user command */
	cstk_reset(_undostk);
}

void
his_undo(void) {
	struct ucmd* uc = cstk_pop(_hstk);

	/* stack is empty. nothing to do */
	if (!uc)
		return;

	ucmd_undo(uc);
	cstk_push(_undostk, uc);
}

void
his_redo(void) {
	struct ucmd* uc = cstk_pop(_undostk);

	/* stack is empty. nothing to do */
	if (!uc)
		return;

	ucmd_redo(uc);
	cstk_push(_hstk, uc);
}

int32_t
his_sz(void) {
	return cstk_sz(_hstk);
}

int32_t
his_undosz(void) {
	return cstk_sz(_undostk);
}
