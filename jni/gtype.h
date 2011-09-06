#ifndef _GTYPe_h_
#define _GTYPe_h_

#include "list.h"

/*
 * Top two priority!!!
 *     Saving memory! Increasing speed!
 *     (Number of lines can be reachs to 1,000,000!! We should consider this!)
 */

struct rect {
	int l, t, r, b;
};

struct div {
	struct list_link   lns;  /* lines */
	struct list_link   objs; /* objects */
	struct rect        boundary;
};

/*
 * x-sorted. That is, (x0 <= x1) should be alwasy true!.
 */
struct line {
	unsigned short	    x0, y0, x1, y1;
	unsigned short	    color; /* 16bit color */
	unsigned char	    alpha; /* alpha value of color (not used yet) */
	unsigned char	    thick; /* 1~255 is enough! */

	struct div*         div;
	struct list_link*   divlk;
};

struct node {
	struct line*       ln;
	struct list_link   lk;
};

struct wsheet {
	int	     divW, divH, colN, rowN;
	struct div** divs; /* divs[row][col] */
};


#endif /* _GTYPe_h_ */
