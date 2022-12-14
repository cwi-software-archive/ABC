/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* B expression evaluation */
#include "b.h"
#include "bcon.h"
#include "bobj.h"
#include "bmem.h" /* for ptr */
#include "bsyn.h"
#include "bsem.h"
#include "bsou.h"

/*************************************************************/
/*                                                           */
/* The operand and operator stacks are modelled as compounds */
/* whose first field is the top and whose second field is    */
/* the remainder of the stack (i.e., linked lists).          */
/* A cleaner and more efficient implementation of            */
/* these heavily used stacks would be in order.              */
/*                                                           */
/*************************************************************/

/* nd = operand, tor = operator (function) */

Hidden value ndstack, torstack;
#define Bot Vnil
Hidden fun Bra, Ket;

Hidden value constant();
Hidden value conversion();
Hidden Procedure pop_stack();
Hidden value popnd();
Hidden Procedure pushnd();
Hidden Procedure pushmontor();
Hidden Procedure pushdyator();
Hidden value statabsel();
Hidden value basexpr();
Hidden bool clocondis();
Hidden bool primary();
Hidden intlet trimi();
Hidden value text_dis();
Hidden value tl_dis();

Visible Procedure inittors() {
	ndstack= torstack= Vnil;
	Bra= mk_fun(-1, -1, Mon, (literal)Dummy, (txptr)Dummy, (txptr)Dummy, (value)Dummy, (bool)Dummy);
	Ket= mk_fun( 0,  0, Dya, (literal)Dummy, (txptr)Dummy, (txptr)Dummy, (value)Dummy, (bool)Dummy);
}

Hidden Procedure pop_stack(stack) value *stack; {
	value oldstack= *stack;
	*stack= Field(*stack, 1);
	put_in_field(Vnil, &oldstack, 0); put_in_field(Vnil, &oldstack, 1);
	release(oldstack);
}

Hidden value popnd() {
	value r;
	if (ndstack == Vnil) syserr("operand stack underflow");
	r= Field(ndstack, 0);
	pop_stack(&ndstack);
	return r;
}

Hidden Procedure pushnd(nd) value nd; {
	value s= ndstack;
	ndstack= mk_compound(2);
	put_in_field(nd, &ndstack, 0); put_in_field(s, &ndstack, 1);
}

Hidden Procedure pushmontor(tor) value tor; {
	value s= torstack;
	torstack= mk_compound(2);
	put_in_field(tor, &torstack, 0); put_in_field(s, &torstack, 1);
}

Hidden Procedure pushdyator(tor2) value tor2; {
	value tor1; funprd *t1, *t2= Funprd(tor2);
	intlet L1, H1, L2= t2->L, H2= t2->H;
 prio:	if (torstack == Vnil) syserr("operator stack underflow");
	tor1= Field(torstack, 0); t1= Funprd(tor1),
	L1= t1->L; H1= t1->H;
	if (L2 > H1)
		if (tor2 == Ket) {
			if (tor1 != Bra)
				syserr("local operator stack underflow");
			pop_stack(&torstack);
		}
		else pushmontor(tor2);
	else if (L1 >= H2) {
		value nd1= Vnil, nd2= popnd();
		if (t1->adic == Dya) nd1= popnd();
		pushnd(formula(nd1, tor1, nd2));
		if (xeq) {
			release(nd2);
			release(nd1);
		}
		pop_stack(&torstack);
		goto prio;
	} else pprerr("priorities? use ( and ) to resolve", "");
}

/*		????
Forward value basexpr();
Forward value text_dis();
Forward value tl_dis();
*/

Hidden value statabsel(t, k) value t, k; {
	/* temporary, while no static type check */
	return mk_elt();
}

Visible value expr(q) txptr q; {
	value c, v; txptr i, j; intlet len, k;
	if ((len= 1+count(",", q)) == 1) return basexpr(q);
	c= mk_compound(len);
	Overall {
		if (!Last()) req(",", q, &i, &j);
		else i= q;
		v= basexpr(i);
		put_in_field(v, &c, k);
		if (!Last()) tx= j;
	}
	return c;
}

