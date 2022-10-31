/* SccsId: @(#)unix.c	1.12 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- UNIX(*) interface, i.e. signal and tty fiddling.
 *
 * (*) UNIX is a trademark of Bell laboratories.
 */

#include <stdio.h>
#include <signal.h>
#include <sgtty.h>

#include "b.h" /* Only for definitions like bool, string, Hidden etc. */


extern bool slowterminal; /* Set for speeds <= 600 baud */
extern bool hushbaby; /* Set if no bells are to be heard */

#define Ctl(x) ('x'&037)

#ifndef QUIT
#define QUIT Ctl(\\)
#endif QUIT

#ifndef INTRCHAR
#define INTRCHAR '\177' /* DEL */
#endif INTRCHAR

#define REDRAW Ctl(L) /* From "keys.h" */

/*
 * Call exit code when signal arrives, then resend the signal.
 */

catch(sig)
	int sig;
{
	signal(sig, SIG_DFL);
	if (sig == SIGQUIT) { /* QUIT only resets terminal modes */
		endterm();
		endunix();
	}
	else {
		endall();
	}
	kill(getpid(), sig);
}


#ifdef SIGTSTP /* I.e., only at BSD systems. */
/*
 * Reset tty modes etc. when STOP signal arrives (control-Z).
 * This is like interrupt but the program may continue later
 * so we must not do all exit code).
 * We don't use the "new signal package" (though we do use the
 * new signals!), so if two signals arrive in a row we are just
 * out of luck.  This doesn't occur under normal circumstances,
 * and anyway we can't avoid this on V7 systems...
 * Note! Since curses' initscr() also executes signal(SIGTSTP, tstp),
 * and initscr() is called after initunix(), the name of this routine
 * must be tstp, overriding a routine of the same name in the curses
 * library which does not do what we want.
 */

tstp(sig)
	int sig;
{
	int (*prevttousig)() = signal(SIGTTOU, SIG_IGN);
		/* Ignore SIGTTOU so stty calls won't stop us again! */

	signal(sig, SIG_DFL);
	endterm();
	unfixttymodes();
	signal(SIGTTOU, prevttousig);
	kill(getpid(), SIGSTOP); /* Hard stop */
}


/*
 * A stop signal made us go to sleep in Tumbolia.
 * When we awake, we first get the SIGCONT signal.
 * The world may well have changed a little bit,
 * so do the tty initializations anew.
 */

cont()
{
	char cread = REDRAW;

	fixttymodes();
	initterm();

#ifdef TIOCSTI
	/* Simulate receipt of REDRAW initially so we come up
	   with a nice display. */
	ioctl(0, TIOCSTI, &cread);
#endif TIOCSTI
	signal(SIGCONT, cont);
	signal(SIGTSTP, tstp);
}
#endif

/*
 * Prepare for interrupts (UNIX `signals') to be caught so
 * we can reset the tty modes and perform miscellaneous other
 * exit routines.
 * Note -- if a signal arrives before the call to fixttymodes,
 * the unfixttymodes may render the terminal useless.  The fix is
 * easy, but I'm too lazy now (just read the statuses BEFORE,
 * but change them only AFTER signal setting).
 */

initunix()
{
	register int i;

	for (i = 1; i <= NSIG; ++i) {
		if (signal(i, SIG_IGN) != SIG_IGN)
			signal(i, catch);
	}
	/* Stop/continue must be handled differently, see stop() above. */
#ifdef SIGTSTP
	if (signal(SIGTSTP, SIG_IGN) != SIG_IGN)
		signal(SIGTSTP, tstp);
	/* We don't bother about SIGTTOU or SIGTTIN here; if they
	   ever arrive they are caught by the standard 'catch'. */
	signal(SIGCONT, cont);
#endif
	fixttymodes();
}


/*
 * The last termination routine to be called.
 * It also resets all signals to their default status.
 */

endunix()
{
	int i;

	fflush(stdout);
	unfixttymodes();
	for (i = 1; i <= NSIG; ++i)
		signal(i, SIG_DFL);
}


/*
 * Return a string like the one that perror(arg) would print
 * (see UNIX manual page perror(3) for details).
 * Like all C library routines returning strings, the string points
 * to static storage that is overwritten on each call.
 * If arg is fairly long, it may get truncated.
 */

