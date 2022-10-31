/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* B numbers, basic external interface */

#include "b.h"
#include "bcon.h"
#include "bobj.h"
#include "bnum.h"

/*
 * This file contains operations on numbers that are not predefined
 * B functions (the latter are defined in `bfun.c').
 * This includes conversion of numeric B values to C `int's and
 * `double's, (numval() and intval()),
 * but also utilities for comparing numeric values and hashing a numeric
 * value to something usable as initialization for the random generator
 * without chance of overflow (so numval(v) is unusable).
 * It is also possible to build numbers of all types using mk_integer,
 * mk_exact (or mk_rat) and mk_approx.  Note the rather irregular
 * (historical!) argument structure for these: mk_approx has a
 * `double' argument, where mk_exact and mk_rat have two values
 * which must be `integer' (not `int's!) as arguments.
 * The random number generator used by the DRAW and CHOOSE statements
 * is also in this file.
 */

Hidden double ival();
Hidden Procedure breakup();

/*
 * numhash produces a `double' number that depends on the value of
 * v, useful for initializing the random generator.
 */

Visible double numhash(v) value v; {
	if (Integral(v)) {
		double d = 0;
		register int i;

		for (i = Size(v) - 1; i >= 0; --i) {
			d *= 2;
			d += Digit((integer)v, i);
		}

		return d;
	}

	if (Rational(v))
		return .777 * numhash(Numerator((rational)v)) +
		       .123 * numhash(Denominator((rational)v));

	return numval(v);
}


/*
 * ival is used internally by numval and intval.
 * It converts an integer to double precision, yielding
 * a huge value when overflow occurs (but giving no error).
 */

Hidden double ival(u) integer u; {
	double x = 0;
	register int i;

	for (i = Size(u)-1; i >= 0; --i) {
		if (x >= Maxreal/BASE)
			return Msd(u) < 0 ? -Maxreal : Maxreal;
		x = x*BASE + Digit(u, i);
	}

	return x;
}


/*
 * intval returns the C `int' value of a B numeric value, if it exists.
 * The number must be exact and have a denominator of 1.
 * All errors give error messages to the user.
 */

Visible int intval(v) value v; {
	double d;
	int l;

	/* v must be an Integral number or a Rational with Denominator==1
	    which may result from n round x [via mk_exact]!. */

	if (!Is_number(v) || !Integral(v) &&
		!(Rational(v) && Denominator((rational)v) == int_1)) {
		error("number not an integer");
		return 0;
	}

	d = ival((integer)v);
	if (d<-Maxintlet || d>Maxintlet) {
		error("exceedingly large integer");
		return 0;
	}

	l= d;
	return l;
}


/* convert an int to an intlet */

Visible intlet propintlet(i) int i; {
	if (i > Maxintlet || i < -Maxintlet) {
		error("exceedingly large integer");
		return 0;
	}
	return i;
}


/*
 * mk_exact makes an exact number out of two integers.
 * The third argument is the desired number of digits after the decimal
 * point when the number is printed (see comments in `bfun.c' for
 * `round' and code in `bnuC.c').
 * This printing size (if positive) is hidden in the otherwise nearly
 * unused * 'Size' field of the value struct
 * (cf. definition of Rational(v) etc.).
 */

Visible value mk_exact(p, q, len) integer p, q; int len; {
	rational r = mk_rat(p, q, len);

	if (Denominator(r) == int_1 && len <= 0) {
		integer i = (integer) Copy(Numerator(r));
		release((value) r);
		return (value) i;
	}

	return (value) r;
}


/*
 * mk_integer makes an integer B value out of an int.
 * This is sometimes useful for other utilities when an integer
 * constant is needed, see e.g. `numconst' in `bnuC.c'.
 */

Visible value mk_integer(x) int x; {
	return (value) mk_int((double)x);
}


/*
 * determine if a number is integer
 */

Visible bool integral(v) value v; {
	return Integral(v);
}

/*
 * mk_approx makes an approximate number out of a given `double'.
 * It checks for very large values (probably resulting from
 * overflowing mathematical functions).
 * Therefore the constant Maxreal must be somewhat smaller ???Guido
 * than the maximum representable floating point number.
 * (On the VAX, 1E38 does fine).
 */

Visible value mk_approx(x) double x; {
	real r = (real) grab_approx();

	if (x > Maxreal || x < -Maxreal) error("arithmetic overflow");

	Realval(r) = x;

	return (value) r;
}


