/* SccsId: @(#)keys.h	1.7 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Function key and control character definitions.
 */


#define Ctl(x) ('x'&037)

/*
 * Commands bound to control characters.
 *
 * Not all control characters can be freely used:
 * ^Q and ^S are used by the Unix operating system
 * for output flow control, and ^Z is used by BSD
 * Unix systems for `job control'.
 *
 * Also note that ^H, ^I and ^M (and somtimes ^J) have their
 * own keys on most keyboards and thus usually have a strong
 * intuitive meaning.
 */

#define COPY	Ctl(C)
#define DELETE	Ctl(D)
#define GOTO	Ctl(G)
#define UNDO	Ctl(H)
#define ACCEPT	Ctl(I)
#define NEWLINE	Ctl(J)
#define REDRAW	Ctl(L)
#define RETURN	Ctl(M)
#define USEMACRO	Ctl(P)
#define SAVEMACRO	Ctl(R)
#define REDO	Ctl(U)
#define EXIT	Ctl(X)


/*
 * Values to be returned by 'inchar()' in "getc.c" for
 * terminal-dependent escape sequences.
 */

#define F1 ('p'|0200) /* F1, left up arrow */
#define F2 ('q'|0200) /* F2, left down arrow */
#define F3 ('r'|0200) /* F3, roll up */
#define F4 ('s'|0200) /* F4, roll down */
 
#define F5 ('t'|0200) /* F5, up arrow */
#define F6 ('u'|0200) /* F6, left arrow */
#define F7 ('v'|0200) /* F7, right arrow */
#define F8 ('w'|0200) /* F8, down arrow */


/*
 * Map of function keys to functions.
 */

#define WIDEN	F1
#define NARROW	F2
#define RNARROW	F3
#define EXTEND	F4

#define UPARROW	F5
#define PREVIOUS	F6
#define NEXT	F7
#define DOWNARROW	F8
