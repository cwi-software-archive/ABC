/* SccsId: %W% %G%
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * A package whose user interface is compatible with
 * a subset of curses(3); i.e., if you use this package you can also
 * use curses(3) with (almost) no changes to your program.
 * (The initialization calls may have to be changed, though).
 *
 * ***** DISCLAIMER *****
 * Use of this package with other software than the `B editor' can not
 * be guaranteed and is discouraged.
 */


#include "curses.h"

typedef int bool;

#define Yes 1
#define No 0

#define PUTS(s) fputs(s, stdout)

static char buf[LINES][COLS+1];
static char *(line[LINES]) = {
	buf[ 0], buf[ 1], buf[ 2], buf[ 3], buf[ 4], buf[ 5],
	buf[ 6], buf[ 7], buf[ 8], buf[ 9], buf[10], buf[11],
	buf[12], buf[13], buf[14], buf[15], buf[16], buf[17],
	buf[18], buf[19], buf[20], buf[21], buf[22], buf[23],
}; /* Silly! */

typedef struct point {
	int y;
	int x;
} point;

static point virtual;
static point actual;
static int so_virtual;
static int so_actual;
static bool ins_mode;

move(y, x)
	register int y;
	register int x;
{
	if (0 <= y && y < LINES && 0 <= x && x < COLS) {
		virtual.y = y;
		virtual.x = x;
	}
}

addch(ch)
	register int ch;
{
	register int c;
	register char *lp;

	if (' ' <= ch && ch < 0177 && virtual.x < COLS) {
		c = ch|so_virtual;
		lp = line[virtual.y];
		if ((lp[virtual.x] & 0377) != c) {
			lp[virtual.x] = c;
			actmove(No, Yes);
			putchar(ch);
			++actual.x;
			if (actual.x >= COLS)
				actfix();
		}
		++virtual.x;
	}
}

insch(ch)
	register int ch;
{
	register int i;
	register char *lp;
	register int c;

	if (' ' <= ch && ch < 0177 && virtual.x < COLS) {
		lp = line[virtual.y];
		for (i = COLS-1; i > virtual.x; --i)
			lp[i] = lp[i-1];
		c = ch|so_virtual;
		lp[virtual.x] = c;
		actmove(Yes, Yes);
		putchar(ch);
		++actual.x;
		if (actual.x >= COLS)
			actfix();
	}
}

static
actmove(ins, so)
	register bool ins;
	register bool so;
{
	if (virtual.y != actual.y) {
		putchar('\r');
		actual.x = 0;
		if (virtual.y == 0) {
			PUTS("\033H");
			actual.y = 0;
			actual.x = 0;
		}
		if (virtual.y <= actual.y + 2) {
			while (virtual.y > actual.y) {
				putchar('\n');
				++actual.y;
				so_actual = 0;
			}
		}
	}
	if (!ins_mode && virtual.y == actual.y) {
		register int start = actual.x;
		if (virtual.x < start)
			start = 0;
		if (virtual.x > start && virtual.x <= start + 5) {
			/* Optimize small forward steps */
			register bool ok = Yes;
			register int i;
			register char *lp = line[virtual.y];
	
			for (i = start; i < virtual.x; ++i) {
				if ((lp[i]&0200) != so_actual) {
					ok = No;
					break;
				}
			}
			if (ok) {
				if (start < actual.x) {
					putchar('\r');
					actual.x = 0;
				}
				for (i = actual.x; i < virtual.x; ++i)
					lp[i] ? putchar(lp[i]) : putchar(' ');
				actual.x = virtual.x;
			}
		}
	}
	if (virtual.y != actual.y || virtual.x != actual.x) {
		PUTS("\033&a");
		if (virtual.y != actual.y) {
			putint(virtual.y);
			putchar('y');
		}
		putint(virtual.x);
		putchar('C');
	}
	if (actual.y != virtual.y)
		so_actual = 0;
	actual.y = virtual.y;
	actual.x = virtual.x;
	if (ins != ins_mode) {
		if (ins)
			PUTS("\033Q");
		else
			PUTS("\033R");
		ins_mode = ins;
	}
	if (so && so_virtual != so_actual) {
		if (so_virtual)
			PUTS("\033&dB");
		else
			PUTS("\033&d@");
		so_actual = so_virtual;
	}
}

