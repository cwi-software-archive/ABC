/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* Facilities supplied by the file system */

#include "b.h"
#include "bcon.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "bobj.h"
#include "bfil.h"

/*This file defines the facilities needed for dealing with files,
  apart from C's standard I/O facilities which are used throughout the system.

  Units are held on files in a 'workspace', which on Unix is modelled
  using directories. The function 'f_uname' converts a unit name into a
  unique filename. On Unix this is done by prepending a character to the unit
  name to indicate the kind of unit (for how'to ', and for tests and yields
  < for zeroadic, " for monadic and > for dyadic; these have been chosen as
  characters that are not usually used in filenames), and truncating the
  name if necessary. If the name does have to be truncated, then it is
  hashed to produce a character that is appended to the filename, in an attempt
  to produce a unique filename. Even so, it is still possible for different
  unit names to produce the same filename, and in the unlikely event of this
  happening you get an error message that the unit already exists when you
  try to create the clashing unit name.

  Filenames are at most SAFEFNLEN characters long, which on standard Unix
  systems gives you one spare character for making backups or whatever.

  It would be better if the B system effectively maintained its own directories
  that mapped units onto files in the real file system, as is done for targets.
  With operating systems with a more limited file system (eg even shorter
  filenames) this is the only possibility.

*/

#define COML 60
Hidden char com_line[COML];
#define At_eos(s) ((s)+= strlen(s))	/* Is this useful ???? */

Hidden value try();
Hidden Procedure app_fname();

Visible Procedure f_edit(fname, errline) value fname; intlet errline; {
	/*The default editor is called with a first parameter of the line number
	  and a second parameter of the file name*/
	string cl= com_line;
	if (getenv("BEDITOR") == NULL) strcpy (cl, DEDI);
	else strcpy(cl, getenv("BEDITOR"));
	if (*(cl+strlen(cl)-1) == '+') {
		sprintf(At_eos(cl), "%d", errline == 0 ? 1 : errline);
	}
	app_fname(At_eos(cl), fname);
	system(com_line);
}

Visible Procedure f_rename(fname, nfname) value fname, nfname; {
	string cl;
	strcpy(cl= com_line, "mv");
	app_fname(At_eos(cl), fname);
	app_fname(At_eos(cl), nfname);
	system(com_line);
	/* what if mv fails??? */
}

Visible Procedure f_delete(fname) value fname; {
	string cl;
	strcpy(cl= com_line, "rm");
	app_fname(At_eos(cl), fname);
	system(com_line);
}

Visible bool f_exists(fname) value fname; {
	FILE *f= fopen(Str(fname), "r");
	if (f==NULL) return No;
	fclose(f);
	return Yes;
}

#define SAFEFNLEN 13

Visible value f_uname(name, utype) value name; literal utype; {
	static char fname[SAFEFNLEN+1];
	string sfn= fname;
	*sfn= utype;
	if (Length(name) < SAFEFNLEN) strcpy(sfn+1, Str(name));
	else {
		double hh= hash(name)*321.987; char h;
		strncpy(sfn+1, Str(name), SAFEFNLEN-2);
		hh= hh-floor(hh); h= (char) floor(hh*29) + '!';
		if (h >= '"') h++;
		if (h >= '\'') h++;
		if (h >= '/') h++;
		if (h >= '0') h+= 10;
		if (h >= 'A') h+= 26;
		if (h >= 'a') h+= 26;
		*(sfn+SAFEFNLEN-1)= h; *(sfn+SAFEFNLEN)= '\0';
	}
	return mk_text(fname);
}

Hidden value try(t, n) value t; intlet n; {
	value eq= mk_text("="), fn, a, b, c;
	if (n == 0) fn= concat(eq, t);
	else { /* PUT "="^(n<<1)^t IN fn */
		fn= concat(eq, a= concat(b= convert(c= mk_integer(n), No, No), t));
		release(a); release(b); release(c);
	}
	release(eq);
	if (Length(fn) > SAFEFNLEN) {
		fn= trim(a= fn, 0, Length(fn)-SAFEFNLEN);
		release(a);
	}
	return(fn);
}

Visible value f_tname(t) value t; {
	value fn= try(t, 0);
	intlet i= 0;
	while (f_exists(fn)) {
		release(fn);
		fn= try(t, ++i);
	}
	return(fn);
}

Hidden Procedure app_fname(ceos, fname) string ceos; value fname; {
	string fp= Str(fname); intlet k;
	*ceos++= ' ';
	Over(fname) {
		*ceos++= '\\';
		*ceos++= *fp++;
	}
	*ceos= '\0';
}

Visible unsigned f_size(ifile) FILE *ifile; {
	struct stat sb;
	if (fstat(fileno(ifile), &sb) != 0) syserr("can't stat file");
	return( (unsigned) (sb.st_size) );
}

Visible bool f_interactive(ifile) FILE *ifile; {
	return isatty(fileno(ifile));
}

Visible Procedure lst_uhds() {
	/*List the headings of the units in this workspace*/
	system("for i in [\\'\\\"\\>\\<]*; do head -1 $i 2>/dev/null; done");
	/*just for now, you understand*/
}
