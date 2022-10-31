/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* B memory management */
#include "b.h"
#include "bobj.h"
#include "bmem.h"

#define Plausible(syze) {if ((int) (syze) < 0) \
	error("creating value of exceedingly large size");}

#define Note(p) /* note(p) */
#define deNote(p) /* denote(p) */

Hidden Procedure note();
Hidden Procedure denote();

Visible ptr getmem(syze) unsigned syze; {
	ptr p;
	Plausible(syze);
	p= (ptr) malloc(syze);
	Note(p);
	if (bugs || p==Nil) printf("{%u}",syze);
	if (p == Nil) memexh();
	return p;
}

Visible Procedure regetmem(v, syze) value *v; unsigned syze; {
	Plausible(syze);
	uniql(v);
	if (bugs) printf("[%u]",syze);
	deNote((ptr)*v);
	*v= (value) realloc((ptr) *v, syze);
	Note((ptr)*v);
	if ((ptr) *v == Nil) memexh();
}

Visible Procedure freemem(p) ptr p; {
	deNote(p);
	free(p);
}

value notel; bool noting=Yes;

Hidden Procedure note(p) int p; {
	if (!noting) {
		value ip;
		noting= Yes;
		insertl(ip= mk_integer(p), &notel);
		release(ip);
		noting= No;
	}
}

Hidden Procedure denote(p) int p; {
	if (!noting) {
		value ip;
		noting= Yes;
		if (!in(ip= mk_integer(p), notel))
			syserr("releasing illegally");
		removel(ip, &notel);
		release(ip);
		noting= No;
	}
}

