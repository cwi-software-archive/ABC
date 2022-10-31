/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* b.h: general */

#include <stdio.h>
#include <math.h>
#include <setjmp.h>

#define Forward extern
#define Visible
#define Hidden static
#define Procedure

/* The following are not intended as pseudo-encapsulation, */
/* but to emphasize intention. */
typedef char literal;
typedef char bool;
typedef char *txptr;
typedef char *string; /* Strings are always terminated with a char '\0'. */

#define Yes ((bool) 1)
#define No  ((bool) 0)
typedef short intlet;
extern bool bugs;

/************************************************************************/
/*                                                                      */
/* Values                                                               */
/*                                                                      */
/* A number is modelled as one of zero, unbounded integer,              */
/*        unbounded rational or approximate.                            */
/*     Zero has a 'length' field of zero, and nothing else.             */
/*     A length field of +n means the number is an n digit integer,     */
/*        (with digits to some large base).                             */
/*     A length of -1 means there follows a floating point number       */
/*     A length of -2 means there follow two values, pointers to two    */
/*        unbounded integers, ie a rational number.                     */
/*     A length of -n, n>2, means it is a rational with a print width   */
/*        of n-2.                                                       */
/*                                                                      */
/* A text is modelled as a sequence of len characters.                  */
/*                                                                      */
/* A compound is modelled as a sequence of len values, its fields.      */
/*                                                                      */
/* A list is modelled as a sequence of len values,                      */
/*         each of which corresponds to a list entry.                   */
/*                                                                      */
/* A table is modelled as a sequence of len values,                     */
/*         each of which corresponds to a table entry;                  */
/*     table entries are modelled as a compound with two fields.        */
/*                                                                      */
/************************************************************************/

typedef struct{char type; intlet refcnt, len; string *cts;} *value;

#define Dummy 0
#define Dumval ((value) Dummy)
#define Vnil ((value) 0)
#define Pnil ((value *) 0)

/* Types: */
#define Num '0'
#define Tex '"'
#define Com ','
#define Lis 'L'
#define Tab 'M'
#define ELT '}'
/* locations: */
#define Sim 'S'
#define Tri '@'
#define Tse '['
#define Glo 'g'
#define Per 'p'
/* units: */
#define How 'h'
#define For 'f'
#define Ref 'r'
#define Fun '+'
#define Prd 'i'

/* Environments and context */

typedef value envtab;
typedef struct ec{envtab tab; struct ec *inv_env;} envchain;
typedef envchain *env;

typedef struct{env curnv; value *bndtgs;
	literal cntxt, resexp; value uname;
	intlet cur_ilev, lino; txptr tx, ceol;} context;

#define Enil ((env) 0)

/* contexts: */
#define In_command 'c'
#define In_read '?'
#define In_unit 'u'
#define In_load 'l'
#define In_value 'v'
#define In_ref 'r'
#define In_formal 'f'
#define In_prmnv 'p'

/* results */
#define Ret 'V'
#define Rep '+'
#define Voi ' '

/* adicity */
#define Zer '0'
#define Mon '1'
#define Dya '2'

/* funprd.def */
#define Pre 'P'
#define Use 'U'

/************************************************************************/
/*                                                                      */
/* A function or predicate is modelled as a compound consisting of      */
/* (i)   two short integers for (L, H) priority                         */
/*           (relevant only for functions);                             */
/* (ii)  Zer/Mon/Dya for zero-, mon- or dyadicity;                      */
/* (iii) Pre/Use for pre- or user-definedness;                          */
/* (iv)  if Pre, a literal to switch on;                                */
/*       if Use, a pointer to the yield/test-unit text.                 */
/*                                                                      */
/************************************************************************/

typedef struct{envtab reftab; txptr fux, lux; bool filed;} how;
typedef struct{envtab reftab; txptr fux, lux; bool filed;
	intlet L, H; literal adic, def;} funprd;
/* The first four fields should have the same structure as those of 'hows' */

typedef struct{context con; txptr ftx;} formal;
typedef struct{txptr rp; intlet rlino;} ref;


/* Valorenv */
#define Val 'V'
#define Env 'O'

/************************************************************************/
/*                                                                      */
/* Locations                                                            */
/*                                                                      */
/* A simple location is modelled as a pair basic-identifier and         */
/*     environment, where a basic-identifier is modelled as a text      */
/*     and an environment as a pointer to a pair (T, E), where T is a   */
/*     table with basic-identifiers as keys and content values as       */
/*     associates, and E is the invoking environment or nil.            */
/*                                                                      */
/* A trimmed-text location is modelled as a triple (R, B, C).           */
/*                                                                      */
/* A compound location is modelled as a compound whose fields are       */
/*     locations, rather than values.                                   */
/*                                                                      */
/* A table-selection location is modelled as a pair (R, K).             */
/*                                                                      */
/************************************************************************/

typedef value loc;

typedef value basidf;
typedef struct{basidf i; env e;} simploc;
typedef struct{loc R; intlet B, C;} trimloc;
typedef struct{loc R; value K;} tbseloc;

/* Functions and Predicates */
typedef value fun;
typedef value prd;

extern char *malloc(), *realloc();
extern char *getenv();
