/* SccsId: @(#)bobj.h	1.5 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Interface to "B machine".
 */

/*
 * General values.
 */

value grab_com();
value copy();

/*
 * Operations on texts.
 */

value mk_text();
value trim();
value concat();
