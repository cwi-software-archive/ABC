/* SccsId: @(#)scrn.c	1.8 1/4/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Screen management package, higher level routines.
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

cell *gettop();
extern int focy;
extern int focx;

Visible int winheigth;
Visible int linelength;

Hidden cell *tops;

/*
 * Actual screen update.
 */

Visible Procedure
actupdate(flag)
	bool flag; /* Yes if called from final screen update */
{
	register cell *p;
	register int diff;
	register int curlno;
	register int delcnt = 0; /* Lines deleted during the process. */
		/* Used as offset for lines that are on the screen. */
	char buf[10];

	focy = Nowhere;
	for (p = gettop(tops), curlno = 0; p && curlno < winheigth;
		curlno += Space(p), p = p->c_link) {
		if (flag) {
			p->c_newfocus = No;
			p->c_newvhole = 0;
		}
		move(curlno, 0);
		if (p->c_onscreen != Nowhere && Space(p) == Oldspace(p)) {
			/* Old comrade */
			diff = p->c_newindent - p->c_oldindent;
			while (curlno+delcnt < p->c_onscreen) { /* Get him here */
				rdeleteln();
				++delcnt;
			}
			for (; diff < 0; ++diff)
				delch();
			for (; diff > 0; --diff) {
				insch(' ');
				addch(' '); /* This is optimized by FAKEcurses! */
			}
			if (p->c_oldfocus || p->c_newfocus
				|| (p->c_oldindent != p->c_newindent
						|| p->c_onscreen+Space(p) > winheigth)
					&& (Space(p) > 1 || Oldspace(p) > 1)) {
				/* Take no risk -- redraw it anyway */
					/* (FAKEcurses will optimize) */
				move(curlno, p->c_newindent);
				outline(p, flag);
				clrtoeol();
			}
		}
		else { /* New guy, make him toe the line */
			delcnt = makeroom(p, curlno, delcnt);
			for (diff = p->c_newindent; diff > 0; --diff)
				addch(' ');
			outline(p, flag);
			clrtoeol();
		}
		p->c_onscreen = curlno;
		p->c_oldindent = p->c_newindent;
		p->c_oldvhole = p->c_newvhole;
		p->c_oldfocus = p->c_newfocus;
	}
	move(curlno, 0);
	curlno += delcnt;
	while (delcnt > 0) { /* Insert new blank lines */
		rinsertln();
		--delcnt;
	}
	for (; curlno < winheigth; ++curlno) { /* Clear lines not affected */
		move(curlno, 0);
		clrtoeol();
	}
	if (focy != Nowhere)
		move(focy, focx);
	refresh();
	while (p) { /* Remove old memories */
		if (p->c_onscreen != Nowhere)
			debug("[Garbage removed from screen list]");
		p->c_onscreen = Nowhere;
		p = p->c_link;
	}
}


/*
 * Make room for possible insertions.
 * (If a blank line is inserted, it may be necessary to delete lines
 * further on the screen.)
 */

Hidden Procedure
makeroom(p, curlno, delcnt)
	register cell *p;
	register int curlno;
	register int delcnt;
{
	register int here = 0;
	register int need = Space(p);
	register int amiss;
	int avail;

	Assert(p);
	do {
		p = p->c_link;
		if (!p)
			return delcnt;
	} while (p->c_onscreen == Nowhere);
	here = p->c_onscreen - delcnt;
	avail = here - curlno;
	amiss = need - avail;
	if (dflag)
		debug("[makeroom: curlno=%d, delcnt=%d, here=%d, avail=%d, amiss=%d]",
			curlno, delcnt, here, avail, amiss);
	if (amiss <= 0)
		return delcnt;
	if (amiss > delcnt) {
		for (; p; p = p->c_link) {
			if (p->c_onscreen != Nowhere) {
				while(amiss > delcnt && p->c_onscreen - delcnt > here) {
					move(here, 0);
					rdeleteln();
					if (dflag)
						debug("[Delete line %d]", here);
					++delcnt;
				}
				p->c_onscreen += -delcnt + amiss;
				here = p->c_onscreen - amiss;
				if (p->c_onscreen >= winheigth)
					p->c_onscreen = Nowhere;
			}
			here += Space(p);
		}
		/* Now for all p encountered whose p->c_onscreen != Nowhere,
		/* p->c_onscreen - amiss is its actual position. */
		while (amiss > delcnt) {
			move(winheigth - amiss, 0);
			rdeleteln();
			if (dflag)
				debug("[Delete end line]");
			++delcnt;
		}
	}
	/* Now amiss <= delcnt */
	move(curlno + avail, 0);
	delcnt -= amiss;
	for (; amiss > 0; --amiss)
		rinsertln();
	move(curlno, 0);
	return delcnt;
}


/*
 * Routine called for every change in the screen.
 */

