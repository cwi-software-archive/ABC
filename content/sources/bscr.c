/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* B screen handling */
#include "b.h"
#include "bobj.h"
#include "bscr.h"
#include "bfil.h"

bool interactive, read_interactive, outeractive;
bool nwl= Yes; /*Yes if currently at the start of an output line*/
bool Eof, Eof0;
Hidden FILE *ofile= stdout; Hidden bool woa, wnwl;
FILE *ifile;	 /* input file */
FILE *sv_ifile;	 /* copy of ifile for restoring after switching to read unit */
string iname;	 /* input name */

jmp_buf reading[MAX_NMB_ACT_READS];
intlet active_reads; 


#define USE_YES_OR_NO \
"\r*** Answer with yes or no (or use interrupt to duck the question)\n"

#define LAST_CHANCE \
"*** This is your last chance. Take it. I really don't know what you want if\n\
    %s\n\
    So answer the question\n"

#define NO_THEN \
"*** Well, I shall assume that your refusal to answer the question means no!\n"

Hidden Procedure putch();

Visible bool is_intended(m) string m; {
	char answer; intlet try, k;
	if (!interactive) return Yes;
	if (outeractive) line();
	fprintf(stderr, "*** %s\n", m);
	for (try= 1; try<=4; try++){
		answer= '?';
		if (outeractive) nwl= No;
		fprintf(stderr, "?\b");
		while ((k= getchar()) != EOF && k != '\n') {
			if ((k == ' ' || k == '\t'));
			else if (answer == '?') answer= k;
		}
		if (k == EOF && try < 4) {
			fprintf(stderr, USE_YES_OR_NO);
			if (outeractive) nwl= Yes;
		} else {
			if (outeractive && k == '\n') nwl= Yes;
			if (answer == 'y' || answer == 'Y') return Yes;
			if (answer == 'n' || answer == 'N') return No;
			if (outeractive) line();
			fprintf(stderr,
				try == 1 ? "*** Please answer with yes or no\n" :
				try == 2 ? "*** Just yes or no, please\n" :
				try == 3 ? LAST_CHANCE :
				NO_THEN, m);
		}
	} /* end for */
	return No;
}

Visible Procedure redirect(of) FILE *of; {
	ofile= of;
	if (of == stdout) {
		outeractive= woa;
		nwl= wnwl;
	} else {
		woa= outeractive; outeractive= No;
		wnwl= nwl; nwl= Yes;
	}
}

Hidden Procedure putch(c) char c; {
	putc(c, ofile);
	if (c == '\n') nwl= Yes;
	else nwl= No;
}

Visible Procedure newline() {
	putch('\n');
	fflush(stdout);
}

Visible Procedure line() {
	if (!nwl) newline();
}

Visible Procedure wri_space() { /* Experiment: no space before outer strings */
	if (!nwl) putch(' ');
}

Visible Procedure writ(v) value v; {
	wri(v, Yes, Yes, No);
	fflush(stdout);
}

#define Putch_sp() {if (!perm) putch(' ');}

Visible Procedure wri(v, coll, outer, perm) value v; bool coll, outer, perm; {
	intlet len= Length(v), k;
	if (outer && !Is_text(v)) wri_space();
	if (Is_number(v)) {
		if (perm) printnum(ofile, v);
		else {
			string cp= convnum(v);
			while(*cp) putch(*cp++);
		}
	} else if (Is_text(v)) {
		string tp= Str(v); char c;
		if (!outer) putch('\'');
		Overall {
			putch(c= *tp++);
			if (!outer && (c == '\'' || c == '`'))
				putch(c);
		}
		if (!outer) putch('\'');
	} else if (Is_compound(v)) {
		outer&= coll;
		if (!coll) putch('(');
		Overall {
			wri(Field(v, k), No, outer, perm);
			if (!Last()) {
				if (!outer){
					putch(',');
					Putch_sp();
				}
			}
		}
		if (!coll) putch(')');
	} else if (Is_list(v) || Is_ELT(v)) {
		value ve;
		putch('{');
		Overall {
			wri(ve= thof(k+1, v), No, No, perm);
			release(ve);
			if (!Last()) {
				putch(';');
				Putch_sp();
			}
		}
		putch('}');
	} else if (Is_table(v)) {
		putch('{');
		Overall {
			putch('['); wri(Key(v, k), Yes, No, perm);
			putch(']'); putch(':'); Putch_sp();
			wri(Assoc(v, k), No, No, perm);
			if (!Last()) {
				putch(';');
				Putch_sp();
			}
		}
		putch('}');
	} else {
		if (bugs) { putch('?'); putch(Type(v)); putch('?'); }
		else syserr("writing value of unknown type");
	}
}

Visible Procedure vs_ifile() {
	ifile= sv_ifile;
}

Visible Procedure re_files() {
	if (interactive && sv_ifile != ifile) {
		if (ifile != stdin) fclose(ifile);
		vs_ifile();
		Eof= Eof0= No;
	}
}

Visible Procedure initscr() {
	read_interactive= f_interactive(stdin);
	outeractive= f_interactive(stdout);
}

Visible Procedure re_screen() {
	sv_ifile= ifile;
	interactive= f_interactive(ifile);
}
