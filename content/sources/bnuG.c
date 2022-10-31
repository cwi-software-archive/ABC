/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
#include "b.h"
#include "bobj.h"
#include "bnum.h"

Hidden digit dig_gcd();
Hidden integer twice();

/*
 * Routines for greatest common divisor calculation
 * Cf. Knuth II Sec. 4.5.2. Algorithm B
 * "Binary gcd algorithm"
 * The labels correspond with those in the book
 *
 * Assumptions about built-in arithmetic:
 * x>>1 == x/2  (if x >= 0)
 * 1<<k == 2**k (if it fits in a word)
 */

/* Single-precision gcd for integers > 0 */

Hidden digit dig_gcd(u, v) register digit u, v; {
	register digit t;
	register int k = 0;

	if (u <= 0 || v <= 0) syserr("dig_gcd of number(s) <= 0");

 /*B1*/	while (Even(u) && Even(v)) ++k, u >>= 1, v >>= 1;

 /*B2*/	if (Even(u)) t = u;
	else {	t = -v;
		goto B4;
	}

	do {
 /*B3*/		do {
			t /= 2;
 B4:			;
		} while (Even(t));

 /*B5*/		if (t > 0) u = t;
		else v = -t;

 /*B6*/		t = u-v;
	} while (t);

	return u * (1<<k);
}


Visible integer int_half(v) integer v; {
	register int i;
	register long carry;

	if (v == int_1) {
		release((value) v);
		return (integer) Copy(int_0);
	}

	if (Msd(v) < 0) {
		i = Size(v)-2;
		if (i < 0) {
			release((value) v);
			return (integer) Copy(int_0);
		}
		carry = BASE;
	} else {
		carry = 0;
		i = Size(v)-1;
	}

	if (Refcnt(v) > 1) uniql(&v);

	for (; i >= 0; --i) {
		carry += Digit(v,i);
		Digit(v,i) = carry/2;
		carry = carry&1 ? BASE : 0;
	}

	return int_canon(v);
}


Hidden integer twice(v) integer v; {
	digit carry = 0;
	int i;

	if (Refcnt(v) > 1) uniql(&v);

	for (i = 0; i < Size(v); ++i) {
		carry += Digit(v,i) * 2;
		if (carry >= BASE)
			Digit(v,i) = carry-BASE, carry = 1;
		else
			Digit(v,i) = carry, carry = 0;
	}

	if (carry) {	/* Enlarge the number */
		v = (integer) regrab_num(v, Size(v)+1);
		Msd(v) = carry;
	}

	return v;
}


/* Multi-precision gcd of integers > 0 */

Visible integer int_gcd(u1, v1) integer u1, v1; {

	if (u1==int_0 || v1==int_0 || Msd(u1)<0 || Msd(v1)<0)
		syserr("gcd of number(s) <= 0");

	if (Size(u1) == 1 && Size(v1) == 1) {
		digit g = dig_gcd(Digit(u1,0), Digit(v1,0));
		integer a;

		a = (integer) grab_num(1);
		Digit(a,0) = g;

		return int_canon(a);
	}

	/* Multi-precision binary gcd algorithm */

	{	long k = 0;
		integer t, u, v;

		u = (integer) Copy((value) u1);
		v = (integer) Copy((value) v1);

 /*B1*/		while (Even(Digit(u,0)) && Even(Digit(v,0))) {
			u = int_half(u);
			v = int_half(v);
			if (++k < 0) {
				/*It's a number we can't cope with,
				  with too many common factors 2.
				  It's hardly likely to occur, and syserr is
				  a bit harsh, but a plain error message would
				  imply it's the user's fault, which it ain't.
				*/
				syserr("problematic number");
				k = 0;
			}
		}

 /*B2*/		if (Even(Digit(u,0))) t = (integer) Copy(u);
		else {
			t = int_neg(v);
			goto B4;
		}

		do {
 /*B3*/			do {
				t = int_half(t);
 B4:				;
			} while (Even(Digit(t,0)));

 /*B5*/			if (Msd(t) >= 0) {
				release(u);
				u = t;
			} else {
				release(v);
				v = int_neg(t);
				release(t);
			}

 /*B6*/			t = int_diff(u, v);
			/* t cannot be int_1 since both u and v are odd! */
		} while (t != int_0);

		release(t);
		release(v);

		while (--k >= 0) u = twice(u);
		
		return u;
	}
}
