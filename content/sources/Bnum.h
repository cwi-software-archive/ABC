/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/

/* Private definitions for the numerical package */

/*************** Definitions exported for integers *****************/

typedef long digit;

typedef struct {
	literal	type;
	intlet  refcnt;
	intlet  len;
	digit	dig[1];
} *integer;

#define BASE 100000000l

extern integer int_sum();
extern integer int_diff();
extern integer int_neg();
extern integer int_canon();
extern integer int_prod();
extern integer int_quot();
extern integer int_gcd();
extern integer mk_int();
extern integer int1mul();
extern integer int_tento();
extern integer int_half();

/* Stuff to get positive remainder after a division */
extern integer int_qrem;
extern digit	int_qdiv;
extern integer int_qmod();

extern double floor();

extern integer int_0;
extern integer int_1;
extern integer int_2;
extern integer int_10;

extern int printwidth;

#define Integral(v) (Length(v)>=0)
#define Modulo(a,b) (((a)%(b)+(b))%(b))
#define Digit(v,n) ((v)->dig[n])
#define Size(v) Length(v)
#define Msd(v) Digit(v,Size(v)-1)

#define Odd(x) ((x)&1)
#define Even(x) (!Odd(x))

/* Provisional definitions */

extern value copy();
#define Copy(x) copy((value)(x))

/***************** Definitions exported for rationals *****************/

typedef struct {
	literal	type;
	intlet  refcnt;
	intlet  len;
	integer	num, den;
} *rational;


#define Numerator(a) ((a)->num)
#define Denominator(a) ((a)->den)
#define Rational(a) (Size(a)<-1)
#define Roundsize(a) (-2-Size(a))

extern rational mk_rat();
extern rational rat_sum();
extern rational rat_diff();
extern rational rat_neg();
extern rational rat_prod();
extern rational rat_quot();
extern rational rat_power();

extern rational rat_zero;
extern rational rat_half;

/***************** Definitions exported for approximate numbers *************/

typedef struct {
	literal	type;
	intlet  refcnt;
	intlet  len;
	double	rval;
} *real;

#define Realval(v) ((v)->rval)

#define Approximate(v) (Size(v)==-1)
#define Exact(v) (!Approximate(v))

extern double frexp();
extern double ldexp();
extern double modf();
extern double floor();
extern double ceil();
extern double fabs();

extern real app_sum();
extern real app_diff();
extern real app_prod();
extern real app_quot();
extern real app_neg();
