/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* bscr.h: screen */
extern bool interactive, read_interactive, outeractive;
extern bool Eof, Eof0;
extern FILE *ifile;
extern string iname;

extern jmp_buf reading[];
#define MAX_NMB_ACT_READS 10
extern intlet active_reads;

extern bool is_intended();
extern bool nwl;
extern redirect();
extern newline();
extern line();
extern wri_space();
extern writ();
extern wri();

extern FILE *sv_ifile;	/*TEMPORARY for syn*/
extern re_files();
extern vs_ifile();

extern initscr();
extern re_screen();
