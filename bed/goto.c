/* SccsId: @(#)goto.c	1.6 11/15/83
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Random access focus positioning.
 */

#include "b.h"
#include "node.h"
#include "gram.h"
#include "supr.h"


#define BEFORE (-1)
#define INSIDE 0
#define BEYOND 1


/*
 * Random cursor positioning (e.g., with a mouse).
 */

Visible bool
gotocursor(ep)
	environ *ep;
{
	int y;
	int x;

	if (!sense(&y, &x))
		return No;
	if (!backtranslate(&y, &x))
		return No;
	return gotoyx(ep, y, x);
}


/*
 * Set the focus to the smallest node or subset surrounding
 * the position (y, x).
 */

Visible bool
gotoyx(ep, y, x)
	register environ *ep;
	register int y;
	register int x;
{
	register node n;
	register string *rp;
	register int i;
	register int pc;

	ep->mode = WHOLE;
	while ((pc = poscomp(ep->focus, y, x)) != INSIDE) {
		if (!up(&ep->focus)) {
			if (pc == BEFORE)
				ep->mode = ATBEGIN;
			else
				ep->mode = ATEND;
			higher(ep);
			return No;
		}
	}
	higher(ep);
	for (;;) {
		switch (poscomp(ep->focus, y, x)) {

		case BEFORE:
			i = ichild(ep->focus);
			n = tree(parent(ep->focus)); /* Parent's !!! */
			rp = noderepr(n);
			if (Fw_positive(rp[i-1])) {
				s_up(ep);
				ep->s1 = ep->s2 = 2*i - 1;
				ep->mode = SUBSET;
			}
			else if (left(&ep->focus))
				ep->mode = ATEND;
			else
				ep->mode = ATBEGIN;
			return Yes;

		case INSIDE:
			n = tree(ep->focus);
			if (nchildren(n) >= 1 && Type(child(n, 1)) != Tex) {
				s_down(ep);
				continue;
			}
			ep->mode = WHOLE;
			return Yes;

		case BEYOND:
			if (rite(&ep->focus))
				continue;
			n = tree(parent(ep->focus)); /* Parent's !!! */
			rp = noderepr(n);
			i = ichild(ep->focus);
			if (Fw_positive(rp[i])) {
				s_up(ep);
				ep->s1 = ep->s2 = 2*i + 1;
				ep->mode = SUBSET;
			}
			else
				ep->mode = ATEND;
			return Yes;

		default:
			Abort();
			/* NOTREACHED */

		}
	}
}


/*
 * Deliver relative position of (y, x) with respect to focus p:
 * < 0: (y, x) precedes focus; == 0: (y, x) contained in focus;
 * > 0: (y, x) follows focus.
 */

Hidden int
poscomp(p, y, x)
	register path p;
	register int y;
	register int x;
{
	register int ly;
	register int lx;
	register int w;
	register string *rp;
	register node n;

	ly = Ycoord(p);
	lx = Xcoord(p);
	if (y < ly || y == ly && (lx < 0 || x < lx))
		return BEFORE;
	n = tree(p);
	w = width(n);
	if (w < 0) {
		if (y == ly) { /* Hack for position beyond end of previous line */
			rp = noderepr(n);
			if (Fw_negative(rp[0]))
				return BEFORE;
		}
		ly += -w;
		lx = -1;
	}
	else {
		if (lx >= 0)
			lx += w;
	}
	if (y < ly || y == ly && (lx < 0 || x < lx))
		return INSIDE;
	return BEYOND;
}
