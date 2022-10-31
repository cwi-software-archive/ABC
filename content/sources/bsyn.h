/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* bsyn.h: syntax */

/* General parsing routines */

#define Eotc '\0'
#define Eouc '\036'
		/* The character Eotc (end of text char)		*/
		/* is placed after the last read character in txbuf.	*/
		/* In general it follows a newline.			*/
		/* If Eotc is encountered and more input is required,	*/
		/* getline() is called.					*/
		/* Eouc (end of unit char) is similar except the system	*/
		/* never has to read beyond it.				*/

#define Char(tx) (*(tx))
#define Eol(tx) (Char(tx) == '\n' || Char(tx) == Eouc)
#define Ceol(tx) (Char(tx) == '\\' || Eol(tx))
#define To_eol(tx) while (!Eol(tx)) tx++;
#define Mark_unit_end(tx) *tx= Eouc;

#define Space(c) ((c) == ' ' || (c) == '\t')
#define Skipsp(tx) while(Space(Char(tx))) tx++

#define Letter(c) ('a'<=c&&c<='z')
#define Cap(c) ('A'<=c&&c<='Z')
#define Dig(c) ('0'<=c&&c<='9')
#define Keymark(c) (Cap(c) || Dig(c) || c=='\'' || c=='"')
#define Tagmark(c) (Letter(c) || Dig(c) || c=='\'' || c=='"')
#define Keytagmark(c) (Keymark(c) || Letter(c))
#define Anytormark(c) (c=='+' || c=='-' || c=='*' || c=='/' || c=='#')
#define Montormark(c) (c=='~' || Anytormark(c))
#define Dyatormark(c) (Anytormark(c) || c=='^' || c=='<' || c=='>')

extern upto();
extern need();
extern nothing();
extern thought();
extern findceol();
extern bool ateol();

extern value findkw();
extern value keyword();
extern reqkw();
extern req();
extern value tag();

extern bool atkw();
extern bool find();
extern intlet count();

extern txptr fcol();
extern txptr lcol();
extern intlet alino;
extern txptr txstart, txend; /*TEMPORARY if possible*/

extern getline();
extern intlet ilev();
extern veli();
extern inistreams();
extern re_streams();
extern open_stream();
extern close_stream();
