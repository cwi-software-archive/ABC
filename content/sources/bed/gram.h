/* SccsId: @(#)gram.h	1.6 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Grammar symbols.
 */

/*
 * Values used in "tabl.c" but also publicly.
 */

#define Rootsymbol	00
#define Optional	98
#define Hole	99
#define Required	Hole


/*
 * Ditto for "lexi.c".
 */

#define LEXICAL 100

/*
 * Routines defined in "gram.c".
 */

string *noderepr();
node gram();
node suggestion();
node variable();
string symname();

/*
 * Macros for oft-used funtion.
 */

#define Fwidth(str) ((str) ? fwidth(str) : 0)

#define Fw_zero(str) (!(str) || index("\b\t", (str)[0]))
#define Fw_positive(str) ((str) && (str)[0] >= ' ')
#define Fw_negative(str) ((str) && (str)[0] == '\n')
