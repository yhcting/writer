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
 * File interface
 *
 * NOTE
 *   Exception handling is NOT implemented yet!!
 *   (Ex, reading wrong formated file!)
 */


#include "config.h"

#include <stdio.h>
#include <stdint.h>

#include "common.h"
#include "wsheet.h"
#include "div.h"
#include "curve.h"


/*
 * There is no meaning... just magic number
 */
static const uint32_t _magicnr[] = {
	0x18ab9eb9,
	0xe48a8aff,
	0x3838af01,
	0x3fa8df00
};



static void
_wcurve(FILE* fp, const struct curve* crv) {
	const struct point *p, *pe;
#define _FW(vAL)  fwrite(&(vAL), sizeof(vAL), 1, fp)

	_FW(crv->nrpts);
	_FW(crv->color);
	_FW(crv->alpha);
	_FW(crv->thick);

	p = crv->pts;
	pe = p + crv->nrpts;
	/*
	 * To increase portability, write 'int32_t' values instead of
	 *   'struct point'.
	 */
	while (p < pe) {
		_FW(p->x);
		_FW(p->y);
		p++;
	}

#undef _FW
}

static struct curve*
_rcurve(FILE* fp) {
	struct curve* crv;
	uint16_t      nrpts;
	struct point *p, *pe;

#define _FR(vAL)  fread(&(vAL), sizeof(vAL), 1, fp)

	_FR(nrpts);
	crv = crv_create(nrpts);
	wassert(crv);
	_FR(crv->color);
	_FR(crv->alpha);
	_FR(crv->thick);

	p = crv->pts;
	pe = p + crv->nrpts;
	/*
	 * To increase portability, write 'int32_t' values instead of
	 *   'struct point'.
	 */
	while (p < pe) {
		_FR(p->x);
		_FR(p->y);
		p++;
	}

	return crv;

#undef _FR
}



int
file_save_wsheet(FILE* fp, const struct wsheet* wsh) {
	struct curve* crv;
	int           i, j;
	uint32_t      nrcrv = 0;
	long          fpos;

#define _FW(vAL)  fwrite(&(vAL), sizeof(vAL), 1, fp)

	for (i = 0; i < arrsz(_magicnr); i++)
		_FW(_magicnr[i]);

	_FW(wsh->divW);
	_FW(wsh->divH);
	_FW(wsh->colN);
	_FW(wsh->rowN);
	fpos = ftell(fp);
	_FW(nrcrv); /* holding position */

	for (i = 0; i < wsh->rowN; i++)
		for (j = 0; j < wsh->colN; j++)
			crv_foreach(crv, &wsh->divs[i][j].crvs) {
				_wcurve(fp, crv);
				nrcrv++;
			}

	/* write to reserved position */
	fseek(fp, fpos, SEEK_SET);
	_FW(nrcrv);
	return 0;

#undef _FW
}

int
file_load_wsheet(FILE* fp, struct wsheet* wsh) {
	struct curve* crv;
	struct div*   div;
	int           i;
	uint32_t      m;

#define _FR(vAL)  fread(&(vAL), sizeof(vAL), 1, fp);

	for (i = 0; i < arrsz(_magicnr); i++) {
		m = 0; /* 0 is NOT one of magic number */
		_FR(m);
		if (m != _magicnr[i])
			return -1;
	}


	_FR(wsh->divW);
	_FR(wsh->divH);
	_FR(wsh->colN);
	_FR(wsh->rowN);
	_FR(m); /* number of curves */

	wsheet_init(wsh, wsh->divW, wsh->divH, wsh->colN, wsh->rowN);
	while (m--) {
		crv = _rcurve(fp);
		wassert(crv->nrpts > 1);
		/*
		 * Save curve is only in one division.
		 */
		div = wsheet_find_div(wsh, crv->pts[0].x, crv->pts[0].y);
		wassert(div);
		div_add_curve(div, crv);
	}

	return 0;

#undef _FR
}
