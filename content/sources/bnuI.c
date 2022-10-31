/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* Multi-precision integer arithmetic */

#include "b.h"
#include "bobj.h"
#include "bnum.h"
#include "bcon.h"

char *sprintf(); /*OS*/

/*
 * Number representation:
 * ======================
 *
 * (Think of BASE = 10 for ordinary decimal notation.)
 * A number is a sequence of N "digits" b1, b2, ..., bN
 * where each bi is in {0..BASE-1}, except for negative numbers,
 * where bN = -1.
 * The number represented by b1, ..., bN is
 *      b1*BASE**(N-1) + b2*BASE**(N-2) + ... + bN .
 * The base BASE is chosen so that multiplication of two positive
 * integers up to BASE-1 can be multiplied exactly using double
 * precision floating point arithmetic.
 * Also it must be possible to add two long integers between
 * -BASE and +BASE (exclusive), giving a result between -2BASE and
 * +2BASE.
 * BASE must be even (so we can easily decide whether the whole
 * number is even), and positive (to avoid all kinds of other trouble).
 * Presently, it is restricted to a power of 10 by the I/O-conversion
 * routines (file bnuC.c).
 *
 * On the VAX, BASE is 100000000 (= 10**8).
 * The theoretical maximum on the VAX is 2**28, as double precision
 * calculations are carried out with 56 bits precision.
 *
 * Canonical representation:
 * bN is never zero (for the number zero itself, N is zero).
 * If bN is -1, b[N-1] is never BASE-1 .
 * All operands are assumed te be in canonical representation.
 * Routine "int_canon" brings a number in canonical representation.
 *
 * Mapping to C objects:
 * A "digit" is an integer of type "digit", probably a "long int".
 * A number is represented as a "B-integer", i.e. something
 * of type "integer" (which is actually a pointer to some struct).
 * The number of digits N is extracted through the macro Size(v).
 * The i-th digit is extracted through the macro Digit(v,N-i).
 * (So in C, we count in a backwards direction from 0 ... n-1 !)
 * A number is created through a call to grab_num(N), which sets
 * N zero digits (thus not in canonical form!).
 */


int printwidth;	/* max. no. of decimal digits per "digit" */

/*
 * Bring an integer into canonical form.
 * Also, replace integers that are zero or one by int_0
 * or int_1, resp.
 */

Visible integer int_canon(v) integer v; {
	register int i;

	for (i = Size(v) - 1; i >= 0 && Digit(v,i) == 0; --i)
		;

	if (i < 0) {
		release((value) v);
		return (integer) Copy(int_0);
	}

	if (i == 0 && Digit(v,0) == 1) {
		release((value) v);
		return (integer) Copy(int_1);
	}

	if (i > 0 && Digit(v,i) == -1) {
		while (i > 0 && Digit(v, i-1) == BASE-1) --i;
		Digit(v,i) = -1;
	}

	if (i+1 < Size(v)) return (integer) regrab_num(v, i+1);

	return v;
}


/* Sum of two integers */

Visible integer int_sum(v, w) integer v, w; {
	integer s;
	register int i;
	digit carry = 0, save;

	if (Size(v) < Size(w)) /* Interchange v, w */
		{ integer temp = v;  v = w;  w = temp; }

	s = (integer) grab_num(Size(v) + 1);

	for (i = 0; i < Size(w); ++i) {
		carry += Digit(v, i) + Digit(w, i);
		Digit(s, i) = save = Modulo(carry, BASE);
		carry = (carry - save) / BASE;
	}

	for ( ; i < Size(v); ++i) {
		carry += Digit(v, i);
		Digit(s, i) = save = Modulo(carry, BASE);
		carry = (carry - save) / BASE;
	}

	Digit(s, i) = carry;	/* Always highest digit */

	return int_canon(s);
}


/* Difference of two integers */

Visible integer int_diff(v, w) integer v, w; {
	integer s;
	register int i, minsize;
	digit carry = 0, save;

	if (Size(v) < Size(w)) {
		i = Size(w);
		minsize = Size(v);
	} else {
		i = Size(v);
		minsize = Size(w);
	}

	s = (integer) grab_num(i+1);

	for (i = 0; i < minsize; ++i) {
		carry += Digit(v, i) - Digit(w, i);
		Digit(s, i) = save = Modulo(carry, BASE);
		carry = (carry - save) / BASE;
	}

	/* At most one of the next two loops executes! */

	for ( ; i < Size(v); ++i) {
		carry += Digit(v, i);
		Digit(s, i) = save = Modulo(carry, BASE);
		carry = (carry - save) / BASE;
	}

	for ( ; i < Size(w); ++i) {
		carry -= Digit(w, i);
		Digit(s, i) = save = Modulo(carry, BASE);
		carry = (carry - save) / BASE;
	}

	Digit(s, i) = carry;	/* Always highest digit */

	return int_canon(s);
}


