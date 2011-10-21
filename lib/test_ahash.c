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

#ifdef CONFIG_TEST_EXECUTABLE
#include <stdint.h>

#include "common.h"
#include "ahash.h"

static int
_test_ahash(void) {
	intptr_t      i;
	struct ahash* ah = ahash_create();

	for (i = 0; i < 1024; i++)
		ahash_add(ah, (void*)i);
	wassert(1024 == ahash_sz(ah));

	for (i = 128; i < 512; i++)
		ahash_del(ah, (void*)i);
	wassert(640 == ahash_sz(ah));

	for (i = 600; i < 1000; i++)
		ahash_del(ah, (void*)i);
	wassert(240 == ahash_sz(ah));

	for (i = 150; i < 500; i++)
		ahash_add(ah, (void*)i);
	wassert(590 == ahash_sz(ah));

	for (i = 200; i < 400; i++)
		ahash_del(ah, (void*)i);
	wassert(390 == ahash_sz(ah));

	for (i = 0; i < 128; i++)
		wassert(ahash_check(ah, (void*)i));

	for (i = 1000; i < 1024; i++)
		wassert(ahash_check(ah, (void*)i));

	for (i = 200; i < 400; i++)
		wassert(!ahash_check(ah, (void*)i));

	ahash_destroy(ah);

	return 0;
}

TESTFN(_test_ahash, AHASH, TESTPRI_MODULE)

#endif /* CONFIG_TEST_EXECUTABLE */
