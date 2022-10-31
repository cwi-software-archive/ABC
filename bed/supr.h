/* SccsId: @(#)supr.h	1.4 11/8/83
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Superstructure for fine focusing.
 */

/*
 * Interpretation of mode and s1, s2, s3:
 * WHOLE: whole node is the focus;
 * SUBSET: s1/2, s2/2 are first and last child number under focus;
 *         even means fixed text, odd means child node;
 * SUBRANGE: s1/2 is fixed text number; s2, s3 are 1st&last char;
 *         if s1 is odd, ditto for child which must be "text";
 * VHOLE: s1/2 is fixed text number; volatile hole before char s2;
 *         if s1 is odd, ditto for child which must be "text".
 * ATEND: a volatile hole just after the entire node.
 * ATBEGIN: ditto just before it.
 * SUBLIST: s3 indicates how many times downrite() bring us
 *         beyond the focus (i.e., the focus is the subtree below
 *         ep->focus EXCLUDING the subtree reached after s3 times
 *         downrite().  Note s3 > 0.
 * FHOLE: Like VHOLE but in Fixed text.
 *
 * It is assumed that if the focus is a substring of fixed text
 * (SUBRANGE, VHOLE), it does not begin or end with lay-out of spaces.
 */

#define WHOLE	00
#define SUBSET	01
#define SUBRANGE	02
#define VHOLE	03
#define ATEND	04
#define ATBEGIN	05
#define SUBLIST	06
#define FHOLE	07

typedef struct {
	path focus;
	char /*0..SUBLIST*/ mode;
	char /*bool*/ copyflag;
	char /*bool*/ spflag;
	char /*bool*/ changed;
	short /*0..2*MAXCHILD+1*/ s1;
	short s2;
	short s3;
	short highest;
	value copybuffer; /* Actually, a queue */
	value oldmacro; /* A text */
	value newmacro; /* A text, too */
	int generation;
} environ;


#define Emove(e1, e2) ((e2) = (e1))
	 /***** Redefine this if no struct assignment available! *****/
#define Ecopy(e1, e2) \
	(Emove(e1, e2), pathcopy((e2).focus), copy((e2).copybuffer), \
	 copy((e2).oldmacro), copy((e2).newmacro))
#define Erelease(e) \
	(pathrelease((e).focus), release((e).copybuffer), \
	 release((e).oldmacro), release((e).newmacro))