/* Negative of an integer */

Visible integer int_neg(v) integer v; {
	return int_diff(int_0, v);
}


/* Product of two integers */

Visible integer int_prod(v, w) integer v, w; {
	digit carry, save, bigcarry;
	double z, zz;
	register int i, j;
	integer a;

	if (v == int_0 || w == int_0) return (integer) Copy(int_0);
	if (v == int_1) return (integer) Copy(w);
	if (w == int_1) return (integer) Copy(v);

	a = (integer) grab_num(Size(v) + Size(w));

	for (i = 0; i < Size(v); ++i) {
		carry = 0;
		for (j = 0; j < Size(w); ++j) {
			/* double z, zz; */
			z = (double)Digit(v,i) * (double)Digit(w,j);
			bigcarry = zz = floor(z/BASE);
			carry += z - zz*BASE + Digit(a,i+j);
			Digit(a,i+j) = save = Modulo(carry, BASE);
			carry = (carry-save)/BASE + bigcarry;
		}
		Digit(a,i+j) = carry;
	}

	return int_canon(a);
}


/* Compare two integers */

Visible int_comp(v, w) integer v, w; {
	int sv, sw;
	register int i;

	/* 1. Compare pointers */
	if (v == w) return 0;

	/* 2. Extract signs */
	sv = Size(v)==0 ? 0 : Digit(v,Size(v)-1)<0 ? -1 : 1;
	sw = Size(w)==0 ? 0 : Digit(w,Size(w)-1)<0 ? -1 : 1;

	/* 3. Compare signs */
	if (sv != sw) return (sv>sw) - (sv<sw);

	/* 4. Compare sizes */
	if (Size(v) != Size(w))
		return sv * ( (Size(v)>Size(w)) - (Size(v)<Size(w)) );

	/* 5. Compare individual digits */
	for (i = Size(v)-1; i >= 0 && Digit(v,i) == Digit(w,i); --i)
		;

	/* 6. All digits equal? */
	if (i < 0) return 0;  /* Yes */

	/* 7. Compare leftmost different digits */
	if (Digit(v,i) < Digit(w,i)) return -1;

	return 1;
}


/* Construct an integer out of a floating point number */

#define GRAN 8	/* Granularity used when requesting more storage */
		/* MOVE TO MEM! */
Visible integer mk_int(x) double x; {
	register integer a = (integer) grab_num(1);
	integer b;
	register int i, j;
	int negate = x < 0 ? 1 : 0;

	if (negate) x = -x;

	for (i = 0; x != 0; ++i) {
		double z = floor(x/BASE);
		digit save = Modulo((digit)(x-z*BASE), BASE);
		if (i >= Size(a)) {
			a = (integer) regrab_num(a, Size(a)+GRAN);
			for (j = Size(a)-1; j > i; --j)
				Digit(a,j) = 0;	/* clear higher digits */
		}
		Digit(a,i) = save;
		x = floor((x-save)/BASE);
	}

	if (negate) {
		b = int_neg(a);
		release(a);
		return b;
	}

	return int_canon(a);
}


/* Efficiently compute 10**n as a B integer, where n is a C int >= 0 */

Visible integer int_tento(n) int n; {
	integer i;
	digit msd = 1;
	if (n < 0) syserr("int_tento(-n)");
	if (n == 0) return (integer) Copy(int_1);
	i = (integer) grab_num(1 + (int)(n/printwidth));
	while (n % printwidth != 0) msd *= 10, --n;
	Msd(i) = msd;
	return i;
}


/* Approximate ceiling(10 log abs(u/v)), as C int.
   It only works for v > 0, u, v both integers.
   The result may be one too large or too small */

Visible Procedure scale(u, v) integer u, v; {
	int s = (Size(u) - Size(v)) * printwidth;
	double z;
	if (Size(v) == 0 || Msd(v) < 0) syserr("scale(u,v<=0)");
	if (Size(u) == 0) return 0; /* `Don't care' case */
	if (Msd(u) >= 0) z = Msd(u);
	else if (Size(u) == 1) z = 1;
	else z = BASE - Digit(u, Size(u)-2);
	z /= Msd(v);
	while (z >= 10) z /= 10, ++s;
	while (z < 1) z *= 10, --s;
	return s;
}

Visible Procedure int_init() {
	char buf[20];

	int_0 = (integer) grab_num(0);
	int_1 = (integer) grab_num(1);  Digit(int_1,  0) = 1;
	int_2 = (integer) grab_num(1);  Digit(int_2,  0) = 2;
	int_10 = (integer) grab_num(1); Digit(int_10, 0) = 10;

	VOID sprintf(buf, "%ld", (long)(BASE)-1l);
	printwidth = strlen(buf);
}
