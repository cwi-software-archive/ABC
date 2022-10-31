/* SccsId: @(#)move.c	1.8 11/21/83
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Process arrow keys in four directions, plus TAB.
 */

#include "b.h"
#include "bobj.h"
#include "node.h"
#include "supr.h"
#include "gram.h"

#define Left (-1)
#define Rite 1


/*
 * Common code for PREVIOUS and NEXT commands.
 */

Hidden bool
prevnext(ep, direction)
	environ *ep;
{
	node n;
	node n1;
	int nch;
	int i;
	int len;
	int sym;
	string *rp;

	higher(ep);
	switch (ep->mode) {
	case VHOLE:
	case FHOLE:
	case ATBEGIN:
	case ATEND:
		if (direction == Left)
			leftvhole(ep);
		else
			ritevhole(ep);
	}

	for (;;) {
		n = tree(ep->focus);
		nch = nchildren(n);
		rp = noderepr(n);

		switch (ep->mode) {

		case ATBEGIN:
		case ATEND:
			ep->mode = WHOLE;
			continue;

		case VHOLE:
		case FHOLE:
			if (direction == Rite) {
				if (ep->s1&1)
					len = Fwidth(rp[ep->s1/2]);
				else {
					n1 = child(n, ep->s1/2);
					len = width(n1);
				}
			}
			if (direction == Rite ? ep->s2 >= len : ep->s2 <= 0) {
				ep->mode = SUBSET;
				ep->s2 = ep->s1;
				return nextchar(ep, direction);
			}
			ep->s2 += direction;
			return Yes;

		case SUBRANGE:
			if (direction == Rite) {
				if (ep->s1&1)
					len = Fwidth(rp[ep->s1/2]);
				else {
					n1 = child(n, ep->s1/2);
					len = width(n1);
				}
			}
			if (direction == Left ? ep->s2 <= 0 : ep->s3 >= len-1) {
				ep->mode = SUBSET;
				ep->s2 = ep->s1;
				return nextchar(ep, direction);
			}
			if (direction == Rite)
				ep->s2 = ++ep->s3;
			else
				ep->s3 = --ep->s2;
			return Yes;

		case SUBSET:
			if (direction == Rite ? ep->s2 > 2*nch : ep->s1 <= 1) {
				ep->mode = WHOLE;
				continue;
			}
			if (direction == Rite)
				ep->s1 = ++ep->s2;
			else
				ep->s2 = --ep->s1;
			if (ep->s1&1) {
				if (!Fw_positive(rp[ep->s1/2]) || allspaces(rp[ep->s1/2]))
					continue;
			}
			else {
				sym = symbol(n);
				if (downi(&ep->focus, ep->s1/2)) {
					n = tree(ep->focus);
					if (((value)n)->type == Tex)
						s_up(ep);
					else {
						if (ep->s1 == 2*nch && direction == Rite
							&& issublist(sym) && samelevel(sym, symbol(n))) {
							ep->mode = SUBLIST;
							ep->s3 = 1;
							return Yes;
						}
						ep->mode = WHOLE;
						if (width(n) == 0)
							continue;
					}
				}
			}
			return Yes;

		case SUBLIST:
			sym = symbol(n);
			if (direction == Left) {
				i = ichild(ep->focus);
				if (!up(&ep->focus))
					return No;
				higher(ep);
				n = tree(ep->focus);
				if (i == nchildren(n) && samelevel(sym, symbol(n))) {
					ep->s3 = 1;
					return Yes;
				}
				ep->mode = SUBSET;
				ep->s1 = ep->s2 = 2*i;
				continue;
			}
			for (i = ep->s3; i > 0; --i)
				if (!downrite(&ep->focus))
					return No; /* Sorry... */
			if (samelevel(sym, symbol(tree(ep->focus))))
				ep->s3 = 1;
			else
				ep->mode = WHOLE;
			return Yes;

		case WHOLE:
			i = ichild(ep->focus);
			if (!up(&ep->focus))
				return No;
			higher(ep);
			ep->mode = SUBSET;
			ep->s1 = ep->s2 = 2*i;
			continue;

		default:
			Abort();
		}
	}
	/* Not reached */
}


Visible bool
previous(ep)
	environ *ep;
{
	if (!prevnext(ep, Left))
		return No;
	return Yes;
}


Visible bool
next(ep)
	environ *ep;
{
	if (!prevnext(ep, Rite))
		return No;
	return Yes;
}


/*
 * Position focus at next or previous char relative to current position.
 * Assume current position given as SUBSET.
 */