string 
unixerror(arg)
	string arg;
{
	static char msg[80];
	extern int sys_nerr, errno;
	extern string sys_errlist[];

	if (errno > 0 && errno < sys_nerr)
		sprintf(msg, "%.30s: %.47s", arg, sys_errlist[errno]);
	else
		sprintf(msg, "%.50s: UNIX error %d", arg, errno);
	return msg;
}


/*
 * Hacks to fix certain peculiarities due to the hostile environment
 * in which the editor lives.
 */

Hidden struct sgttyb oldtty;

#ifdef TIOCSETC
Hidden struct tchars oldtchars;
#endif

#ifdef TIOCSLTC
Hidden struct ltchars oldltchars;
#endif

Hidden Procedure
fixttymodes()
{
	gtty(2, &oldtty);
	if (oldtty.sg_ospeed <= B600)
		slowterminal = Yes;
#ifdef XTABSCRAP
	/*
	 * Turn on XTABS mode, to be able to live when terminal tabs are
	 * set at 4 rather than 8 columns (the B interpreter used to set
	 * this).
	 * ***** IS THIS STILL NECESSARY? *****
	 */
	if (!(oldtty.sg_flags & XTABS)) {
		struct sgttyb newtty;
		gtty(2, &newtty);
		newtty.sg_flags |= XTABS;
		ioctl(2, TIOCSETN, &newtty);
	}
#endif XTABSCRAP

#ifdef TIOCSETC /* I.e., not at pre-version 7 UNIX systems */
	/*
	 * Set the quit character to ^\, turn off the interrupt
	 * character (so DELETE can be used as the editor's DELETE command).
	 * The start/stop characters are kept only if they are ^S/^Q.
	 */
	{
		struct tchars newtchars;
		ioctl(2, TIOCGETC, &oldtchars);
		ioctl(2, TIOCGETC, &newtchars);
		if ((newtchars.t_intrc & 0377) != 0377)
			newtchars.t_intrc = INTRCHAR;
		if ((newtchars.t_quitc & 0377) != 0377)
			newtchars.t_quitc = QUIT;
		if (newtchars.t_startc != Ctl(Q))
			newtchars.t_startc = -1;
		if (newtchars.t_stopc != Ctl(S))
			newtchars.t_stopc = -1;
		ioctl(2, TIOCSETC, &newtchars);
	}
#endif TIOCSETC

#ifdef TIOCSLTC /* I.e., at 4.xBSD systems */
	/*
	 * Turn off all local control characters except make ^Z delayed stop,
	 * if at least this was the original stop character.
	 */
	{
		static struct ltchars newltchars = {-1, -1, -1, -1, -1, -1};

		ioctl(2, TIOCGLTC, &oldltchars);
		if (oldltchars.t_suspc == Ctl(Z))
			newltchars.t_dsuspc = Ctl(Z);
		ioctl(2, TIOCSLTC, &newltchars);
	}
#endif
}


/*
 * Undo the effects of fixttymodes(), see comments there.
 */

Hidden Procedure
unfixttymodes()
{
	if (!oldtty.sg_ospeed)
		return; /* Not yet initialized! */
#ifdef XTABSCRAP
	ioctl(2, TIOCSETN, &oldtty);
#endif
#ifdef TIOCSETC
	ioctl(2, TIOCSETC, &oldtchars);
#endif
#ifdef TIOCSLTC
	ioctl(2, TIOCSLTC, &oldltchars);
#endif
}


/*
 * Return Yes if more input immediately available
 *
 * ***** UNIX DEPENDENCE *****
 * Assumes the standard UNIX definition of FILE: assumes there is
 * buffered input if stdin->_cnt > 0, so uses the `_cnt' field.
 *
 * ***** 4.X BSD DEPENDENCE *****
 * If the symbol FIONREAD is defined, uses this ioctl call to determine
 * whether more input is available; see tty(4) in 4.X BSD manual.
 */

Visible bool
moreinput()
{
	if (stdin->_cnt > 0)
		return Yes;

#ifdef FIONREAD
	{
		long n = 0;
		if (ioctl(0, FIONREAD, (struct sgttyb *)(&n)) == -1)
			return No;
		return n > 0;
	}
#else
	return No;
#endif
}