Hidden value basexpr(q) txptr q; {
	value v= obasexpr(q);
	Skipsp(tx);
	if (tx < q && Char(tx) == ',')
		parerr("no commas allowed in this context", "");
	upto(q, "expression");
	return v;
}

/*	????
Forward bool primary(), clocondis();
*/

#define Pbot {pushnd(Bot); pushmontor(Bra);}
#define Ipush if (!pushing) {Pbot; pushing= Yes;}
#define Fpush if (pushing) {                                    \
		      pushnd(v); pushdyator(Ket); v= popnd();   \
		      if (popnd() != Bot) syserr(               \
			      xeq ? "formula evaluation awry" : \
				      "formula parsing awry");  \
	      }

Visible value obasexpr(q) txptr q; {
	value v, t; bool pushing= No;
 nxtnd:	Skipsp(tx);
	nothing(q, "expression");
	t= tag();
	if (primary(q, t, &v, Yes)) /* then t is released */;
	else if (t != Vnil) {
		value f;
		if (is_monfun(t, &f)) {
			release(t);
			Ipush;
			pushmontor(f);
			goto nxtnd;
		} else {
			release(t);
			error("target has not yet received a value");
		}
	} else if (Montormark(Char(tx))) {
		Ipush;
		pushmontor(montor());
		goto nxtnd;
	} else parerr("no expression where expected", "");
	/* We are past an operand and look for an operator */
	Skipsp(tx);
	if (tx < q) {
		txptr tx0= tx; bool l, e, g;
		if (Letter(Char(tx))) {
			fun f;
			t= tag();
			if (is_dyafun(t, &f)) {
				release(t);
				Ipush;
				pushnd(v);
				pushdyator(f);
				goto nxtnd;
			}
			release(t);
		} else if (relop(&l, &e, &g));
		else if (Dyatormark(Char(tx))) {
			Ipush;
			pushnd(v);
			pushdyator(dyator());
			goto nxtnd;
		}
		tx= tx0;
	}
	Fpush;
	return v;
}

Hidden bool clocondis(q, p) txptr q; value *p; {
	txptr i, j;
	Skipsp(tx);
	nothing(q, "expression");
	if (Char(tx) == '(') {
		tx++; req(")", q, &i, &j);
		*p= expr(i); tx= j;
		return Yes;
	}
	if (Dig(Char(tx)) || Char(tx) == '.' || Char(tx) == 'E' &&
	   (Dig(Char(tx+1)) || Char(tx+1)=='+' || Char(tx+1)=='-')) {
		*p= constant(q);
		return Yes;
	}
	if (Char(tx) == '\'' || Char(tx) == '"') {
		*p= text_dis(q);
		return Yes;
	}
	if (Char(tx) == '{') {
		*p= tl_dis(q);
		return Yes;
	}
	return No;
}

