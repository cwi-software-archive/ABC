/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
#include "b.h"
#include "bcon.h"
#include "bobj.h"
#include "bnum.h"


/* Sum of two approximate numbers */

Visible real app_sum(u, v) real u, v; {
	double xu = Realval(u), xv = Realval(v), x;

	if (fabs(xu/2 + xv/2) > Maxreal/2) {
		error("in x+y, the result is too large");
		x = 0;
	} else
		x = xu + xv;

	return (real) mk_approx(x);
}


/* Difference of two approximate numbers */

Visible real app_diff(u, v) real u, v; {
	double xu = Realval(u), xv = Realval(v), x;

	if (fabs(xu/2 - xv/2) > Maxreal/2) {
		error("in x-y, the result is too large");
		x = 0;
	} else
		x = xu - xv;

	return (real) mk_approx(x);
}


/* Product of two approximate numbers */

Visible real app_prod(u, v) real u, v; {
	double xu = Realval(u), xv = Realval(v), x;
	int eu = 0, ev = 0;

	if (xu != 0) frexp(xu, &eu);
	if (xv != 0) frexp(xv, &ev);
	if (eu + ev > Maxexpo) {
		error("in x*y, the result is too large");
		x = 0;
	} else	x = xu * xv;

	return (real) mk_approx(x);
}


/* Quotient of two approximate numbers */

Visible real app_quot(u, v) real u, v; {
	double xu = Realval(u), xv = Realval(v), x = 0;
	int eu = 0, ev = 0;

	if (xv == 0) error("in x/y, y is zero");
	else {
		if (xu != 0) VOID frexp(xu, &eu);
		VOID frexp(xv, &ev);
		if (eu - ev > Maxexpo)
			error("in x/y, the result is too large");
		else
			x = xu / xv;
	}

	return (real) mk_approx(x);
}


/* Negative of approximate number */

Visible real app_neg(u) real u; {
	return (real) mk_approx(-Realval(u));
}


/* Compare two approximate numbers */

Visible app_comp(u, v) real u, v; {
	double xu = Realval(u), xv = Realval(v);

	return (xu > xv) - (xu < xv); /*tricky!*/
}
