/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* bobj.h: objects */
/* B values, locations, environments */


#define Type(v) ((v)->type)
#define Length(v) ((v)->len)
#define Refcnt(v) ((v)->refcnt)
#define Unique(v) ((v)->refcnt==1)

#define Over(t) for (k= 0; k < Length(t); k++)
#define Overall for (k= 0; k <       len; k++)
#define Last()	(k == len-1)
#define Ats(v) ((value *)&((v)->cts))
#define Str(v) ((string)&((v)->cts))
#define Cts(v) (*Ats(v))
#define Dts(v) (*(Ats(v)+1))

/* The following count the elements from 0 to length-1,
   and do not take a copy */

#define Key(t, i) (Cts(*(Ats(t)+i)))
#define Assoc(t, i) (Dts(*(Ats(t)+i)))
#define Field(c, i) (*(Ats(c)+i))

/****************************** general ******************************/
typedef intlet relation; /* < 0, == 0, > 0 */
extern relation compare();

#define Is_text(v) (Type(v) == Tex)
#define Is_number(v) (Type(v) == Num)
#define Is_compound(v) (Type(v) == Com)
#define Is_list(v) (Type(v) == Lis || Type(v) == ELT)
#define Is_table(v) (Type(v) == Tab || Type(v) == ELT)
#define Is_tlt(v) (Type(v)==Tex || Type(v)==Lis || Type(v)==Tab || Type(v)==ELT)
#define Is_ELT(v) (Type(v) == ELT)

#define Is_simploc(v) (Type(v) == Sim)
#define Is_tbseloc(v) (Type(v) == Tse)
#define Is_trimloc(v) (Type(v) == Tri)
#define Is_refinement(v) (Type(v) == Ref)
#define Is_formal(v) (Type(v) == For)
#define Is_shared(v) (Type(v) == Glo)
#define Is_filed(v) (Type(v) == Per)
#define Is_function(v) (Type(v) == Fun)
#define Is_predicate(v) (Type(v) == Prd)
#define Is_howto(v) (Type(v) == How)

extern value grab_num();
extern value regrab_num();
extern value grab_rat();
extern value grab_approx();
extern value grab_tex();
extern value grab_com();
extern value grab_elt();
extern value grab_lis();
extern value grab_tab();
extern value grab_sim();
extern value grab_tri();
extern value grab_tse();
extern value grab_how();
extern value grab_for();
extern value grab_glo();
extern value grab_per();
extern value grab_fun();
extern value grab_prd();
extern value grab_ref();
extern value copy();
extern release();
extern uniql();
extern uniq_assoc();
extern double hash();

/****************************** Texts ******************************/
extern string strcpy(), strncpy(), strcat(), sprintf(), index();

extern value mk_text();
extern char charval();
extern value concat();
extern concato();
extern value trim();
extern value reptext();
extern value adjleft();
extern value center();
extern value adjright();
extern value convert();
extern string convnum();
extern inittex();

/****************************** Numbers ******************************/

/* Predicates */
extern bool integral();

/* Conversion of abstract values to concrete objects */
extern double numval();     /* numeric value of any number */
extern int intval();        /* numeric value of integral number */
extern intlet propintlet(); /* int argument, checks if in bounds for intlet */
extern char *convnum();     /* character string approximation of any number */
extern int numcomp();       /* Comparison of two numbers: yields -1, 0 or 1 */
extern double numhash();    /* Hashes any abstract number to a 'double' */

/* Conversion of concrete objects to abstract numbers */
extern value numconst();    /* string argument */
extern value mk_integer();  /* int argument */
extern value mk_approx();   /* double argument */
extern value mk_exact();    /* two int arguments */

/* Functions on numbers */
extern value sum();
extern value diff();
extern value negated();
extern value prod();
extern value quot();
extern value modulo();
extern value floorf();
extern value ceilf();
extern value round1();
extern value round2();
extern value mod();
extern value power();
extern value absval();
extern value signum();
extern value numerator();
extern value denominator();
extern value approximate();
extern double random();
extern value root1();
extern value sin1();
extern value cos1();
extern value tan1();
extern value atn1();
extern value exp1();
extern value log1();
extern value root2();
extern value atn2();
extern value log2();
extern value pi();
extern value e();

/****************************** Compounds ******************************/
#define mk_compound(len) grab_com(len)

/****************************** Lists ******************************/
extern value mk_elist();
extern value mk_numrange();
extern value mk_charrange();
extern insertl();
extern removel();

/****************************** Tables ******************************/
extern value mk_etable();
extern value keys();
extern replace();
extern tte_delete();
extern value *adrassoc();
extern value *envassoc();
extern bool in_env();

/****************************** Texts, Lists, and Tables *******************/
extern value mk_elt();
extern bool in();
extern bool is_tlt();
extern value size();
extern value size2();
extern value min1();
extern value min2();
extern value max1();
extern value max2();
extern value th_of();
extern value thof();

/****************************** Other kinds of value ************************/

#define Simploc(l) ((simploc *)Ats(l))
#define Tbseloc(l) ((tbseloc *)Ats(l))
#define Trimloc(l) ((trimloc *)Ats(l))
#define Funprd(f)  ((funprd *)Ats(f))
#define How_to(u)  ((how *)Ats(u))
#define Formal(p)  ((formal *)Ats(f))
#define Refinement(r) ((ref *)Ats(r))

extern loc mk_simploc();
extern loc mk_trimloc();
extern loc mk_tbseloc();

extern value mk_per();
extern fun mk_fun();
extern prd mk_prd();
extern value mk_how();
extern value mk_ref();

/* environments and context */
extern env curnv; extern value *bndtgs;
extern literal cntxt, resexp; extern value uname;
extern intlet cur_ilev, lino; extern txptr tx, ceol;

extern context read_context;
extern context how_context;

extern bool xeq;

extern envtab prmnvtab;
extern env prmnv;

extern value* lookup();
extern setprmnv();
extern sv_context();
extern restore_env();
extern set_context();
extern extbnd_tags();
extern initenv();
extern re_env();
