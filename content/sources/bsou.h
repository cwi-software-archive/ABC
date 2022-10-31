/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* bsou.h: sources */

extern value aster;

extern bool unit(); /* get a new unit */
extern bool is_unit(); /*enquire if a keyword is a unit */
extern def_unit(); /*enter a keyword into the list of units*/
extern value unit_info(); /*retrieve the details of a unit*/
extern special(); /*execute a special command*/
extern vs_ifile(); /*restore the input file*/

extern bool is_tloaded();
extern getprmnv();
extern putprmnv();

extern initsou();
