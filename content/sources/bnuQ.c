/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
#include "b.h"
#include "bobj.h"
#include "bnum.h"


Hidden integer int1div();

/* Product of integer and single "digit" */

Visible integer int1mul(v, n1) integer v; digit n1; {
	integer a;
	digit save, bigcarry, carry = 0;
	double z, zz, n = n1;
	register int i;

	a = (integer) grab_num(Size(v)+2);

	for (i = 0; i < Size(v); ++i) {
		z = Digit(v,i) * n;
		bigcarry = zz = floor(z/BASE);
		carry += z - zz*BASE;
		Digit(a,i) = save = Modulo(carry, BASE);
		carry = (carry-save)/BASE + bigcarry;
	}

	Digit(a,i) = save = Modulo(carry, BASE);
	Digit(a,i+1) = (carry-save)/BASE;

	return int_canon(a);
}


/* Quotient of positive integer and single "digit" > 0 */

Hidden integer int1div(v, n1) integer v; digit n1; {
	integer q;
	double r_over_n, r = 0, n = n1;
	register int i;

	q = (integer) grab_num(Size(v));
	for (i = Size(v)-1; i >= 0; --i) {
		r = r*BASE + Digit(v,i);
		Digit(q,i) = r_over_n = floor(r/n);
		r -= r_over_n * n;
	}

	int_qrem = mk_int(r);	/* Store remainder; int_qdiv is set by int_quot */

	return int_canon(q);
}


/* Quotient of two integers */

Visible integer int_quot(v1, w1) integer v1, w1; {
	integer a;
	int sign = 1, rel_v = 0, rel_w = 0;

	if (int_qdiv != 0) {	/* Release remainder of previous call */
		release(int_qrem);
		int_qdiv = 0;	/* Flag remainder released */
	}

	if (w1 == int_0) syserr("zero division (int_quot)");

	/* Make v, w positive */
	if (v1 != int_0 && Msd(v1) < 0) {
		sign = -1;
		++rel_v;
		v1 = int_neg(v1);
	}

	if (Msd(w1) < 0) {
		sign *= -1;
		++rel_w;
		w1 = int_neg(w1);
	}

	int_qdiv = sign;

	/* Check v << w or single-digit w */
	if (Size(v1) < Size(w1) ||
		Size(v1) == Size(w1) && Msd(v1) < Msd(w1)) {
		a = (integer) Copy(int_0);
		int_qrem = (integer) Copy(v1);
	} else if (Size(w1) == 1) {
		/* Single-precision division */
		a = int1div(v1, Digit(w1,0));
	} else {
		/* Multi-precision division */
		/* Cf. Knuth II Sec. 4.3.1. Algorithm D */
		/* Note that we count in the reverse direction (not easier!) */

		double z, zz;
		digit carry, save, bigcarry;
		double q, d = BASE/(Msd(w1)+1);
		register int i, j, k;
		integer v, w;
		digit vj;

		/* Normalize: make Msd(w) >= BASE/2 by multiplying
		   both v and w by d */

		v = int1mul(v1, (digit)d);
			/* v is used as accumulator, must make a copy */
			/* v cannot be int_1 */
			/* (then it would be one of the cases above) */

		if (d == 1) w = (integer) Copy(w1);
		else w = int1mul(w1, (digit)d);

		a = (integer) grab_num(Size(v1)-Size(w)+1);

		/* Division loop */

		for (j = Size(v1), k = Size(a)-1; k >= 0; --j, --k) {
			vj = j >= Size(v) ? 0 : Digit(v,j);

			/* Find trial digit */

			if (vj == Msd(w)) q = BASE-1;
			else q = floor( ((double)vj*BASE
					+ Digit(v,j-1)) / Msd(w) );

			/* Correct trial digit */

			while (Digit(w,Size(w)-2) * q >
				((double)vj*BASE + Digit(v,j-1)
					- q*Msd(w)) *BASE + Digit(v,j-2))
				--q;

			/* Subtract q*w from v */

			carry = 0;
			for (i = 0; i < Size(w) && i+k < Size(v); ++i) {
				z = Digit(w,i) * q;
				bigcarry = zz = floor(z/BASE);
				carry += Digit(v,i+k) - z + zz*BASE;
				Digit(v,i+k) =
					save = Modulo(carry, BASE);
				carry = (carry-save)/BASE - bigcarry;
			}

			if (i+k < Size(v)) carry += Digit(v,i+k);
			save = Modulo(carry, BASE);
			if (i+k < Size(v)) Digit(v,i+k) = save;
			carry = (carry-save)/BASE;

			/* Add back necessary? */

			if (carry >= 0)		/* No */
				Digit(a,k) = q;
			else {		/* Yes, add back */
		/* This code is almost never used.  Try to find a test case!
		   However, it has been tested with BASE set to 10. */
				Digit(a,k) = q-1;
				carry = 0;
				for (i = 0; i < Size(w) && i+k < Size(v); ++i) {
					carry += Digit(v, i+k) + Digit(w,i);
					Digit(v,i+k) =
						save = Modulo(carry, BASE);
					carry = (carry-save)/BASE;
				}
				if (i+k < Size(v))
					Digit(v,i+k) = Modulo(carry, BASE);
			}
		}	/* End for(j) */

		int_qrem = int_canon(v);	/* Store remainder for int_qmod() */
		int_qdiv = sign*d;	/* Store normalization factor */
		release(w);
		a = int_canon(a);
	}

	if (rel_v) release(v1);
	if (rel_w) release(w1);

	if (sign < 0) {
		integer temp = a;
		a = int_neg(a);
		release(temp);
	}

	return a;
}


/* Yield remainder of division, saved by int_quot() */

Visible integer int_qmod(u) integer u; {
	integer v, w;
	digit d = int_qdiv;	/* Saved normalization factor from int_quot() */

	if (d == 0) syserr("int_qmod() while int_qdiv==0");

	int_qdiv = 0;	/* Flag int_qrem not to be released */

	if (d < 0) {	/* Swap signs to make d positive for int1div() */
		v = int_neg(int_qrem);
		release(int_qrem);
		d = -d;
	} else
		v = int_qrem;

	if (d > 1) {	/* Divide by d to get proper remainder back */
		w = int1div(v, d);
			/* The ice is thin here: int1div stores its remainder
			   in int_qrem, but does not set int_qdiv. */
		release(int_qrem);
		release(v);
	} else
		w = v;

	if (w != int_0 && (Msd(w) < 0) + (Msd((integer)u) < 0) == 1) {
		/* Make same sign as u */
		v = int_sum(u, w);
		release(w);
		return v;
	}

	return w;
}
