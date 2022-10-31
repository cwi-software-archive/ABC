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

/* Rational arithmetic */

/* Length calculations used for fraction sizes: */
#define Maxlen(u, v) \
	(Roundsize(u) > Roundsize(v) ? Roundsize(u) : Roundsize(v))
#define Sumlen(u, v) (Roundsize(u)+Roundsize(v))
#define Difflen(u, v) (Roundsize(u)-Roundsize(v))

/* To shut off lint and other warnings: */
#undef Copy
#define Copy(x) ((integer)copy((value)(x)))

/* Make a normalized rational from two integers */

Visible rational mk_rat(x, y, len) integer x, y; int len; {
	rational a;
	integer u,v;

	if (y == int_0) syserr("mk_rat(x, y) with y=0");

	if (x == int_0 && len <= 0) return (rational) Copy(rat_zero);

	if (Msd(y) < 0) {	/* interchange signs */
		u = int_neg(x);
		v = int_neg(y);
	} else {
		u = Copy(x);
		v = Copy(y);
	}

	a = (rational) grab_rat();
	if (len > 0 && len+2 <= Maxintlet) Size(a) = -2 - len;

	if (u == int_0 || v == int_1) {
		/* No simplification possible */
		Numerator(a) = Copy(u);
		Denominator(a) = Copy(int_1);
	} else {
		integer g, abs_u;

		if (Msd(u) < 0) abs_u = int_neg(u);
		else abs_u = Copy(u);
		g = int_gcd(abs_u, v);
		release(abs_u);

		if (g != int_1) {
			Numerator(a) = int_quot(u, g);
			Denominator(a) = int_quot(v, g);
		} else {
			Numerator(a) = Copy(u);
			Denominator(a) = Copy(v);
		}
		release(g);
	}

	release(u); release(v);

	return a;
}


/* Arithmetic on rational numbers */

/* Shorthands: */
#define N(u) Numerator(u)
#define D(u) Denominator(u)

Visible rational rat_sum(u, v) register rational u, v; {
	integer t1, t2, t3, t4;
	rational a;

	a = mk_rat(t1=int_sum(t2=int_prod(N(u), D(v)),
			  t3=int_prod(N(v), D(u))), t4=int_prod(D(u), D(v)),
			  Maxlen(u, v));
	release(t1), release(t2), release(t3), release(t4);

	return a;
}


Visible rational rat_diff(u, v) register rational u, v; {
	integer t1, t2, t3, t4;
	rational a;

	a = mk_rat(t1=int_diff(t2=int_prod(N(u), D(v)),
			  t3=int_prod(N(v), D(u))), t4=int_prod(D(u), D(v)),
			  Maxlen(u, v));
	release(t1), release(t2), release(t3), release(t4);

	return a;
}


Visible rational rat_prod(u, v) register rational u, v; {
	integer t1, t2;
	rational a;

	a = mk_rat(t1=int_prod(N(u), N(v)), t2=int_prod(D(u), D(v)),
		Sumlen(u, v));
	release(t1), release(t2);

	return a;
}


Visible rational rat_quot(u, v) register rational u, v; {
	integer t1, t2;
	rational a;

	if (Numerator(v) == int_0) {
		error("in u/v, v is zero");
		return (rational) Copy(rat_zero);
	}

	a = mk_rat(t1=int_prod(N(u), D(v)), t2=int_prod(D(u), N(v)),
		Difflen(u, v));
	release(t1), release(t2);

	return a;
}


Visible rational rat_neg(u) register rational u; {
	register rational a;

	/* Avoid a real subtraction from zero */

	if (Numerator(u) == int_0) return (rational) Copy(u);

	a = (rational) grab_rat();
	N(a) = int_neg(N(u));
	D(a) = Copy(D(u));
	Size(a) = Size(u);

	return a;
}


/* Rational number to the integral power */

Visible rational rat_power(a, n) rational a; integer n; {
	integer u, v, tu, tv, temp;

	if (n == int_0) return mk_rat(int_1, int_1, 0);

	if (Size(n) > 0 && Msd(n) < 0) {
		if (Numerator(a) == int_0) {
			error("in 0**n, n is negative");
			return (rational) Copy(a);
		}
		if (Msd(Numerator(a)) < 0) {
			u= int_neg(Denominator(a));
			v = int_neg(Numerator(a));
		}
		else {
			u = Copy(Denominator(a));
			v = Copy(Numerator(a));
		}
		n = int_neg(n);
	} else {
		if (Numerator(a) == int_0) return (rational) Copy(a);
			/* To avoid necessary simplification later on */
		u = Copy(Numerator(a));
		v = Copy(Denominator(a));
		n = Copy(n);
	}

	tu = Copy(int_1);
	tv = Copy(int_1);

	while (n != int_0) {
		if (Odd(Digit(n,0))) {
			if (u != int_1) {
				temp = tu;
				tu = int_prod(u, tu);
				release(temp);
			}
			if (v != int_1) {
				temp = tv;
				tv = int_prod(v, tv);
				release(temp);
			}
		}

		/* Square u, v */

		if (u != int_1) {
			temp = u;
			u = int_prod(u, u);
			release(temp);
		}
		if (v != int_1) {
			temp = v;
			v = int_prod(v, v);
			release(temp);
		}

		n = int_half(n);
	} /* while (n!=0) */

	release(n);
	release(u);
	release(v);
	a = (rational) grab_rat();
	Numerator(a) = tu;
	Denominator(a) = tv;

	return a;
}


/* Compare two rational numbers */

Visible rat_comp(u, v) register rational u, v; {
	register rational d;
	int sd, su, sv;

	/* 1. Compare pointers */
	if (u == v || N(u) == N(v) && D(u) == D(v)) return 0;

	/* 2. Either zero? */
	if (N(u) == int_0) return int_comp(int_0, N(v));
	if (N(v) == int_0) return int_comp(N(u), int_0);

	/* 3. Compare signs */
	su = Msd(N(u));
	sv = Msd(N(v));
	su = (su>0) - (su<0);
	sv = (sv>0) - (sv<0);
	if (su != sv) return su > sv ? 1 : -1;

	/* 4. Subtract and return sign of difference */
	d = rat_diff(u, v);
	if (N(d) == int_0) {
		release((value) d);
		return 0;
	}
	sd = Msd(N(d));
	release((value) d);

	return sd < 0 ? -1 : 1;
}

Visible Procedure rat_init() {
	int_init();

	rat_zero = (rational) grab_rat();
	Numerator(rat_zero) = Copy(int_0);
	Denominator(rat_zero) = Copy(int_1);

	rat_half = (rational) grab_rat();
	Numerator(rat_half) = Copy(int_1);
	Denominator(rat_half) = Copy(int_2);
}
