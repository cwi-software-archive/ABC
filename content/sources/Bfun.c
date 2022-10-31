/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* Functions defined on numeric values. */

#include <math.h>
#include "b.h"
#include "bcon.h"
#include "bobj.h"
#include "bnum.h"

/*
 * Everything in this file is a function implementing a predefined
 * B arithmetic operator, taking one or two numeric values as operands,
 * and returning a numeric value.
 * It is guaranteed (by function `formula' in file `bfpr.c') that the
 * arguments are always numeric, so this isn't checked here.
 *
 * Since many predefined functions are defined in terms of others,
 * they may freely call each other.  Also they are sometimes called
 * from other modules related with numbers, e.g. the conversion routines.
 */


/* The next typedefs are only used so the declaration of `dyop' below
   does not disturb my cross-reference program `uses'.  GvR */

typedef value (*valfun)();
typedef rational (*ratfun)();
typedef real (*appfun)();

Hidden value dyop();
Hidden value xxx_quot();

/*
 * For the arithmetic functions (+, -, *, /) the same action is needed:
 * 1) if both operands are Integral, use function from int_* submodule;
 * 2) if both are Exact, use function from rat_* submodule (after possibly
 *    converting one of them from Integral to Rational);
 * 3) otherwise, make both approximate and use function from app_*
 *    submodule.
 * The functions performing the appropriate action for each of the submodules
 * are passed as parameters.
 * Note that division is a bit of an exception, as the result of i/j
 * is in general not an integer but a rational number; see `quot' below.
 */

Hidden value dyop(u, v, int_fun, rat_fun, app_fun)
	value u, v;
	valfun int_fun;
	ratfun rat_fun;
	appfun app_fun;
{
	if (Integral(u) && Integral(v))	/* Use integral operation */
		return (*int_fun)(u, v);

	if (Exact(u) && Exact(v)) {
		rational u1, v1, a;

		/* Use rational operation */

		u1 = Integral(u) ? mk_rat((integer)u, int_1, 0) :
				(rational) Copy(u);
		v1 = Integral(v) ? mk_rat((integer)v, int_1, 0) :
				(rational) Copy(v);
		a = (*rat_fun)(u1, v1);
		release((value) u1);
		release((value) v1);

		if (Denominator(a) == int_1 && Roundsize(a) == 0) {
			integer b = (integer) Copy(Numerator(a));
			release((value) a);
			return (value) b;
		}

		return (value) a;
	}

	/* Use approximate operation */

	{
		real u1, v1, a;
		u1 = Approximate(u) ? (real) Copy(u) : (real) approximate(u);
		v1 = Approximate(v) ? (real) Copy(v) : (real) approximate(v);
		a = (*app_fun)(u1, v1);
		release((value) u1);
		release((value) v1);

		return (value) a;
	}
}


/*
 * sum, diff, prod and quot call dyop above with the appropriate
 * function parameters.
 */

Visible value sum(u, v) value u, v; {
	return dyop(u, v, (value (*)())int_sum, rat_sum, app_sum);
}

Visible value diff(u, v) value u, v; {
	return dyop(u, v, (value (*)())int_diff, rat_diff, app_diff);
}

Visible value prod(u, v) value u, v; {
	return dyop(u, v, (value (*)())int_prod, rat_prod, app_prod);
}


/*
 * Here comes the exception for integer division: the result is not
 * an integer but a rational number.
 * Therefore we cannot use int_quot (which performs integer division with
 * truncation).  The routine `xxx_quot' below makes a rational number
 * out of two integers and simplifies it back to integer if the denominator
 * turns out to be 1.
 */

Hidden value xxx_quot(u, v) integer u, v; {

	if (v == int_0) {
		error("in i/j, j is zero");
		return (value) Copy(u);
	}

	return mk_exact(u, v, 0);
}

Visible value quot(u, v) value u, v; {
	return dyop(u, v, xxx_quot, rat_quot, app_quot);
}


/*
 * Unary minus follows the same general principle but only with
 * one operand, and thus can't use dyop.
 */

Visible value negated(u) value u; {
	if (Integral(u))
		return (value) int_neg((integer)u);
	if (Rational(u))
		return (value) rat_neg((rational)u);
	return (value) app_neg((real)u);
}


/*
 * The remaining operators follow less similar paths and some of
 * them contain quite subtle code.
 */

Visible value mod(u, v) value u, v; {
	value p, q, m, n;

	if (v == (value)int_0 ||
		Rational(v) && Numerator((rational)v) == int_0 ||
		Approximate(v) && Realval((real)v) == 0) {
		error("in x mod y, y is zero");
		return Copy(u);
	}

	if (Integral(u) && Integral(v)) {	/* Use facilities built in int_quot */
		release(int_quot((integer)u, (integer)v));
		return (value) int_qmod((integer)v);
	}

	/* Compute `u-v*floor(u/v)', as in the formal definition of `u mod v'. */

	q = quot(u, v);
	n = floorf(q);
	release(q);
	p = prod(n, v);
	release(n);
	m = diff(u, p);
	release(p);

	return m;
}


