/* SccsId: @(#)getc.c	1.9 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Keyboard input.
 */

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <ctype.h>

#include "b.h"
#include "keys.h"

#include "curses.h"


extern bool hushbaby;

#define DC1 021
#define ESC 033


/*
 * Structure describing function keys and arrow keys as may be defined
 * in /etc/termcap.
 * It can also be used for general aliasing of control keys.
 * Because it turned out that some VT100s (look-alikes?) send ESC-[-A etc.,
 * where termcap says ESC-O-A, these are also predefined.
 * The same is done for the unshifted arrow keys of some HP 2621's
 * (with "A" strap on, causing all keys to transmit their escape sequence
 * rather than executing it).
 *
 */

Hidden struct {
	string k_entry;
	string k_name;
	int k_value;
} keystrokes[] = {
	/* Arrow keys from termcap */
	{"\33t\r", "ku", UPARROW},
	{"\33u\r", "kl", PREVIOUS},
	{"\33v\r", "kr", NEXT},
	{"\33w\r", "kd", DOWNARROW},

	/* Function keys from termcap */
	{"\33p\r", "k1", F1},
	{"\33q\r", "k2", F2},
	{"\33r\r", "k3", F3},
	{"\33s\r", "k4", F4},
	{"\33t\r", "k5", F5},
	{"\33u\r", "k6", F6},
	{"\33v\r", "k7", F7},
	{"\33w\r", "k8", F8},
	
	/* Unshifted arrow keys for new HP 2621 with strap A set */
	{"\33A", 0, UPARROW},
	{"\33B", 0, DOWNARROW},
	{"\33C", 0, NEXT},
	{"\33D", 0, PREVIOUS},
	
	/* VT100 arrow keys (according to /etc/termcap) */
	{"\33OA", 0, UPARROW},
	{"\33OB", 0, DOWNARROW},
	{"\33OC", 0, NEXT},
	{"\33OD", 0, PREVIOUS},
	
	/* VT100 (look-alike?) arrow keys */
	{"\33[A", 0, UPARROW},
	{"\33[B", 0, DOWNARROW},
	{"\33[C", 0, NEXT},
	{"\33[D", 0, PREVIOUS},
	
	/* VT100 function keys */
	{"\33OP", 0, F1},
	{"\33OQ", 0, F2},
	{"\33OR", 0, F3},
	{"\33OS", 0, F4},
	
	/* VT100 (look-alike?) function keys */
	
	{"\33[P", 0, F1},
	{"\33[Q", 0, F2},
	{"\33[R", 0, F3},
	{"\33[S", 0, F4},
};


/*
 * Get a command character.
 */

Visible int
inchar()
{
	char buf[10]; /* Keeps multiple keystrokes */
	register int inbuf; /* Index in buf */
	register int c; /* First character read */
	register int i;
	register int firsthit;
	register int lasthit;
	register string cp;

	c = getch();
	for (;;) {
		inbuf = 1;
		buf[0] = c;
		firsthit = lasthit = -1;
		for (i = 0; i < (sizeof keystrokes) / (sizeof keystrokes[0]); ++i) {
			cp = keystrokes[i].k_entry;
			if (cp && cp[0] == c) {
				if (firsthit < 0)
					firsthit = i;
				lasthit = i;
			}
		}
		if (firsthit < 0)
			return c;
		while (keystrokes[firsthit].k_entry[inbuf]) {
			c = getch();
			buf[inbuf] = c;
			++inbuf;
			for (i = firsthit, firsthit = -1; i <= lasthit; ++i) {
				cp = keystrokes[i].k_entry;
				if (cp && strncmp(cp, buf, inbuf) == 0) {
					firsthit = i;
					break;
				}
			}
			if (firsthit < 0)
				break;
		}
		if (firsthit >= 0)
			return keystrokes[firsthit].k_value;
	}
}


/*
 * Static storage for KS and KE strings.
 */

Hidden char *KS;
Hidden char *KE;


/*
 * Initialization for inchar -- read description of terminal's
 * function keys from termcap entry.
 */

Visible Procedure
initgetc()
{
	string tgetstr();
	char buffer[1024];
	static char area[1024]; /* Constant dictated by termcap manual entry */
	auto string endarea = area;
	register string term = getenv("TERM");
	register int i;
	register string entry;

	if (!term) {
		fprintf(stderr, "TERM variable not set\n");
		exit(1);
	}
	switch (tgetent(buffer, term)) {
	
	default:
		fprintf(stderr, "Bad tgetent() return value\n");
	case -1:
		fprintf(stderr, "Cannot read terminal description database\n");
	case 0:
		fprintf(stderr, "No description for your terminal\n");
		exit(1);
		
	case 1:
		break;
	}
	for (i = 0; i < (sizeof keystrokes) / (sizeof keystrokes[0]); ++i) {
		if (keystrokes[i].k_name) {
			entry = tgetstr(keystrokes[i].k_name, &endarea);
			if (entry && entry[0])
				keystrokes[i].k_entry = entry;
		}
	}
	KS = tgetstr("ks", &endarea);
	KE = tgetstr("ke", &endarea);
	if (KS)
		fputs(KS, stdout);
}


/*
 * Termination code.
 * Currently only output termcap's "ke" string to turn off function keys.
 */

endgetc()
{
	if (KE)
		fputs(KE, stdout);
}
