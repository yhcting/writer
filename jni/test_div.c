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

#include "common.h"
#include "wsheet.h"

#define DIV_W 100
#define DIV_H 100

#define DIV1_L 0
#define DIV1_T 0
#define DIV1_R (DIV1_L + DIV_W)
#define DIV1_B (DIV1_T + DIV_H)

#define DIV2_L DIV1_R
#define DIV2_T DIV1_T
#define DIV2_R (DIV2_L + DIV_W)
#define DIV2_B (DIV2_T + DIV_H)

#define OUTL_L (DIV1_L - 50)
#define OUTL_R (DIV1_L - 10)
#define OUTR_L (DIV2_R + 10)
#define OUTR_R (DIV2_R + 50)
#define OUTT_T (DIV1_T - 50)
#define OUTT_B (DIV1_T - 10)
#define OUTB_T (DIV1_B + 10)
#define OUTB_B (DIV1_B + 50)

#define IN1_L  (DIV1_L + 10)
#define IN1_T  (DIV1_T + 10)
#define IN1_R  (DIV1_R - 10)
#define IN1_B  (DIV1_B - 10)

#define IN2_L  (DIV2_L + 10)
#define IN2_T  (DIV2_T + 10)
#define IN2_R  (DIV2_R - 10)
#define IN2_B  (DIV2_B - 10)

#define IN12_L (DIV1_R - 50)
#define IN12_T (DIV1_T + 10)
#define IN12_R (DIV2_L + 50)
#define IN12_B (DIV1_B - 10)

static int
_test_div(void) {
	int    i;
	struct wsheet* wsh;
	struct obj objs[] = {
		/*
		 * objs out of div.
		 * ----------------
		 */

		/* out at left */
		{{OUTL_L, IN1_T, OUTL_R, IN1_B},
		 0, 0, NULL},
		/* out at right */
		{{OUTR_L, IN1_T, OUTR_R, IN1_B},
		 0, 0, NULL},
		/* out at top */
		{{IN12_L, OUTT_T, IN12_R, OUTT_B},
		 0, 0, NULL},
		/* out at bottom */
		{{IN12_L, OUTB_T, IN12_R, OUTB_B},
		 0, 0, NULL},
		/* out at top left */
		{{OUTL_L, OUTT_T, OUTL_R, OUTT_B},
		 0, 0, NULL},
		/* out at top right */
		{{OUTR_L, OUTT_T, OUTR_R, OUTT_B},
		 0, 0, NULL},
		/* out at bottom left */
		{{OUTL_L, OUTB_T, OUTL_R, OUTB_B},
		 0, 0, NULL},
		/* out at bottom right */
		{{OUTR_L, OUTB_T, OUTR_R, OUTB_B},
		 0, 0, NULL},


		/*
		 * objs overwrapped
		 * ----------------
		 */
		/* overwrapped at left */
		{{OUTL_R, IN1_T, IN1_L, IN1_B},
		 0, 0, NULL},
		/* overwrapped at right */
		{{IN2_R, IN1_T, OUTR_L, IN1_B},
		 0, 0, NULL},
		/* overwrapped at top */
		{{IN1_L, OUTT_B, IN1_R, IN1_T},
		 0, 0, NULL},
		/* overwrapped at bottom */
		{{IN1_L, IN1_B, IN1_R, OUTB_T},
		 0, 0, NULL},
		/* overwrapped at left top */
		{{OUTL_R, OUTT_B, IN1_L, IN1_B},
		 0, 0, NULL},
		/* overwrapped at right top */
		{{IN2_R, OUTT_B, OUTR_L, IN1_B},
		 0, 0, NULL},
		/* overwrapped at left bottom */
		{{OUTL_R, IN1_B, IN1_R, OUTB_T},
		 0, 0, NULL},
		/* overwrapped at right bottom */
		{{IN2_R, IN2_B, OUTR_L, OUTB_T},
		 0, 0, NULL},
		/* overwrapped at top with 1 and 2 */
		{{OUTL_R, OUTT_B, OUTR_L, IN1_T},
		 0, 0, NULL},
		/* overwrapped at bottom with 1 and 2 */
		{{OUTL_R, IN1_B, OUTR_L, OUTB_T},
		 0, 0, NULL},
		/* overwrapped at middle with 1 and 2 */
		{{OUTL_R, IN1_T, OUTR_L, IN1_B},
		 0, 0, NULL},

		/*
		 * objects covering 1 and 2
		 * ------------------------
		 */
		/* overwrapped (covering 1 and 2) */
		{{OUTL_R, OUTT_B, OUTR_L, OUTB_T},
		 0, 0, NULL},
	};

	wsh = wsheet_create();
	wsheet_init(wsh, 100, 100, 2, 1);
	for (i = 0; i < sizeof(objs)/sizeof(objs[0]); i++) {
		wsheet_add_obj(wsh,
			       objs[i].ty, objs[i].priv,
			       objs[i].extent.l, objs[i].extent.t,
			       objs[i].extent.r, objs[i].extent.b);
		/*
		printf("%d, %d\n",
		       list_size(&wsh->divs[0][0].objs),
		       list_size(&wsh->divs[0][1].objs));
		*/
	}
	wassert(9 == list_size(&wsh->divs[0][0].objs));
	wassert(7 == list_size(&wsh->divs[0][1].objs));

	wsheet_destroy(wsh);
	wsys_deinit();

	return 0;
}

TESTFN(_test_div, DIV)

#endif /* CONFIG_TEST_EXECUTABLE */