Hidden bool
nextchar(ep, direction)
	environ *ep;
	int direction;
{
	int ich;
	int nch;
	node n;
	node n1;
	int len;
	string *rp;

	Assert(ep->mode == SUBSET);
	for (;;) {
		n = tree(ep->focus);
		rp = noderepr(n);
		nch = nchildren(n);
		if (direction == Left)
			ep->s2 = --ep->s1;
		else
			ep->s1 = ++ep->s2;
		if (direction == Left ? ep->s1 < 1 : ep->s2 > 2*nch+1) {
			ich = ichild(ep->focus);
			if (!up(&ep->focus))
				return No; /* *ep is garbage now! */
			higher(ep);
			ep->s1 = ep->s2 = 2*ich;
			continue;
		}
		if (ep->s1&1) {
			len = Fwidth(rp[ep->s1/2]);
			if (len > 0) {
				ep->mode = SUBRANGE;
				ep->s2 = ep->s3 = direction == Left ? len-1 : 0;
				return Yes;
			}
			continue;
		}
		n1 = child(n, ep->s1/2);
		len = width(n1);
		if (len == 0)
			continue;
		if (!downi(&ep->focus, ep->s1/2))
			return No; /* Sorry... */
		n = tree(ep->focus);
		if (((value)n)->type == Tex) {
			s_up(ep);
			ep->mode = SUBRANGE;
			ep->s2 = ep->s3 = direction == Left ? len-1 : 0;
			return Yes;
		}
		if (direction == Left) {
			nch = nchildren(n);
			ep->s1 = ep->s2 = 2*(nch+1);
		}
		else
			ep->s1 = ep->s2 = 0;
	}
	/* Not reached */
}


/*
 * Up and down arrows.
 */

Visible bool
uparrow(ep)
	environ *ep;
{
	register int y;

	y = lineno(ep->focus);
	if (y <= 0)
		return No;
	if (!gotoyx(ep, y-1, 0))
		return No;
	oneline(ep);
	return Yes;
}

Visible bool
downarrow(ep)
	environ *ep;
{
	if (!parent(ep->focus) && ep->mode == ATEND)
		return No; /* Superfluous? */
	if (!gotoyx(ep, lineno(ep->focus) + 1, 0))
		return No;
	oneline(ep);
	return Yes;
}


/*
 * Tab function -- move to next Hole hole or to end of line.
 */

Visible bool
tabulate(ep)
	register environ *ep;
{
	register int skipit = No;
	register int ich;

	higher(ep);
	shrink(ep);
	switch (ep->mode) {
	case VHOLE:
	case FHOLE:
	case ATEND:
	case ATBEGIN:
		ritevhole(ep);
	}
	switch (ep->mode) {

	case ATBEGIN:
		debug("[Sorry--tab at ATBEGIN]"); /*****/
		return No;

	case WHOLE:
		skipit = Yes;
		break;

	case ATEND:
		debug("[Sorry--tab at ATEND]"); /*****/
		return No;

	case SUBLIST:
		Assert(ep->s3 > 1);
		ep->s1 = 1;
		/* Fall through */
	case VHOLE:
	case FHOLE:
	case SUBRANGE:
		ep->mode = SUBSET;
		ep->s2 = ep->s1;
		/* Fall through */
	case SUBSET:
		ep->s1 = ep->s2;
		do {
			if (!nextnnitem(ep)) {
				skipit = Yes;
				break;
			}
		} while(ep->s1&1);
		if (!skipit)
			s_downi(ep, ep->s1/2);
		break;

	default:
		Abort();

	}
	for (; skipit || Type(tree(ep->focus)) == Tex
			|| symbol(tree(ep->focus)) != Hole
				&& fwidth(noderepr(tree(ep->focus))[0]) >= 0;
		skipit = No) {
		if (skipit || !down(&ep->focus)) { /* Simulate nextnode */
			do {
				ich = ichild(ep->focus);
				if (!up(&ep->focus))
					goto endhit; /* Bah! */
			} while (ich >= nchildren(tree(ep->focus)));
			higher(ep);
			s_downi(ep, ich+1);
		}
	}
	if (symbol(tree(ep->focus)) == Hole) {
		ep->mode = WHOLE;
		return Yes;
	}
	while (ichild(ep->focus) == 1) {
		s_up(ep);
		ep->s1 = 1;
		ep->s2 = lenitem(ep);
		if (ep->s2 >= 0) {
			ep->mode = FHOLE;
			return Yes;
		}
	}
	ep->s1 = 2*ichild(ep->focus);
	s_up(ep);
	previtem(ep) || Abort();
	if (isunititem(ep)) {
		ep->mode = VHOLE;
		ep->s2 = lenitem(ep);
		return Yes;
	}
	higher(ep);
 endhit:
	for (;;) {
		lastnnitem(ep);
		if (isunititem(ep))
			break;
		s_downi(ep, ep->s1/2);
	}
	ep->mode = (ep->s1&1) ? FHOLE : VHOLE;
	ep->s2 = lenitem(ep);
	return Yes;
}