static
actfix()
{
	if (actual.x >= COLS) {
		++actual.y;
		actual.x = 0;
		so_actual = 0;
	}
	if (actual.y >= LINES)
		/* SHOULD SCROLL! */;
}

static
putint(n)
	register int n;
{
	if (n >= 10)
		putchar('0'+(n/10));
	putchar('0'+(n%10));
}

char
peekch(y, x)
	register int y;
	register int x;
{
	if (0 <= y && y < LINES && 0 <= x && x < COLS)
		return line[y][x];
}

standout()
{
	so_virtual = 0200;
}

standend()
{
	so_virtual = 0;
}

addstr(str)
	register char *str;
{
	if (!str)
		return;
	for (; *str; ++str)
		addch(*str);
}

clear()
{
	register int i;

	PUTS("\033H\033J");
	actual.y = actual.x = 0;
	virtual.y = actual.y, virtual.x = actual.x;
	so_actual = 0;
	for (i = LINES-1; i >= 0; --i)
		blank(line[i], 0);
}

static bool
blank(lp, i)
	register char *lp;
	register int i;
{
	register bool cleared = No;

	for (; i <= COLS; ++i) {
		if (lp[i]) {
			cleared = Yes;
			lp[i] = 0;
		}
	}
	return cleared;
}

static bool
isblank(lp, i)
	register char *lp;
	register int i;
{
	register int cleared = 0;
	for (; i < COLS; ++i) {
		if (lp[i])
			return No;
	}
	return Yes;
}

clrtobot()
{
	register int i;
	register bool cleared;

	cleared = blank(line[virtual.y], virtual.x);
	actmove(ins_mode, No);
	PUTS("\033J");
}

clrtoeol()
{
	if (!blank(line[virtual.y], virtual.x))
		return; /* It was already blank */
	actmove(ins_mode, No);
	PUTS("\033K");
}

delch()
{
	register int i;
	register char *lp;
	register int cleared = 0;

	lp = line[virtual.y];
	for (i = virtual.x; i <= COLS; ++i) {
		cleared |= lp[i];
		lp[i] = lp[i+1];
	}
	if (!cleared)
		return;
	actmove(ins_mode, No);
	PUTS("\033P");
}

deleteln()
{
	register int i;
	register char *lp;
	register bool useful = No;

	virtual.x = 0;
	lp = line[virtual.y];
	for (i = virtual.y; i < LINES-1; ++i) {
		line[i] = line[i+1];
		if (!useful)
			useful = !isblank(line[i], 0);
	}
	line[LINES-1] = lp;
	useful |= blank(lp, 0);
	if (!useful)
		return;
	actmove(ins_mode, No);
	PUTS("\033M");
	so_actual = 0;
}

insertln()
{
	register int i;
	register char *lp;
	register bool useful = No;

	virtual.x = 0;
	lp = line[LINES-1];
	for (i = LINES-1; i > virtual.y; --i) {
		line[i] = line[i-1];
		if (!useful)
			useful = !isblank(line[i], 0);
	}
	line[virtual.y] = lp;
	useful |= blank(lp, 0);
	if (!useful)
		return;
	actmove(ins_mode, No);
	PUTS("\033L");
	so_actual = 0;
}

refresh()
{
	actmove(No, Yes);
	fflush(stdout);
}

static struct sgttyb ttysave;

initscr()
{
	struct sgttyb ttynew;
	ioctl(2, TIOCGETP, &ttysave);
	ttynew = ttysave;
	ttynew.sg_flags = (ttynew.sg_flags & ~ECHO & ~CRMOD) | CBREAK;
	ioctl(2, TIOCSETN, &ttynew);
	actual.x = -1;
}

endwin()
{
	if (ttysave.sg_ospeed)
		ioctl(2, TIOCSETN, &ttysave);
}

_getyx(py, px)
	register int *py;
	register int *px;
{
	*py = virtual.y;
	*px = virtual.x;
}

char ttytype[] = "hp2621";