/*
 * u**v probably has the mose special cases, exceptions and ideosyncrasies
 * of all arithmetic functions.  Everything that can be assigned a meaning
 * without yielding af complex number is computed correctly (barring
 * arithmetic overflow).
 * The remaining cases give various error messages that are intended to
 * clear up the fog but you must know some mathemetics to understand
 * why `x**(p/q)' is in error if x is negative and q odd . . .
 */

Visible value power(u, v) value u, v; {
	if (Exact(u) && (Integral(v) ||
			/* Next check catches for integers disguised as rationals: */
			Rational(v) && Denominator((rational)v) == int_1)) {
		rational a;
		integer b = Integral(v) ? (integer)v : Numerator((rational)v);
			/* Now b is really an integer. */

		u = Integral(u) ? (value) mk_rat((integer)u, int_1, 0) :
				Copy(u);
			
		a = rat_power((rational)u, b);
		release(u);
		if (Denominator(a) == int_1) { /* Make integral result */
			b = (integer) Copy(Numerator(a));
			release((value) a);
			return (value)b;
		}
		return (value)a;
	}

	if (Exact(v)) {
		integer vn, vd;
		double x;
		double xu = numval(u);
		double xv;
		int s = (xu > 0) - (xu < 0);

		if (s < 0)
			xu = -xu;

		if (Integral(v)) {
			vn = (integer)v;
			vd = int_1;
		} else {
			vd = Denominator((rational)v);
			if (s < 0 && Even(Digit(vd, 0))) {
				error("in x**(p/q), x is negative and q is even");
				return Copy(u);
			}
			vn = Numerator((rational)v);
		}

		if (vn == int_0)
			return (value) Copy(int_1);

		if (s == 0 && Msd(vn) < 0) {
			error("0**y with negative y");
			return u;
		}

		if (s < 0 && Even(Digit(vn, 0)))
			s = 1;
		xv = numval(v);
		x = pow(xu, xv);
		if (x > Maxreal)
			error("arithmetic overflow in x**y");

		return mk_approx(x*s);
	}

	/* Everything else: we now know u or v is approximate */

	{
		double x;
		double xu = numval(u);
		double xv = numval(v);

		if (xu < 0) {
			error("in x**y, x is negative and y is not exact");
			return Copy(u);
		}

		if (xu == 0 && xv < 0) {
			error("0**y with negative y");
			return Copy(u);
		}

		if (xv == 0)
			x = 1;
		else {
			x = pow(xu, xv);
			if (x > Maxreal)
				error("arithmetic overflow in x**y");
		}

		return mk_approx(x);
	}
}


/*
 * floor: for approximate numbers floor(x) from the C mathematical
 * library is used, while for integers it is a no-op.
 */

Visible value floorf(u) value u; {
	integer a, b;

	if (Integral(u)) return Copy(u);
	if (Approximate(u)) return (value) mk_int(floor(Realval((real)u)));

	/* It is a rational number */

	a = int_quot(Numerator((rational)u), Denominator((rational)u));

	/* Inspect remainder of division in global variables: */

	if (int_qdiv > 0 || int_qrem == int_0)
		return (value) a;	/* No correction necessary */

	b = int_diff(a, int_1);
	release(a);

	return (value) b;
}


/*
 * ceiling u is defined as - floor(-u), and that is how it's implemented.
 */

Visible value ceilf(u) value u; {
	value v, w;

	v = negated(u);
	w = floorf(v);
	release(v);
	v = negated(w);
	release(w);

	return v;
}


/*
 * round u is defined as floor(u+0.5), which is what is done here,
 * except for integers which are left unchanged.
 */

Visible value round1(u) value u; {
	value v, w;

	if (Integral(u)) return Copy(u);

	v = sum(u, (value)rat_half);
	w = floorf(v);
	release(v);

	return w;
}


/*
 * u round v (defined as 10**-u * round(v*10**u)) is a very bad beast.
 * The calculations are not too complicated (although they
 * soon become quite inefficient when u is large), but the
 * implication (not too clear in the Draft Proposal for B)
 * is that u round v is always printed with exactly u digits
 * after the decimal point, even if this involves trailing zeros,
 * even if v is an integer.
 * At least the pilot implementation did this, and therefore the length
 * was carried around with all exact numbers.
 * I had to invent something else.  The (nasty) solution was to keep the
 * result as a rational, even if it could be simplified to an integer,
 * and store the length field is the size field of the rational number
 * (which is negative to distinguish it from integers, and < -1 to
 * distinguish it from approximate numbers).
 * So a size of -2 means a normal rational number, and a size < -2
 * means a rounded number to be printed with (-2 - length) digits
 * after the decimal point.  This last expression can be retrieved using
 * the macro Roundsize(v) which should only be applied to Rational
 * numbers.
 * Actually, most gory details are hidden in mk_exact, but the case is
 * even complicated because u can be negative, in which case 10**u
 * is not an integer.  Therefore mk_exact is only called under certain
 * circumstances, and first a check on the sensibility of u is made
 * (HUGE u values give error messages).
 */

