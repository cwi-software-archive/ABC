/* SccsId: @(#)cell.h	1.4 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Definitions for linked lists of screen lines, baptized `cells'.
 * (This is NOT an abstract data type!)
 */

typedef struct cell cell;

struct cell {
	cell *c_link;
	node c_data;
	short c_onscreen;
	short c_oldindent;
	short c_newindent;
	short c_length;
	char c_oldvhole;
	char c_newvhole; /* Yes if this line contains a `vhole' */
	char c_oldfocus;
	char c_newfocus; /* Yes if this line contains underlining */
};

#define Cnil ((cell*) NULL)

#define Nowhere (-9999)

#define Space(p) \
	(((p)->c_length + (p)->c_newindent + (p)->c_newvhole + linelength - 1) \
		/ linelength)
#define Oldspace(p) \
	(((p)->c_length + (p)->c_oldindent + (p)->c_oldvhole + linelength - 1) \
		/ linelength)

cell *replist();

extern int linelength;
extern int winheigth;
