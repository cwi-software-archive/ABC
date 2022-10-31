/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/*Configuration file: some easy changes to the system*/

/*if your compiler accepts (void) you can add " (void)" to the next line
  to reduce lint messages*/
#define VOID

/* Default editor: end the string with <space>+ if it accepts a line number */
/* of the form +<number> before the file name when it is called		    */
/* eg vi +10 myunit 							    */

#define DEDI "/usr/ucb/vi +"

/* 'Small' number package. */
#define BIG 72057594037927935.0 /*Largest integral real number*/
#define LONG 9999999999999999.5 /*Largest power of 10 less than BIG*/
#define MAXNUMDIG 16		/*The number of 9's in LONG*/
#define MINNUMDIG  6		/*Don't change*/

/* Unbounded number package */
#define Maxreal 1.7E38
#define Maxexpo 127
#define Minexpo (-127)

/*Other definitions*/
#define Maxintlet ((1<<15)-1)	/*Largest short*/

#define RDBUFSIZE 2000		/*Buffer used for read commands*/
#define TXDBUFSIZE 100		/*Text displays*/

#define SEED getpid()		/*Any suitable random int (eg date or time) */
				/*to start the random number generator with */