Hidden bool primary(q, t, p, tri) txptr q; value t, *p; bool tri; {
/* If a tag has been seen, it is held in t.
   Releasing t is a task of primary, but only if the call succeeds. */
	fun f; value tt, relt= Vnil; value *aa= &t;
	if (t != Vnil) /* tag */ {
		if (xeq) {
			tt= t;
			aa= lookup(t);
			if (aa == Pnil) {
				if (is_zerfun(t, &f)) {
					t= formula(Vnil, f, Vnil);	  
					aa= &t;
				} else return No;
			} else if (Is_refinement(*aa)) {
				ref_et(*aa, Ret); t= resval; resval= Vnil;
				aa= &t;
			} else if (Is_formal(*aa)) {
				t= eva_formal(*aa);
				aa= &t;
			} else if (Is_shared(*aa)) {
				if (!in_env(prmnv->tab, t, &aa)) return No;
				if (Is_filed(*aa))
					if (!is_tloaded(t, &aa)) return No;
				t= Vnil;
			} else if (Is_filed(*aa)) {
				if (!is_tloaded(t, &aa)) return No;
				t= Vnil;
			} else t= Vnil;
			release(tt);
		}
	} else if (clocondis(q, &t)) aa= &t;
	else return No;
	Skipsp(tx);
	while (tx < q && Char(tx) == '[') {
		txptr i, j; value s;
		tx++; req("]", q, &i, &j);
		s= expr(i); tx= j;
		/* don't copy table for selection */
		if (xeq) {
			aa= adrassoc(*aa, s, Val);
			release(s);
			relt= t;
			if (aa == Pnil) error("key not in table");
		} else {
			t= statabsel(tt= t, s);
			release(tt); release(s);
		}
		Skipsp(tx);
	}
	if (tri && tx < q && (Char(tx) == '@' || Char(tx) == '|')) {
		intlet B, C;
		if (xeq && !Is_text(*aa))
			parerr("in t@p or t|p, t is not a text", "");
		trimbc(q, xeq ? Length(*aa) : 0, &B, &C);
		if (xeq) {
			relt= t;
			t= trim(*aa, B, C);
			aa= &t;
		}
	}
	*p= t == Vnil || relt != Vnil ? copy(*aa) : t;
	release(relt);
	return Yes;
}

/*		???
Forward intlet trimi();
*/

Visible Procedure trimbc(q, len, B, C) txptr q; intlet len, *B, *C; {
	char bc; intlet N;
	*B= *C= 0;
	while (tx < q && (Char(tx) == '@' || Char(tx) == '|')) {
		bc= Char(tx++);
		N= trimi(q);
		if (bc == '@') *B+= N-1;
		else *C+= (len-*B-*C)-N;
		if (*B < 0 || *C < 0 || *B+*C > len)
			error("in t@p or t|p, p is out of bounds");
		Skipsp(tx);
	}
}

Hidden intlet trimi(q) txptr q; {
	value v, t; bool pushing= No;
 nxtnd:	Skipsp(tx);
	nothing(q, "expression");
	t= tag();
	if (primary(q, t, &v, No)); /* then t is released */
	else if (t != Vnil) {
		value f;
		if (is_monfun(t, &f)) {
			release(t);
			Ipush;
			pushmontor(f);
			goto nxtnd;
		} else {
			release(t);
			error("target has not yet received a value");
		}
	} else if (Montormark(Char(tx))) {
		Ipush;
		pushmontor(montor());
		goto nxtnd;
	} else parerr("no expression where expected", "");
	Fpush;
	{int ii; intlet i= 0;
		if (xeq) {
			ii= intval(v);
			if (ii < 0) error("in t@p or t|p, p is negative");
			if (ii > Maxintlet)
				error("in t@p or t|p, p is excessive");
			i= ii;
		}
		release(v);
		return i;
	}
}

Hidden value constant(q) txptr q; {
	bool dig= No; intlet ndap; txptr first= tx;
	while (tx < q && Dig(Char(tx))) {
		++tx;
		dig= Yes;
	}
	if (tx < q && Char(tx) == '.') {
		tx++; ndap= 0;
		while (tx < q && Dig(Char(tx))) {
			dig= Yes; ndap++;
			++tx;
		}
		if (!dig) pprerr("point without digits", "");
	}
	if (tx < q && Char(tx) == 'E') {
		tx++;
		if (!(Dig(Char(tx))) && Keymark(Char(tx))) {
			tx--;
			goto done;
		}
		if (tx < q && (Char(tx) == '+' || Char(tx) == '-')) ++tx;
		dig= No;
		while (tx < q && Dig(Char(tx))) {
			dig= Yes;
			++tx;
		}
		if (!dig) parerr("E not followed by exponent", "");
	}
 done:	return numconst(first, tx);
}

Hidden char txdbuf[TXDBUFSIZE];
Hidden txptr txdbufend= &txdbuf[TXDBUFSIZE];

