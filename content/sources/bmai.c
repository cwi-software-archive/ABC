/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* B driver for interpreter */
#include "b.h"
#include <sys/types.h>

#include "bobj.h"
#include "bmem.h"
#include "bscr.h"
#include "berr.h"
#include "bfil.h"
#include "bsig.h"
#include "bsyn.h"
#include "bsem.h"
#include "bsou.h"

Hidden bool call_error, in_process;

Hidden Procedure erm();
Hidden Procedure call();
Hidden Procedure show_call();
Hidden Procedure init();

main(argc, argv) int argc; string argv[]; {
	in_process= No; call_error= No;
	call(argc, argv);
	if (call_error) exit(-1);
	in_process= Yes;
	init();
	call(argc, argv);
	bye(0);
}

#define Cllerr stderr

Hidden string pname;	 /* program name */

Hidden Procedure erm(m, n, argc, pargc, pargv) string m, n; int argc, pargc; string pargv[]; {
	fprintf(Cllerr,
 "*** There is something I don't quite get in your call of %s\n", pname);
	show_call(argc, pargc, pargv);
	fprintf(Cllerr, "*** The problem is: %s %s\n", m, n);
	if (in_process) bye(-1);
	call_error= Yes;
}

Hidden Procedure call(pargc, pargv) int pargc; string pargv[]; {
	int argc; string *argv;

	pname = pargv[0];
	argc = pargc-1;
	argv = pargv+1;
	while (argc >= 0)
	if (argc > 0 && argv[0][0] == '-' && argv[0][1] != '\0') {
		if (argv[0][1] == 'q') { if (in_process) bye(0); }
		else erm("I never learned about the option", argv[0], argc, pargc, pargv);
		argc -= 1;
		argv += 1;
	} else {
		if (argc == 0 || (argv[0][0] == '-' && argv[0][1] == '\0')) {
			iname = "standard input";
			ifile = stdin;
		} else {
			iname = *argv;
			ifile = fopen(iname, "r");
		}
		if (ifile != NULL) { if (in_process) process();
		} else erm("can't open input file", iname, argc, pargc, pargv);
		if (ifile != NULL && ifile != stdin) fclose(ifile);
		++argv; --argc;
	}
}

Hidden Procedure show_call(eargc, pargc, pargv)
 int eargc, pargc; string pargv[]; {
	int argc= pargc; string *argv= pargv;
	intlet p, pos= 4;
	fprintf(Cllerr, "    ");
	while (argc > 0) {
		fprintf(Cllerr, *argv);
		pos+= strlen(*argv);
		if (argc == eargc) p= pos-1;
		++argv; --argc;
		if (argc > 0) {
			putc(' ', Cllerr);
			pos++;
		}
	}
	putc('\n', Cllerr);
	for (pos= 0; pos < p; pos++) putc(' ', Cllerr);
	fprintf(Cllerr, "^\n");
}

Hidden Procedure init() {
	initmem();
	initnum();
	inittex();
	initenv();
	inittors();
	initsou();
	initfprs();
	initcom();
	inithow();
	initscr();
	setprmnv();
	getprmnv();
	inisigs();
}

Visible Procedure process() {
	re_screen();
	alino= 0; Eof= Eof0= No;
	xeq= Yes; skipping= No; cntxt= In_command;
	if (setjmp(main_loop) != 0) /* long jump occurred */ {
		re_files();
		if (Eof) return;
		xeq= interactive; skipping= !interactive;
		/* skipping may be reset by command() */
	}
	inistreams();
	re_env();
	active_reads= 0;
	while (!Eof) {
		re_streams();
		cntxt= In_command; resexp= Voi; lino= 0;
		cur_ilev= 0;
		terminated= No;
		if (ilev(Yes) > 0) parerr("outer indentation not zero", "");
		findceol(); command();
		re_files();
	}
}