Visible Procedure
virtupdate(oldep, newep, highest)
	environ *oldep;
	environ *newep;
	int highest;
{
	environ old;
	environ new;
	register int oldlno;
	register int newlno;
	register int oldlcnt;
	register int newlcnt;
	register int i;

	if (!oldep) {
		highest = 1;
		clear();
		discard(tops);
		tops = Cnil;
		Ecopy(*newep, old);
	}
	else
		Ecopy(*oldep, old);
	Ecopy(*newep, new);

	savefocus(&new);

	oldlcnt = fixlevels(&old, &new, highest);
	newlcnt = -width(tree(new.focus));
	if (newlcnt < 0)
		newlcnt = 0;
	i = -width(tree(old.focus));
	if (i < 0)
		i = 0;
	newlcnt -= i - oldlcnt;
		/* Offset newlcnt as much as oldcnt is offset */
	
	oldlno = Ycoord(old.focus);
	newlno = Ycoord(new.focus);
	if (!atlinestart(&old))
		++oldlcnt;
	else
		++oldlno;
	if (!atlinestart(&new))
		++newlcnt;
	else
		++newlno;
	Assert(oldlno == newlno);

	if (dflag)
		debug("[At line %d, replacing %d lines by %d]",
			oldlno, oldlcnt, newlcnt);
	tops = replist(tops, build(new.focus, newlcnt), oldlno, oldlcnt);

	setfocus(tops); /* Incorporate the information saved by savefocus */

	Erelease(old);
	Erelease(new);
}


Hidden bool
atlinestart(ep)
	environ *ep;
{
	register string repr = noderepr(tree(ep->focus))[0];

	return Fw_negative(repr);
}


/*
 * Make the two levels the same, and make sure they both are line starters
 * if at all possible.  Return the OLD number of lines to be replaced.
 * (0 if the whole unit has no linefeeds.)
 */

Hidden int
fixlevels(oldep, newep, highest)
	register environ *oldep;
	register environ *newep;
	register int highest;
{
	register int oldpl = pathlength(oldep->focus);
	register int newpl = pathlength(newep->focus);
	register bool intraline = No;
	register int w;

	if (oldpl < highest)
		highest = oldpl;
	if (newpl < highest)
		highest = newpl;
	while (oldpl > highest) {
		up(&oldep->focus) || Abort();
		--oldpl;
	}
	while (newpl > highest) {
		up(&newep->focus) || Abort();
		--newpl;
	}
	if (Ycoord(newep->focus) != Ycoord(oldep->focus) ||
		Level(newep->focus) != Level(newep->focus)) {
		/* Inconsistency found.  */
		Assert(highest > 1); /* Inconsistency at top level. Stop. */
		return fixlevels(oldep, newep, 1); /* Try to recover. */
	}
	intraline = width(tree(oldep->focus)) >= 0
		&& width(tree(newep->focus)) >= 0;
	while (!atlinestart(oldep) || !atlinestart(newep)) {
		/* Find beginning of lines for both */
		if (!up(&newep->focus)) {
			Assert(!up(&newep->focus));
			break;
		}
		--oldpl;
		up(&oldep->focus) || Abort();
		--newpl;
	}
	if (intraline)
		return atlinestart(oldep);
	w = width(tree(oldep->focus));
	return w < 0 ? -w : 0;
}


/*
 * Initialization code.
 */

Visible Procedure
initshow()
{
	winheigth = LINES-1;
	linelength = COLS;
}


/*
 * Extermination code.
 */

Visible Procedure
endshow()
{
	register cell *p;
	register int last = winheigth;

	for (p = tops; p; p = p->c_link) {
		if (p->c_onscreen != Nowhere)
			last = p->c_onscreen + Oldspace(p);
	}
	if (last > winheigth)
		last = winheigth;
	discard(tops);
	tops = Cnil;
	move(last, 0);
	clrtobot();
	move(last, 0);
	refresh();
}


/*
 * Translate a cursor position in tree coordinates.
 *
 * ***** DOESN'T WORK IF SCREEN INDENT DIFFERS FROM TREE INDENT! *****
 */

Visible bool
backtranslate(py, px)
	int *py;
	int *px;
{
	cell *p;
	int y = *py;
	int x = *px;
	int i;

	for (i = 0, p = tops; p; ++i, p = p->c_link) {
		if (p->c_onscreen != Nowhere
			&& y >= p->c_onscreen && y < p->c_onscreen + Space(p)) {
			*px += (y - p->c_onscreen) * linelength;
			*py = i;
			if (p->c_oldvhole && (y > focy || y == focy && x >= focx))
				--*px; /* Correction if beyond Vhole on same logical line */
			return Yes;
		}
	}
	error("The cursor isn't pointing at a part of the buffer");
	return No;
}


/*
 * Extensions for curses -- really delete or insert a line.
 */

extern char *DL; /* Initialized but not used by curses! */
extern char *AL;

int _putchar(); /* Defined in curses library */

Hidden Procedure
rdeleteln()
{
#ifdef WINDOW
	if (DL) {
		refresh();
		wdeleteln(curscr);
		tputs(DL, LINES - curscr->_cury, _putchar);
	}
#endif WINDOW
	deleteln();
}

Hidden Procedure
rinsertln()
{
#ifdef WINDOW
	if (AL) {
		refresh();
		winsertln(curscr);
		tputs(AL, LINES - curscr->_cury, _putchar);
	}
#endif WINDOW
	insertln();
}
