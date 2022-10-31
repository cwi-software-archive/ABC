/* SccsId: @(#)term.c	1.10 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Terminal dependent code except curses/termlib simulation.
 *
 * This file may require an amount of hacking if you want to tailor the
 * B system to another type of terminal; however, for most terminals that
 * can be used with curses(3), there should be no problem.
 */

#include <ctype.h>
#include <signal.h>
#include <setjmp.h>

#include "b.h"

#include "curses.h"
	/* Implies #include <stdio.h> and <sgtty.h>, which we need here */


#define DC1 021
#define ESC 033

#define Strnequ(s1, s2, n) !strncmp(s1, s2, n)
#define Strequ(s1, s2) !strcmp(s1, s2)


/*
 * Find out whether we know enough about the terminal.
 * (I.e., it is an HP.)
 */

Visible bool
ttylookup()
{
	extern char ttytype[]; /* Set by curses' initscr(). */

	return Strnequ(ttytype, "hp", 2) || Strnequ(ttytype, "2621", 4);
}


/*
 * Initialization code.
 * Also call initialization code of other terminal-dependent modules.
 */

Visible Procedure
initterm()
{
	initgetc();
	initscr();
	noecho();
	nonl();
	crmode();
	initshow();
}


/*
 * Extermination code, reverse of initterm().
 */

Visible Procedure
endterm()
{
	endshow();
	endwin();
	endgetc();
}


/*
 * Timeout mechanism
 */

Hidden jmp_buf tout;

Hidden Procedure
timeout(sig)
	int sig;
{
	/* Assume sig > 0 */
	longjmp(tout, sig);
}


/*
 * Cursor sensing routine -- deliver cursor y, x coordinates.
 * Return No if cursor didn't move or if coordinates are unknown.
 *
 * ***** CURSES INTERNALS DEPENDENT CODE *****
 * The new (y, x) coordinates are plugged in the curscr structure,
 * so the refresh() routine correctly knows from where it must start.
 *
 * ***** HP DEPENDENT CODE *****
 * Cursor sensing is not defined terminal-independently in termcap(5).
 * Therefore we necessarily resort to terminal-dependent code.
 * If the terminal type is not that of the terminal we know, the
 * routine returns No.
 */

Visible bool
sense(yp, xp)
	int *yp, *xp;
{
	bool ok = No;
	char buf[100];
	string bufp;
	int (*prevsig)() = SIG_DFL;

	if (!ttylookup()) {
		error("Sorry -- no GOTO on your terminal type");
		return No; /* Not a terminal type we know of */
	}

	putchar(ESC);
	putchar('`'); /* Screen-related, not memory-related coordinates */
	putchar(DC1);
	fflush(stdout);

	if (setjmp(tout)) { /* Timeout occurred */
		if (!ok)
			error("Cursor sense time-out");
		return ok;
	}

	prevsig = signal(SIGALRM, timeout);
	alarm(2); /* Give up after 1 a 2 seconds (see setjmp above) */

	for (bufp = buf; ; ++bufp) {
		*bufp = getch();
		if (*bufp == '\r' || *bufp == '\n')
			break;
	}
	alarm(0); /* Stop alarm clock */
	signal(SIGALRM, prevsig);
	*bufp = 0;
	ok = sscanf(buf, "\33&a%dc%dY", xp, yp) == 2;


	if (!ok) {
		error("Cursor sense failed");
		*yp = *xp = 0;
	}
	/* Tell curses where the cursor now is */
#ifdef WINDOW
	curscr->_cury = *yp;
	curscr->_curx = *xp;
#else WINDOW
	move(0, 0);
	refresh();
	move(*yp, *xp);
	refresh();
#endif WINDOW
	return ok;
}
