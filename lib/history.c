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

void
his_init(struct wsheet* wsh) {
	wsh->hstk = cstk_create(CONFIG_HISTORY_SZBITS,
			    (void(*)(void*))&ucmd_free);
	wsh->undostk = cstk_create(CONFIG_HISTORY_SZBITS,
			       (void(*)(void*))&ucmd_free);
}

void
his_deinit(struct wsheet* wsh) {
	/* destory all ucmds */
	cstk_destroy(wsh->hstk);
	cstk_destroy(wsh->undostk);
	wsh->hstk = wsh->undostk = NULL;
}

void
his_clean(struct wsheet* wsh) {
	his_deinit(wsh);
	his_init(wsh);
}

void
his_add(struct wsheet* wsh, struct ucmd* uc) {
	wassert(UCMD_ST_DONE == uc->state);
	cstk_push(wsh->hstk, uc);
	/* clean 'undo'ed user command */
	cstk_reset(wsh->undostk);
}

int32_t
his_undo(struct wsheet* wsh) {
	struct ucmd* uc = cstk_pop(wsh->hstk);

	/* stack is empty. nothing to do */
	if (!uc)
		return 1;

	ucmd_undo(uc);
	cstk_push(wsh->undostk, uc);
	return 0;
}

int32_t
his_redo(struct wsheet* wsh) {
	struct ucmd* uc = cstk_pop(wsh->undostk);

	/* stack is empty. nothing to do */
	if (!uc)
		return 1;

	ucmd_redo(uc);
	cstk_push(wsh->hstk, uc);
	return 0;
}

int32_t
his_sz(struct wsheet* wsh) {
	return cstk_sz(wsh->hstk);
}

int32_t
his_undosz(struct wsheet* wsh) {
	return cstk_sz(wsh->undostk);
}
