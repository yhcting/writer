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

#ifndef _TESTPTs_h_
#define _TESTPTs_h_

/*
 * +----->
 * |  +----------+----------+----------+
 * |  |*         |          |          |
 * v  |        * |          |          |
 *    |          |          |          |
 *    |          |    *     |          |
 *    +----------+----------+----------+
 *    |          |          |          |
 *    |          |          |          |
 *    |          |       *  |          |
 *    |          |          |    *     |
 *    +----------+----------+----------+
 */
static int32_t _pts0[] __attribute__((unused)) = {
	1,  1,
	9,  3,
	14, 9,
	17, 15,
	24, 19
};

/*
 * +----->
 * |  +----------+----------+----------+
 * |  |          |          |          |
 * v  |        * |          |          |
 *    |          |          |          |
 *    |          *          |          |
 *    +----------+----------+----------+
 *    |          |          |          |
 *    |          *          |          |
 *    |          |          |          |
 *    |          | *        |          |
 *    +----------+---*----*-+----------+
 *    |          |          |   *      |
 *    |          |          |          |
 *    |          |          |          |
 *    |          |          |          |
 *    +----------+----------+----------+
 */
static int32_t _pts1[] __attribute__((unused)) = {
	8,  3,
	10, 8,
	10, 13,
	12, 17,
	14, 20,
	18, 20,
	24, 3
};

/*
 * +----->
 * |  +----------+----------+----------+----------+
 * |  |          |          |          |          |
 * v  |        * |          |          |          |
 *    |          |          | *        |          |
 *    |          |          |        * |          |
 *    +----------+----------+----------+--------*-+
 */
static int32_t _pts2[] __attribute__((unused)) = {
	8,  3,
	23, 6,
	28, 7,
	37, 10,
};

/*
 * +----->
 * |  +----------+----------+----------+----------+
 * |  |          |          |          |          |
 * v  |          |          |          |          |
 *    |          |          |          |          |
 *    | *        |          |          |          |
 *    +----------+----------+----------+----------+
 *    |          |          |          |          |
 *    |          | *        |          |          |
 *    |          |          |          |          |
 *    |          |       *  |          |          |
 *    +----------+----------+----------+----------+
 *    |          |          |          |          |
 *    |          |          |          |          |
 *    |          |          |          |          |
 *    |          |          |*         |          |
 *    +----------+----------+----------+----------+
 *    |          |          |          |          |
 *    |          |          |          |          |
 *    |          |          |          |          |
 *    |          |          |          | *        |
 *    +----------+----------+----------+----------+
 */
static int32_t _pts3[] __attribute__((unused)) = {
	3,  7,
	12, 14,
	17, 17,
	22, 27,
	33, 37,
};


#endif /* _TESTPTs_h_ */
