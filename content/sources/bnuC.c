/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
#include "b.h"
#include "bcon.h"
#include "bobj.h"
#include "bmem.h"
#include "bnum.h"
#include "bsyn.h" /* temporary until numconst is fixed */

char *sprintf(); /* OS */
extern value tento();
extern integer int_tento();

#define EXPDIGITS 10	/* Extra positions to allow for exponent part */
			/* -- must be larger than printwidth */
#define MAXDIGITS 15	/* Max precision for fixed/floating numbers */
#define CONVBUFSIZE (MAXDIGITS+4)
			/* Maximum number of digits to print in integer notation */
			/* (4 is the size of 'e+00' added by sprintf) */


/* Convert an integer to a C character string.
   The character string is overwritten on each next call.
   It assumes BASE is a power of 10. */

Hidden char *convint(v) register integer v; {
	static char *buffer;
	register char *cp;
	register int i;
	bool negated = No;

	if (Size(v) == 0) return "0"; /* Shortcut for zero */
	if (Msd(v) < 0) {
		negated = Yes;
		v = int_neg(v);
	}
	if (buffer) freemem(buffer);
	buffer = getmem((unsigned)(Size(v)*printwidth + 1 + negated));
	cp = buffer;
	if (negated) *cp++ = '-';
	sprintf(cp, "%d", Msd(v));
	while (*cp) ++cp;
	for (i = Size(v)-2; i >= 0; --i, cp += printwidth)
		sprintf(cp, "%0*d", printwidth, Digit(v, i));
	if (negated) release(v);
	return buffer;
}


/* Convert a numeric value to a C character string.
   The character string is overwritten on each next call. */

Visible char *convnum(v) register value v; {
	static char convbuf[3+CONVBUFSIZE+EXPDIGITS];
		/* 3 extra for things (sign, 0.) to be stuck on front of it */
	char *bufstart = convbuf+3;
	int i;
	register char *cp = bufstart;
	double x;

	if (Integral(v)) return convint((integer)v);

	/* Rational numbers and reals treated alike.
	   However, not-too-large rationals resulting from
	   'n round x' are transformed to f-format.
	   Also, huge rationals that cannot be transformed
	   to real, are converted to integer (via floor)
	   and printed as integers. */
	
	if (Rational(v)) { /* Provision for huge rationals */
		if (scale(Numerator((rational)v), Denominator((rational)v))
				> CONVBUFSIZE+1) { /* Huge, print floor v */
			v = floorf(v);
			bufstart = convnum(v);
			release(v);
			return bufstart;
		}
	}

	x = numval(v);
	sprintf(bufstart, "%.*g", MAXDIGITS, x);

	for (cp = bufstart; *cp != '\0'; ++cp)
		if (*cp == 'e') {	/* change sprintf's 'e' to 'E' */
			*cp = 'E';
			break;
		}

	if (Rational(v) && Roundsize(v) > 0 && *cp != 'E') {
		int i = Roundsize(v);
		int j = 1;
			/* Counts digits allowed beyond MAXDIGITS, 1 for '.' */

		for (cp = bufstart; *cp == '0'; ++cp)
			++j; /* Allow a trailing zero for each leading zero */

		for (; *cp != '\0' && *cp != '.'; ++cp)
			; /* Find '.' or end of string */

		if (*cp == '\0') {
			*cp = '.'; /* Append '.' if not found */
			*++cp = '\0';
		} else {
			while (*++cp == '0')
				/* Allow more precision if leading zeros */
				++j, --i;
			while (*cp != '\0')
				--i, ++cp; /* Find last digit */
		}

		/* Append extra zeros (but don't show more precision
		   than sprintf can!) */
		while (--i >= 0 && cp < bufstart+MAXDIGITS+j)
			*cp++ = '0';

		*cp = '\0'; /* Append new terminating null byte */
	}

	return bufstart;
}


/* Convert a string to a number.
   Pointers to the first and last+1 characters are given.
   Again, BASE must be a power of 10.
   ********** NEW **********
   All numbers are made exact, even if Exponent notation is used.
   Optimization is thrown out in favor of small and understandable
   algorithm
   ********** WARNING **********
   This routine must be fixed, because it accesses the source buffer
   and it shouldn't because it's in the wrong place in the hierarchy
*/

Visible value numconst(text, end) register txptr text, end; {
	int intlen = 0;
	int fraclen = 0;
	integer a = (integer) Copy(int_0), a1, a2;
	value c;

	/* Integer part: */
	for (; text<end && Char(text)>='0' && Char(text)<='9';
		++intlen, ++text) {
		a1 = int1mul(a, 10l);
		release(a);
		a2 = mk_int((double)(Char(text)-'0'));
		a = int_sum(a1, a2);
		release(a1), release(a2);
	}
	/* Fraction: */
	if (text < end && Char(text) == '.') ++text;
	for (; text<end && Char(text)>='0' && Char(text)<='9';
			++fraclen, ++text) {
		a1 = int1mul(a, 10l);
		release(a);
		a2 = mk_int((double)(Char(text)-'0'));
		a = int_sum(a1, a2);
		release(a1), release(a2);
	}
	/* Exponent: */
	if (text < end && Char(text) == 'E') {
		double expo = 0;
		int sign = 1;
		value b;
		++text;
		if (text < end) {
			if (Char(text) == '+') ++text;
			else if (Char(text) == '-') {
				++text;
				sign = -1;
			}
		}
		for (; text<end && Char(text)>='0' && Char(text)<='9';
				++text) {
			expo = expo*10 + Char(text)-'0';
			if (expo > Maxintlet) {
				error("excessive exponent in E-notation");
				expo = 0;
				break;
			}
		}
		b = tento((int)expo * sign - fraclen);
		if (intlen+fraclen == 0) c = b; /* E without int/fract */
		else {
			c = prod((value)a, b);
			release(b);
		}
#ifndef E_EXACT
		/* Make approximate number if E-notation used */
		b = approximate(c);
		release(c);
		c = b;
#endif
	}
	else {
		integer b = int_tento(fraclen);
		c = mk_exact(a, b, fraclen);
		release(b);
	}
	release(a);
	return c;
}


Visible Procedure printnum(f, v) FILE *f; value v; {
	if (f == NULL) f= stdout;
	if (Approximate(v)) putc('~', f);
	else if (Rational(v) && Denominator((rational)v) != int_1) {
		int i = Roundsize(v);
		fputs(convnum(Numerator((rational)v)), f);
		if (i > 0 && i <= MAXDIGITS) {
			/* The assumption here is that in u/v, the Roundsize
			   of the result is the sum of that of the operands. */
			putc('.', f);
			do putc('0', f); while (--i > 0);
		}
		putc('/', f);
		v = (value) Denominator((rational)v);
	}
	fputs(convnum(v), f);
}
