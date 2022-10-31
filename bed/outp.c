/* SccsId: @(#)outp.c	1.7 1/4/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Screen management package, lower level output part.
 */

#include "b.h"
#include "bobj.h"
#include "node.h"
#include "supr.h"
#include "gram.h"
#include "cell.h"

#include "curses.h"

#ifdef bool
#undef bool /* Defeat curses which has `#define bool char' */
#endif


extern bool dflag;


/*
 * Variables used for communication with outfocus.
 */

Hidden node thefocus;
Hidden environ wherebuf;
Hidden environ *where = &wherebuf;
Hidden bool realvhole;
Hidden int multiline; /* Heigth of focus */
Hidden int yfocus;

Visible int focy; /* Where the cursor must go */
Visible int focx;


/*
 * Save position of the focus for use by outnode/outfocus.
 */

Visible Procedure
savefocus(ep)
	register environ *ep;
{
	register int sym;
	register int w;

	realvhole = No;
	thefocus = Nnil;
	multiline = 0;
	yfocus = Ycoord(ep->focus);
	w = focoffset(ep);
	if (w < 0)
		yfocus += -w;
	w = focwidth(ep);
	if (w < 0) {
		multiline = -w;
		if (focchar(ep) == '\n')
			++yfocus;
		else
			++multiline;
		if (dflag)
			debug("[multiline=%d]", multiline);
		return;
	}
	if (ep->mode == WHOLE) {
		sym = symbol(tree(ep->focus));
		if (sym == Optional)
			ep->mode = ATBEGIN;
	}
	switch(ep->mode) {
	case VHOLE:
		if (ep->s1&1)
			ep->mode = FHOLE;
	case ATBEGIN:
	case ATEND:
	case FHOLE:
		ritevhole(ep);
		if (dflag)
			dbmess(ep);
		switch (ep->mode) {
		case ATBEGIN:
		case FHOLE:
			sym = symbol(tree(ep->focus));
			if (sym == Hole && (ep->mode == ATBEGIN || ep->s2 == 0)) {
				ep->mode = WHOLE;
				break;
			}
			/* Fall through */
		case VHOLE:
		case ATEND:
			leftvhole(ep);
			realvhole = 1 + ep->spflag;
		}
	}
	touchpath(&ep->focus); /* Make sure it is a unique pointer */
	thefocus = tree(ep->focus); /* No copy; used for comparison only! */
	where->mode = ep->mode;
	where->s1 = ep->s1;
	where->s2 = ep->s2;
	where->s3 = ep->s3;
	where->spflag = ep->spflag;
}


/*
 * Incorporate the information saved about the focus.
 */

Visible Procedure
setfocus(tops)
	register cell *tops;
{
	register cell *p;
	register int i;

	for (p = tops, i = 0; i < yfocus; ++i, p = p->c_link) {
		if (!p) {
			debug("[Focus lost (setfocus)]");
			return;
		}
	}
	p->c_newvhole = realvhole;
	i = multiline;
	do {
		p->c_newfocus = Yes;
		p = p->c_link;
	} while (--i > 0);
}


/*
 * Output a line of text.
 */

Visible Procedure
outline(p, flag)
	register cell *p;
	register bool flag; /* Yes if called from final update */
{
	register node n = p->c_data;

	if (flag) {
		thefocus = Nnil;
		multiline = 0;
	}
	if (!p->c_newfocus)
		outnode(n);
	else if (multiline) {
		if (focy == Nowhere) {
			getyx(stdscr, focy, focx);
			if (focx == linelength) {
				++focy;
				focx = 0;
			}
		}
		standout();
		outnode(n);
		standend();
	}
	else if (n == thefocus)
		outfocus(n);
	else
		outnode(n);
}


/*
 * Output a node.
 */

Hidden Procedure
outnode(n)
	register node n;
{
	register string *rp = noderepr(n);
	register int nch = nchildren(n);
	register int w;
	register int i;

	if (rp[0])
		outstring(rp[0]);
	for (i = 1; i <= nch; ++i) {
		if (!outchild(n, i))
			return;
		if (rp[i])
			outstring(rp[i]);
	}
}