Hidden value text_dis(q) txptr q; {
	char aq[2]; txptr tp= txdbuf; value t= mk_text("");
	aq[1]= '\0'; *aq= Char(tx++);
 fbuf:	while (tx < q && Char(tx) != *aq) {
		if (Char(tx) == '`') {
			if (Char(tx+1) == '`') tx++;
			else {
				value cv;
				*tp= '\0';
				concato(&t, txdbuf);
				concato(&t, Str(cv= conversion(q)));
				release(cv);
				tp= txdbuf; goto fbuf;
			}
		}
		*tp++= Char(tx++);
		if (tp+1 >= txdbufend) {
			*(txdbufend-1)= '\0';
			concato(&t, txdbuf);
			tp= txdbuf;
		}
	}
	if (tx >= q) parerr("cannot find matching ", aq);
	if (++tx < q && Char(tx) == *aq) {
		*tp++= Char(tx++);
		goto fbuf;
	}
	*tp= '\0';
	concato(&t, txdbuf);
	return t;
}

Hidden value conversion(q) txptr q; {
	txptr f, t; value v, c;
	thought('`');
	req("`", q, &f, &t);
	v= expr(f); c= Ifxeq(convert(v, Yes, Yes));
	if (xeq) release(v);
	tx= t; return c;
}

Hidden value tl_dis(q) txptr q; {
	txptr f, t, ff, tt;
	intlet len, k;
	thought('{');
	Skipsp(tx);
	if (Char(tx) == '}') {
		tx++;
		return Ifxeq(grab_elt());
	}
	req("}", q, &f, &t);
	if (find("..", f, &ff, &tt)) {
		value enu, lo, hi;
		lo= basexpr(ff);
		if (!xeq || Is_number(lo)) {
			tx= tt; while (Char(tx) == '.') tx++;
			hi= basexpr(f);
			if (xeq) {
				value entries, one;
				if (!integral(lo))
				  error("in {p..q}, p is a number but not an integer");
				if (!Is_number(hi))
				  error("in {p..q}, p is a number but q is not");
				if (!integral(hi))
				  error("in {p..q}, q is a number but not an integer");
				entries= diff(lo, hi);
				one= mk_integer(1);
				if (compare(entries, one)>0)
					error("in {p..q}, integer q < x < p");
				enu= mk_numrange(lo, hi);
				release(entries); release(one);
			} else enu= mk_elist();
			release(hi); release(lo);
		} else if (Is_text(lo)) {
			char a, z;
			if (Length(lo) != 1)
			  error("in {p..q}, p is a text but not a character");
			tx= tt; hi= basexpr(f);
			if (!Is_text(hi))
			  error("in {p..q}, p is a text but q is not");
			if (Length(hi) != 1)
			error("in {p..q}, q is a text but not a character");
			a= charval(lo); z= charval(hi);
			if (z < a-1) error("in {p..q}, character q < x < p");
			enu= mk_charrange(lo, hi);
			release(lo); release(hi);
		} else error("in {p..q}, p is neither a number nor a text");
		tx= t; return enu;
	}
	len= 1+count(";", f);
	Skipsp(tx);
	if (Char(tx) == '[') {
		value ta, key, a;
		ta= mk_etable();
		Overall {
			Skipsp(tx);
			need("[");
			req("]", f, &ff, &tt);
			key= expr(ff); tx= tt;
			need(":");
			if (!Last()) req(";", f, &ff, &tt);
			else {ff= f; tt= t;}
			a= basexpr(ff); tx= tt;
			replace(&ta, key, a, Val);
			release(key); release(a);
		}
		return ta;
	}
	{value l, e;
		l= mk_elist();
		Overall {
			if (!Last()) req(";", f, &ff, &tt);
			else {ff= f; tt= t;}
			e= basexpr(ff); tx= tt;
			insertl(e, &l);
			release(e);
		}
		return l;
	}
}
