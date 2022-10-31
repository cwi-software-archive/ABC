/* SccsId: @(#)main.c	1.11 12/2/83
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Main program (init/exit processing), error handling.
 */

/*
 * The B editor is a structured editor for a programming language
 * for beginners and non-professional computer users.
 * [L.G.L.T. Meertens: Draft Proposal for the B programming language,
 * Mathematical Centre, Amsterdam, 1982, ISBN 90 6169 238 2.]
 * Note that `B' is only a provisional name for the language.
 * The editor uses a subset of the run-time system for the B
 * interpreter, so that they may be linked together in a later stage.
 * Also the sharing strategy of the B run-time routines makes a very
 * elegant and powerful UNDO-mechanism possible.
 */

#include <stdio.h>

#include "b.h" /* Contains definitions like string, etc. */
#include "bobj.h"

#define DEFAULTFILE ".Bed_save"
#define SAVEPOSFILE ".Bed_pos"
#define MAXSAVE 50

/* Command line flags */
bool dflag; /* Set if debugging output wanted */
bool slowterminal;
	/* Set if the terminal is so slow that long messages are annoying */
bool hushbaby; /* Set if no bells are to be heard */

/*
 * Main program -- call module initializations, do some work, 
 * call module shut-off code, exit.
 */

Visible Procedure
main(argc, argv)
	int argc;
	string *argv;
{
	bool status;
	int lineno = 0;
	string arg0 = argv[0];
	string filename;

	/* Process UNIX command line options */
	for (; argc > 1 && argv[1][0] == '-'; --argc, ++argv) {
		switch (argv[1][1]) {

		case 'd':
			dflag = Yes;
			break;

		case 'h':
			hushbaby = Yes;
			break;

		case 's':
			slowterminal = Yes;
			break;

		default:
			fprintf(stderr, "Usage: %s [+lineno] [file [debug-output]]\n",
				arg0);
			exit(1);

		}
	}
	if (argc > 1 && argv[1][0] == '+') { /* +lineno option */
		lineno = atoi(argv[1] + 1);
		if (lineno < 0)
			lineno = 0;
		++argv, --argc;
	}
	filename = argc > 1 ? argv[1] : DEFAULTFILE;
	if (lineno <= 1) /***** Change to <= 0 when Steven fixed bnew! *****/
		lineno = getpos(filename);
	initall();
	status = demo(filename, lineno);
	endall();
	if (argc >= 3) {
		freopen(argv[2], "w", stderr);
		objstats();
	}
	if (status)
		objcheck();
	else
		objdump();
	return !status;
}




/*
 * Module initializations -- for each module xxxx that needs dynamic
 * initialization, call a routine named initxxxx.
 * The order is determined by the inter-module dependencies.
 * Also note that all terminal- and screen-related initializations are called
 * indirectly by initterm().
 */

Hidden Procedure
initall()
{
	initgram();
	initunix();
	initterm();
}


/*
 * Module shut-off code -- for each module xxxx that needs dynamic
 * shut-off code (what is the inverse of `initialization'?),
 * call a routine named endxxxx.
 * As enddemo() attempts to save the edit buffer, and endall() is
 * called whenever an interrupt occurs, the edited material is saved
 * under all circumstances.  Exception is SIGQUIT, which only resets
 * the terminal modes (see "unix.c" for details).
 */

Visible Procedure
endall()
{
	endterm();
	enddemo();
	endunix();
	objstats();
}



/*
 * System error -- abort the editor with a short error message.
 * Should only be called for catastrophic, unrecoverable errors
 * or those that `cannot happen'.
 */

/* VARARGS 1 */
Visible Procedure
syserr(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
	string fmt;
{
	endall();
	fprintf(stderr, "*** System error: ");
	fprintf(stderr, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	fprintf(stderr, "\n");
#ifndef NDEBUG
	fprintf(stderr, "*** Core dump for B guru: ");
	abort();
#endif
	exit(1);
	/* NOTREACHED */
}


/*
 * Assertion error.
 * Call syserr with information about where something was wrong.
 * (Sorry, WHAT was wrong must be dug out of the core dump.)
 */

Visible Procedure
asserr(file, line)
	string file;
	int line;
{
	syserr("Assertion failed: file %s, line %d", file, line);
}


/* ---------------------------------------------------------------- */

/*
 * Keep a simple database of file name vs. line number, so that
 * when an edit session is stopped and later continued, the focus
 * is at least at the same line as before.
 * The database is kept in most-recently-used-first order.
 * When it is rewritten, only its first MAXSAVE lines are saved,
 * thus limiting the amount of disk space wasted by files
 * that were once edited but then removed, renamed or forgotten.
 */


Visible int
getpos(file)
	register string file;
{
	register FILE *fp = fopen(SAVEPOSFILE, "r");
	char buf[BUFSIZ];
	auto int line = 0;
	register int len = strlen(file);

	if (!fp)
		return 0;
	while (fgets(buf, sizeof buf, fp) != NULL) {
		if (strncmp(buf, file, len) == 0
			&& (buf[len] == '\t' || buf[len] == ' ')) {
			if (sscanf(buf+len+1, "%d", &line) != 1)
				line = 0;
			break;
		}
	}
	fclose(fp);
	return line;
}


/*
 * Save position 'line' for file 'file'.
 * If 'line' is <= 1, the entry for 'file' is removed from the database.
 * Return Yes if save succeeded.
 */

Visible bool
savepos(file, line)
	register string file;
	int line;
{
	register int nsave = 0;
	register int i;
	register FILE *fp = fopen(SAVEPOSFILE, "r");
	char buf[BUFSIZ];
	register int len = strlen(file);
	value saved[MAXSAVE];

	if (fp) {
		while (fgets(buf, sizeof buf, fp) != NULL && nsave < MAXSAVE) {
			if (strncmp(file, buf, len) == 0
				&& (buf[len] == ' ' || buf[len] == '\t'))
				continue;
			saved[nsave] = mk_text(buf);
			++nsave;
		}
		fclose(fp);
		if (nsave == 0 && line <= 1) { /* Remove the database altogether */
			if (unlink(SAVEPOSFILE) != -1)
				return Yes;
		}
	}
	else if (line <= 1)
		return Yes;
	fp = fopen(SAVEPOSFILE, "w");
	if (line > 1)
		fprintf(fp, "%s\t%d\n", file, line);
	for (i = 0; i < nsave; ++i) {
		fputs(Str(saved[i]), fp);
		release(saved[i]);
	}
	return fclose(fp) != EOF;
}