Visible numcomp(u, v) value u, v; {
	real u1, v1;
	int s;

	if (Integral(u) && Integral(v))
		return int_comp((integer)u, (integer)v);

	if (Exact(u) && Exact(v)) {
		rational u1 = Integral(u) ? mk_rat((integer)u, int_1, 0) :
						(rational) Copy(u);
		rational v1 = Integral(v) ? mk_rat((integer)v, int_1, 0) :
						(rational) Copy(v);
		s = rat_comp(u1, v1);
		release((value) u1);
		release((value) v1);

		return s;
	}

	u1 = Approximate(u) ? (real) Copy(u) : (real) approximate(u);
	v1 = Approximate(v) ? (real) Copy(v) : (real) approximate(v);
	s = app_comp(u1, v1);
	release((value) u1);
	release((value) v1);

	if (s == 0) {	/* Make sure x and ~x are never equal */
		if (Approximate(u)) {
			if (!Approximate(v)) return 1;	/* ~v > v */
		} else	return -1;	/* u < ~u */
	}

	return s;
}


#if BASE == 100000000
#define MAXCONVSIZE 4
#endif
	/*
	 * This is intended to be a safeguard, integers with
	 * Size(u) <= MAXCONVSIZE can surely be converted to `double',
	 * larger ones may (but need not) cause overflow.
	 * Also, BASE**(MAXCONVSIZE-1) must have more precision than can
	 * be expressed in a `double'.
	 * It is clear, that MAXCONVSIZE is both very machine dependent
	 * and that it also depends on the value of BASE.???Guido
	 */


/*
 * Deliver 10**n, where n is a (maybe negative!) C integer.
 * The result is a value (integer or rational, actually).
 */

Visible value tento(n) int n; {
	if (n < 0) {
		integer i= int_tento(-n);
		value v= (value) mk_exact(int_1, i, 0);
		release(i);
		return v;
	}
	return (value) int_tento(n);
}


/*
 * Find a C double f and a C int e which satisfy f * 10**e = ~ u/v,
 * where the arguments u and v are B integers, u > 0, v > 0.
 */

Hidden Procedure breakup(u, v, pfrac, pexpo) integer u, v; double *pfrac; int *pexpo; {
	int expo; double frac, sign; value t, p, q; integer f;
	if (u == int_0) {
		*pfrac = 0, *pexpo = 0;
		return;
	}
	expo = scale(u, v);
	/* Compute floor (10**(20-expo) * u / v), which is about 10**20 */
	t = tento(20-expo);
	p = prod(u, t);
	q = quot(p, v);
	f = (integer) floorf(q);
	frac = numval((value)f);
	release(t), release(p), release(q), release(f);
	sign = frac;
	if (sign < 0) frac = -frac;
	expo -= 20;
	while (frac >= 10) frac /= 10, ++expo;
	*pfrac = (sign < 0 ? -frac : frac), *pexpo = expo;
}


/*
 * numval returns the numerical value of any numeric B value
 * as a C `double'.
 */

Visible double numval(u) value u; {
	if (!Is_number(u)) {
		error("value not a number");
		return 0;
	}

	if (Approximate(u)) return Realval((real)u);

	if (Integral(u)) {
		double x = ival((integer)u);
		if (x >= Maxreal || x < -Maxreal) {
			error("in ~x, x is too large");
			return 0;
		}
		return x;
	}

	if (Rational(u)) {
		double frac;
		int expo;
		breakup(Numerator((rational)u), Denominator((rational)u),
			&frac, &expo);
		while (expo > 0) {
			if (frac >= Maxreal/10) {
				error("in ~x, x is too large");
				return 0;
			}
			frac *= 10, --expo;
		}
		while (expo < 0) frac /= 10, ++expo; /* Assume underflow = 0 */
		return frac;
	}

	syserr("numval");
	/* NOTREACHED */
}


/*
 * Random numbers
 */


/* Initialize the random generator */

double lastran;

Visible Procedure setrandom (seed) double seed; {
	double x;

	x = seed >= 0 ? seed : -seed;
	while (x >= 1) x /= 10;
	lastran = floor(67108864.0*x);
}


/* Return a random number in [0, 1). */

Visible double random() {
	double p;

	p = 26353589.0 * lastran + 1;
	lastran = p - 67108864.0*floor(p/67108864.0);

	return lastran / 67108864.0;
}

Visible Procedure initnum() {
	rat_init();
}
