/* Facilities supplied by the operating system */

#include "b.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "bobj.h"
#include "bxxx.h"

#define EDITOR (getenv("BEDITOR") != NULL ? getenv("BEDITOR") : DEDI)
#define COML 60
char com_line[COML];
#define At_eos(s) ((s)+= strlen(s))

f_edit(fname, errline) value fname; intlet errline; {
	string cl= com_line;
	strcpy (cl, EDITOR);
	if (errline > 0) sprintf(At_eos(cl), " +%d", errline);
	app_fname(At_eos(cl), fname);
	system(com_line);
}

f_rename(fname, nfname) value fname, nfname; {
	string cl;
	strcpy(cl= com_line, "mv");
	app_fname(At_eos(cl), fname);
	app_fname(At_eos(cl), nfname);
	system(com_line);
	/* what if mv fails??? */
}

f_delete(fname) value fname; {
	string cl;
	strcpy(cl= com_line, "rm");
	app_fname(At_eos(cl), fname);
	system(com_line);
}

#define SAFEFNLEN 13

value f_uname(name, utype) value name; literal utype; {
	intlet l= name->len > SAFEFNLEN-1 ? SAFEFNLEN-1 : name->len;
	value fn= grab_tex(l+1); string sfn= Str(fn);
	*sfn= utype;
	if (name->len < SAFEFNLEN) strcpy(sfn+1, Str(name));
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
	return fn;
}

app_fname(ceos, fname) string ceos; value fname; {
	string fp= Str(fname); intlet k;
	*ceos++= ' ';
	Over(fname) {
		*ceos++= '\\';
		*ceos++= *fp++;
	}
	*ceos= '\0';
}

unsigned f_size(ifile) FILE *ifile; {
	struct stat sb;
	if (fstat(fileno(ifile), &sb) != 0) syserr("can't stat file");
	return( (unsigned) (sb.st_size) );
}