/*
 * Output a node's child.
 * Return No if it contained a newline (to stop the parent).
 */

Hidden bool
outchild(n, i)
	register node n;
	register int i;
{
	register node nn = child(n, i);
	register int w;

	if (Type(nn) == Tex) {
		outstring(Str((value)nn));
		return Yes;
	}
	w = width(nn);
	if (w < 0 && Fw_negative(noderepr(nn)[0]))
		return No;
	if (nn == thefocus)
		outfocus(nn);
	else
		outnode(nn);
	return w >= 0;
}


/*
 * Output a string, such as might be found in a representation array.
 * (That is, a nil pointer is ignored, and so are non-printing characters.)
 */

Hidden Procedure
outstring(str)
	register string str;
{
	auto int y;
	auto int x1;
	register int x;
	
	if (!str)
		return;
	getyx(stdscr, y, x1);
	if (y >= winheigth)
		return;
	x = x1;
	for (; *str; ++str) {
		if (*str >= ' ' && *str < 0177) {
			if (x >= linelength) {
				++y;
				x = 0;
				move(y, x);
				if (y >= winheigth)
					return;
			}
			addch(*str);
			++x;
		}
	}
}


/*
 * Output the focus.
 */

#define MASK 0200

Hidden Procedure
outfocus(n)
	register node n;
{
	register int w = width(n);
	register string buf = malloc(4 + (w < 0 ? linelen(n) : w));
		/* Some extra for spflag and vhole */
	auto string bp = buf;
	register string cp;
	register int c;

	Assert(n == thefocus);
	if (!buf)
		syserr("outfocus: malloc");
	focsmash(&bp, n);
	*bp = 0;
	for (cp = buf; *cp && !(*cp&MASK); ++cp)
		;
	c = *cp;
	*cp = 0;
	outstring(buf);
	*cp = c;
	bp = cp;
	for (; *cp && (*cp&MASK); ++cp)
		*cp &= ~MASK;
	c = *cp;
	*cp = 0;
	getyx(stdscr, focy, focx);
	if (focx >= linelength) {
		focx = 0;
		++focy;
	}
	standout();
	outstring(bp);
	standend();
	*cp = c;
	outstring(cp);
	free(buf); /* Don't forget! */
}


/*
 * Smash -- produce a linear version of a node in a buffer (which had
 * better be long enough!).  The buffer pointer is moved to the end of
 * the resulting string.
 * Care is taken to represent the focus.
 * Characters in the focus have their uppr bit set.
 */

#define Outvhole() \
	(where->spflag && strsmash(pbuf, " ", 0), strsmash(pbuf, "?", MASK))

