/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* bmem.h: B memory management */

typedef char *ptr;
#define Nil ((ptr) 0)

extern ptr getmem();
extern regetmem();
extern freemem();
extern prgr();
extern xtndtex();
extern xtndlt();
extern initmem();
extern value notel; /*TEMPORARY*/
extern bool noting; /*TEMPORARY*/
