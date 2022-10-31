/* SccsId: @(#)b.h	1.5 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Basics copied from B interpreter's run-time system.
 */

#define NULL 0

#define Visible
#define Hidden static
#define Procedure

typedef int bool;
typedef short intlet;
typedef char *string;

#define No 0
#define Yes 1

#define Maxintlet ((1<<15)-1) /* MACHINE DEPENDENT */

typedef struct {
	char type;
	intlet len;
	intlet refcnt;
	string *cts;
} *value;

#define Refcnt(v) ((v)->refcnt)
#define Type(v) ((v)->type)
#define Length(v) ((v)->len)
#define Str(v) ((char*)(&(v)->cts))

#define Vnil ((value) NULL)

/* Types: */
#define Num '0'
#define Tex '"'
#define Com ','
#define Nod 'N'
#define Pat 'P'

/*
 * C library standard functions
 */

string malloc();
string realloc();

string sprintf();

string strcpy();
string strncpy();
string index();
string rindex();

string getenv();

#define Strequ(s, t) !strcmp(s, t)
#define Strnequ(s, t, n) !strncmp(s, t, n)
