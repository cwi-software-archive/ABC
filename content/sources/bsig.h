/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/*interrupts and signals*/
extern inisigs(); /*initialise*/
extern ignsigs(); /*ignore signals from the user*/
extern re_sigs(); /*notice user signals again*/
extern accept_int(); /*stop ignoring interrupt signals*/
extern dump(); /*cause a core-dump*/
