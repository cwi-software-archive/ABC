/*
 * Header file for use with FAKE curses package.
 */

#include <stdio.h>
#include <sgtty.h>

#define LINES	24
#define COLS	80

#define crmode() NULL
#define getch() getchar()
#define getyx(win, y, x) _getyx(&(y), &(x))
#define noecho() NULL
#define nonl() NULL
#define scanw scanf
