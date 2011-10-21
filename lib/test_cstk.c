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

#include <stdio.h>
#include <assert.h>

#include "cstack.h"

#define _TESTBITSZ 2
#define _TESTSZ    4 /* 2^2 */

static int
_test_cstack(void) {
	struct cstk* s = cstk_create(_TESTBITSZ, NULL);
	int          i;
	int*         p;

	wassert(cstk_is_empty(s) && !cstk_is_full(s));

	p = wmalloc(sizeof *p);
	*p = 1;
	cstk_push(s, p);
	wassert(!cstk_is_empty(s) && !cstk_is_full(s));

	p = cstk_pop(s);
	wassert((1 == *p)
		&& cstk_is_empty(s) && !cstk_is_full(s));
	wfree(p);

	for (i = 0; i < _TESTSZ + 2; i++) {
		p = wmalloc(sizeof *p);
		*p = i;
		cstk_push(s, p);
	}
	wassert(!cstk_is_empty(s) && cstk_is_full(s));
	/* cstk_dump(s); */

	p = cstk_pop(s);
	wassert((_TESTSZ + 1) == *p);
	wfree(p);
	/* cstk_dump(s); */

	p = cstk_pop(s);
	wassert(_TESTSZ == *p);
	wfree(p);
	/* cstk_dump(s); */

	p = cstk_pop(s);
	wassert(3 == *p);
	wfree(p);
	/* cstk_dump(s); */

	cstk_destroy(s);

	return 0;
}

#undef _TESTSZ

TESTFN(_test_cstack, CSTACK, TESTPRI_MODULE)


#endif /* CONFIG_TEST_EXECUTABLE */
