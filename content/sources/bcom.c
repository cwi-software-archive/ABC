/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* B compounds */
#include "b.h"
#include "bobj.h"

Visible Procedure put_in_field(v, c, i) value v, *c; intlet i; {
	/*Note that no copy of v is made: the caller must do this*/
	*(Ats(*c)+i)= v;
}

