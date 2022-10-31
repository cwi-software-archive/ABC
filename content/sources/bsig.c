/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/*Handle interrupts and signals*/
#include "b.h"
#include "bobj.h"
#include <signal.h>
#include "berr.h"

/*Test whether a signal represents an arithmetic error.
  This test is useful since machines differ considerably in the number
  and nature of their arithmetic signals.
*/
#define	SIG_ARERR(s)	(s == SIGFPE)

/*The operating system provides a function signal(s,f)
  that associates function f with the signal s, and returns
  a pointer to the previous function associated with s.
  Then, when signal s occurs, f is called and the function associated with s
  is reset. Thus f needs to call signal(s,f) again to reset it.

  There are two signals that can come from the user: quit and interrupt.
  Interrupt should just stop the interpreter and return to B command level;
  quit should stop the B system completely and produce a dump.
  All other signals are caused by errors (eg memory exhausted)
  or come from outside the program, and are therefore fatal.

  SIG_IGN is the system supplied routine to ignore a signal.
  SIG_DFL is the system supplied default for a signal.
  kill(getpid(), signal) kills the program according to 'signal'
*/

Hidden Procedure oops();
Hidden Procedure burp();
Hidden Procedure aog();
Hidden Procedure fpe_signal();
Hidden Procedure intsig();
Hidden Procedure signote();
Hidden int(* setsig())();

Visible Procedure dump() {
	signal(SIGQUIT, SIG_DFL);
	kill(getpid(), SIGQUIT);
}

Hidden Procedure oops(sig, m) int sig; string m; {
	fflush(stdout);
	fprintf(stdout, "*** Oops, %s\n", m);
	fflush(stdout);
	if (cntxt != In_prmnv) putprmnv();
	kill(getpid(), sig);
}

Hidden Procedure burp(sig) int sig; {
	oops(sig,
 "I feel suddenly (BURP!) indisposed. I'll call it a day. Sorry.");
}

Hidden Procedure aog(sig) int sig; {
	oops(sig,
 "an act of God has occurred compelling me to discontinue service.");
}

Hidden Procedure fpe_signal(sig) int sig; {
	signal(sig, fpe_signal);
	error("arithmetic error");
}

Hidden Procedure intsig(sig) int sig; { /*sig==SIGINT*/
	signal(sig, SIG_IGN);
	int_signal(No);
}

Visible Procedure accept_int() {
	signal(SIGINT, intsig);
}

Hidden int (*si)(), (*sq)();
Hidden bool sawi= No, sawq= No;

Hidden Procedure signote(sig) int sig; {
	/*Note but otherwise ignore a quit or interrupt*/
	signal(sig, signote);
	fprintf(stderr, "*** Just a moment\n");
	if (sig == SIGINT) sawi= Yes;
	else if (sig == SIGQUIT) sawq= Yes;
}

Hidden int(* setsig(sig, func))() int sig, (*func)(); {
	/*Set a signal, unless it's being ignored*/
	int (*f)()= signal(sig, SIG_IGN);
	if (f != SIG_IGN) signal(sig, func);
	return f;
}

Visible Procedure ignsigs() {
	/*Henceforth only note quits and interrupts*/
	si= setsig(SIGINT, signote);
	sq= setsig(SIGQUIT, signote);
}

Visible Procedure re_sigs() {
	/*Start processing quits and interrupts again*/
	signal(SIGINT, si);
	signal(SIGQUIT, sq);
	if (sawi) {
		sawi= sawq= No;
		if (si != SIG_IGN && si != SIG_DFL) (*si)(SIGINT);
	} else if (sawq) {
		sawq= No;
		if (sq != SIG_IGN && sq != SIG_DFL) (*sq)(SIGQUIT);
	}
}

Visible Procedure inisigs() {
	int s;

	for (s = 1; s < NSIG; s++)	{
		if (s == SIGINT)
			setsig(s, intsig);
		else
		if (s == SIGQUIT || s == SIGPIPE)
			setsig(s, aog);
		else
		if (SIG_ARERR(s))
			setsig(s, fpe_signal);
		else
			setsig(s, burp);
	}
}
