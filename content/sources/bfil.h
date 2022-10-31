/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* File accessing */

extern f_edit(); /* call the editor for a file */
extern f_rename(); /* rename a file */
extern f_delete(); /* delete a file */
extern value f_uname(); /* devise a filename for a unit */
extern value f_tname(); /* devise a filename for a target */
extern unsigned f_size(); /* size of a file */
extern bool f_exists(); /*enquire if a file exists*/
extern bool f_interactive(); /*enquire if a file is the keyboard/screen*/
extern lst_uhds(); /*list the headings of units*/

#define FHW '\''
#define FZR '<'
#define FMN '"'
#define FDY '>'