Hidden Procedure
focsmash(pbuf, n)
	node n;
{
	value v;
	string str;
	register string *rp;
	register int maxs2;
	register int i;
	register bool ok;
	register int j;
	register int mask;

	switch (where->mode) {

	case WHOLE:
		smash(pbuf, n, MASK);
		break;

	case ATBEGIN:
		Outvhole();
		smash(pbuf, n, 0);
		break;

	case ATEND:
		smash(pbuf, n, 0);
		Outvhole();
		break;

	case VHOLE:
		if (!(where->s1&1)) {
			v = (value) child(n, where->s1/2);
			Assert(Type(v) == Tex);
			subsmash(pbuf, Str(v), where->s2, 0);
			Outvhole();
			strsmash(pbuf, Str(v) + where->s2, 0);
			break;
		}
		/* Else, fall through */
	case FHOLE:
		rp = noderepr(n);
		maxs2 = 2*nchildren(n) + 1;
		for (ok = Yes, i = 1; ok && i <= maxs2; ++i) {
			if (i&1) {
				if (i == where->s1) {
					subsmash(pbuf, rp[i/2], where->s2, 0);
					Outvhole();
					if (rp[i/2])
						strsmash(pbuf, rp[i/2] + where->s2, 0);
				}
				else
					strsmash(pbuf, rp[i/2], 0);
			}
			else
				ok = chismash(pbuf, n, i/2, 0);
		}
		break;

	case SUBRANGE:
		rp = noderepr(n);
		maxs2 = 2*nchildren(n) + 1;
		for (ok = Yes, i = 1; ok && i <= maxs2; ++i) {
			if (i&1) {
				if (i == where->s1) {
					subsmash(pbuf, rp[i/2], where->s2,0);
					if (rp[i/2])
						subsmash(pbuf, rp[i/2] + where->s2,
							where->s3 - where->s2 + 1, MASK);
					if (rp[i/2])
						strsmash(pbuf, rp[i/2] + where->s3 + 1, 0);
				}
				else
					strsmash(pbuf, rp[i/2], 0);
			}
			else if (i == where->s1) {
				v = (value)child(n, i/2);
				Assert(Type(v) == Tex);
				str = Str(v);
				subsmash(pbuf, str, where->s2, 0);
				subsmash(pbuf, str + where->s2, where->s3 - where->s2 + 1,
					MASK);
				strsmash(pbuf, str + where->s3 + 1, 0);
			}
			else
				ok = chismash(pbuf, n, i/2, 0);
		}
		break;

	case SUBLIST:
		for (ok = Yes, j = where->s3; j > 0; --j) {
			rp = noderepr(n);
			maxs2 = 2*nchildren(n) - 1;
			for (i = 1; ok && i <= maxs2; ++i) {
				if (i&1)
					strsmash(pbuf, rp[i/2], MASK);
				else
					ok = chismash(pbuf, n, i/2, MASK);
			}
			if (ok)
				n = child(n, nchildren(n));
		}
		if (ok)
			smash(pbuf, n, 0);
		break;

	case SUBSET:
		rp = noderepr(n);
		maxs2 = 2*nchildren(n) + 1;
		mask = 0;
		for (ok = Yes, i = 1; ok && i <= maxs2; ++i) {
			if (i == where->s1)
				mask = MASK;
			if (i&1)
				strsmash(pbuf, rp[i/2], mask);
			else
				ok = chismash(pbuf, n, i/2, mask);
			if (i == where->s2)
				mask = 0;
		}
		break;

	default:
		Abort();
	}
}

Hidden Procedure
smash(pbuf, n, mask)
	register string *pbuf;
	register node n;
	register int mask;
{
	register string *rp;
	register int i;
	register int nch;
	register node nn;

	rp = noderepr(n);
	strsmash(pbuf, rp[0], mask);
	nch = nchildren(n);
	for (i = 1; i <= nch; ++i) {
		if (!chismash(pbuf, n, i, mask))
			break;
		strsmash(pbuf, rp[i], mask);
	}
}

Hidden Procedure
strsmash(pbuf, str, mask)
	register string *pbuf;
	register string str;
	register int mask;
{
	if (!str)
		return;
	for (; *str; ++str) {
		if (*str >= ' ' && *str < 0177)
			**pbuf = *str|mask, ++*pbuf;
	}
}

Hidden Procedure
subsmash(pbuf, str, len, mask)
	register string *pbuf;
	register string str;
	register int len;
	register int mask;
{
	if (!str)
		return;
	for (; len > 0 && *str; --len, ++str) {
		if (*str >= ' ' && *str < 0177)
			**pbuf = *str|mask, ++*pbuf;
	}
}


/*
 * Smash a node's child.
 * Return No if it contained a newline (to stop the parent).
 */

Hidden bool
chismash(pbuf, n, i, mask)
	register string *pbuf;
	register node n;
	register int i;
{
	register node nn = child(n, i);
	register int w;

	if (Type(nn) == Tex) {
		strsmash(pbuf, Str((value)nn), mask);
		return Yes;
	}
	w = width(nn);
	if (w < 0 && Fw_negative(noderepr(nn)[0]))
		return No;
	if (nn == thefocus)
		focsmash(pbuf, nn);
	else
		smash(pbuf, nn, mask);
	return w >= 0;
}