Visible value round2(u, v) value u, v; {
	value w, tenp;
	int i;

	if (!Integral(u)) {
		error("in n round x, n is not an integer");
		u = (value) int_0;
	} else
		i = propintlet(intval(u));

	tenp = power((value)int_10, u);

	v = prod(v, tenp);
	w = round1(v);

	release(v);
	v = quot(w, tenp);
	release(w);
	release(tenp);

	if (i > 0) {	/* Set number of digits to be printed */
		if (propintlet(-2 - i) < -2) {
			if (Rational(v))
				Size(v) = -2 - i;
			else if (Integral(v)) {
				w = v;
				v = mk_exact(w, int_1, i);
				release(w);
			}
		}
	}

	return v;
}


/*
 * abs u is comparable in its logic to `-u'.
 */

Visible value absval(u) value u; {
	if (Integral(u)) {
		if (u != (value)int_0 && Msd((integer)u) < 0)
			return (value) int_neg((integer)u);
	} else if (Rational(u)) {
		if (Msd(Numerator((rational)u)) < 0)
			return (value) rat_neg((rational)u);
	} else if (Approximate(u) && Realval((real)u) < 0)
		return (value) app_neg((real)u);

	return Copy(u);
}


/*
 * sign u inspects the sign of either u or u's numerator.
 */

Visible value signum(u) value u; {
	int s;

	if (Exact(u)) {
		if (Rational(u))
			u = (value) Numerator((rational)u);
		s = u==(value)int_0 ? 0 : Msd((integer)u) < 0 ? -1 : 1;
	} else
		s = Realval((real)u) > 0 ? 1 : Realval((real)u) < 0 ? -1 : 0;

	return (value) mk_int((double)s);
}


/*
 * ~u makes an approximate number of any numerical value.
 * It optimizes for the case where u is already approximate.
 * It is also used by dyop, far above.
 * Note that very large exact numbers can cause an error message
 * about arithmetic overflow in the conversion routine numval.
 */

Visible value approximate(v) value v; {
	if (Approximate(v)) return Copy(v);

	return mk_approx(numval(v));
}


/*
 * numerator v returns the numerator of v, whenever v is an exact number.
 * For integers, that is v itself.
 */

Visible value numerator(v) value v; {
	if (!Exact(v)) {
		error("*/ on approximate number");
		return (value) Copy(int_0);
	}

	if (Integral(v)) return Copy(v);

	return (value) Copy(Numerator((rational)v));
}


/*
 * /*v returns the denominator of v, whenever v is an exact number.
 * For integers, that is 1.
 */

Visible value denominator(v) value v; {
	if (!Exact(v)) {
		error("/* on approximate number");
		return (value) Copy(int_0);
	}

	if (Integral(v)) return (value) Copy(int_1);

	return (value) Copy(Denominator((rational)v));
}


/*
 * u root v is defined as v**(1/u), where u is usually but need not be
 * an integer.
 * The case u=0 is caught here, otherwise the user would get a
 * confusing error message about x/0.
 */

Visible value root2(u, v) value u, v; {
	if (u == (value)int_0 ||
		Rational(u) && Numerator((rational)u) == int_0 ||
		Approximate(u) && Realval((real)u) == 0) {
		error("in x root y, x is zero");
		v = Copy(v);
	} else {
		u = quot((value)int_1, u);
		v = power(v, u);
		release(u);
	}

	return v;
}

Visible value root1(v) value v; {
	value two= mk_integer(2);
	v= root2(two, v);
	release(two);
	return(v);
}

Visible value pi() { return mk_approx(3.141592653589793238462); }
Visible value e() { return mk_approx(exp(1.0)); }

Visible value sin1(v) value v; { return mk_approx(sin(numval(v))); }
Visible value cos1(v) value v; { return mk_approx(cos(numval(v))); }
Visible value tan1(v) value v; { return mk_approx(tan(numval(v))); }
Visible value atn1(v) value v; { return mk_approx(atan(numval(v))); }
Visible value exp1(v) value v; { return mk_approx(exp(numval(v))); }
Visible value log1(v) value v; { return mk_approx(log(numval(v))); }

Visible value log2(u, v) value u, v;{
	return mk_approx(log(numval(v)) / log(numval(u)));
}

Visible value atn2(u, v) value u, v; {
	return mk_approx(atan2(numval(v), numval(u)));
}
