/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* berr.h: B error message handling */

extern intlet errlino; extern value erruname;
extern jmp_buf main_loop, load_error;
extern bool skipping;
extern bool tracing;

extern syserr();
extern memexh();
extern error();
extern parerr();
extern pprerr();
extern checkerr();
extern debug();
extern trace();
extern int_signal();
extern bye();
