/* General operations for objects */

#include "b.h"
#include "bobj.h"
#include "bmem.h"
#include "bscr.h" /* TEMPORARY for nwl */
#include "bsem.h" /* TEMPORARY for grab */
#include "bnum.h"
#ifndef SMALLNUMBERS
#include "bnuI.h" /* for grab, grab_num, ccopy */
#include "bnuR.h" /* for grab, grab_rat, ccopy */
#include "bnuA.h" /* for grab, grab_real, ccopy */
#else
#include "bnuM.h" /* For grab */
#endif


#define LL (len < 200 ? 1 : 8)
#define Len (len == 0 ? 0 : ((len-1)/LL+1)*LL)
#define Adj(s) (unsigned) (sizeof(*Vnil)-sizeof(Vnil->cts)+(s))

#define Grabber() {if(len>Maxintlet)syserr("big grabber");}
#define Regrabber() {if(len>Maxintlet)syserr("big regrabber");}

value etxt, elis, etab, elt;

long gr= 0;
Gr(i) {nwl=No;printf(" gr%d:%ld",i,gr);gr=0;}
prgr() {nwl=No;printf(" gr:%ld",gr);gr=0;}

value grab(type, len) literal type; intlet len; {
	unsigned syze; value v;
	Grabber();
	switch (type) {
	case Num:
#ifdef SMALLNUMBERS
		syze= sizeof(number);
#else
		if (len >= 0) syze= Len*sizeof(digit);		/* Integral */
		else if (len == -1) syze= sizeof(double);	/* Approximate */
		else syze= 2*sizeof(value);			/* Rational */
#endif
		break;
	case Tex: syze= (len+1)*sizeof(char); break; /* one extra for the '\0' */
	case Com: syze= len*sizeof(value); break;
	case ELT: syze= (len= 0); break;
	case Lis:
	case Tab: syze= Len*sizeof(value); break;
	case Sim: syze= sizeof(simploc); break;
	case Tri: syze= sizeof(trimloc); break;
	case Tse: syze= sizeof(tbseloc); break;
	case How: syze= sizeof(how); break;
	case For: syze= sizeof(formal); break;
	case Glo: syze= 0; break;
	case Fun:
	case Prd: syze= sizeof(funprd); break;
	case Ref: syze= sizeof(ref); break;
	default:
		printf("\ngrabtype{%c}\n", type);
		syserr("grab called with unknown type");
	}
	v= (value) getmem(Adj(syze), "grab");
	v->type= type; v->len= len; v->refcnt= 1;
gr+=1;
	return v;
}

#ifdef SMALLNUMBERS
value grab_num(len) intlet len; { return grab(Num, len); }
#else
value grab_num(len) register int len; {
	integer v;
	register int i;

	v = (integer) grab(Num, len);
	for (i = Size(v)-1; i >= 0; --i) Digit(v, i) = 0;
	return (value) v;
}


value grab_rat() {
	return (value) grab(Num, -2);
}

value grab_real() {
	return (value) grab(Num, -1);
}

value regrab_num(v, len) value v; register int len; {
	register unsigned syze;

	syze = Len * sizeof(digit);
	regetmem(&v, Adj(syze), "regrab_num");
	Size(v) = len;
	return v;
}
#endif

value grab_tex(len) intlet len; {
	if (len == 0) return copy(etxt);
	return grab(Tex, len);
}

value grab_com(len) intlet len; { return grab(Com, len); }

value grab_elt() { return copy(elt); }

value grab_lis(len) intlet len; {
	if (len == 0) return copy(elis);
	return grab(Lis, len);
}

value grab_tab(len) intlet len; {
	if (len == 0) return copy(etab);
	return grab(Tab, len);
}

value grab_sim() { return grab(Sim, 0); }

value grab_tri() { return grab(Tri, 0); }

value grab_tse() { return grab(Tse, 0); }

value grab_how() { return grab(How, 0); }

value grab_for() { return grab(For, 0); }

value grab_glo() { return grab(Glo, 0); }

value grab_fun() { return grab(Fun, 0); }

value grab_prd() { return grab(Prd, 0); }

value grab_ref() { return grab(Ref, 0); }

value copy(v) value v; {
	if (v != Vnil && v->refcnt < Maxintlet) (v->refcnt)++;
gr+=1;
	return v;
}

release(v) value v; {
	intlet *r= &(v->refcnt);
	if (v == Vnil) return;
	if (*r == 0) syserr("releasing unreferenced value");
if(bugs){printf("releasing: ");wri(v,No,No,No);line();}
	if (*r < Maxintlet && --(*r) == 0) rrelease(v);
gr-=1;
}

value ccopy(v) value v; {
	literal type= v->type; intlet len= v->len, k; value w;
	w= grab(type, len);
	switch (type) {
	case Num:
#ifdef SMALLNUMBERS
		Numerator(w)= Numerator(v);
		Denominator(w)= Denominator(v);
#else
		if (Integral(v)) {
			register int i;
			for (i = len-1; i >= 0; --i)
				Digit((integer)w, i) = Digit((integer)v, i);
		} else if (Approximate(v))
			Realval((real)w) = Realval((real)v);
		else if (Rational(v)) {
			Numerator((rational)w) = Numerator((rational)v);
			Denominator((rational)w) = Denominator((rational)v);
		}
#endif
		break;
	case Tex:
		strcpy(Str(w), Str(v));
		break;
	case Com:
	case Lis:
	case Tab:
	case ELT:
		{value *vp= Ats(v), *wp= Ats(w);
			Overall *wp++= copy(*vp++);
		} break;
	case Sim:
		{simploc *vv= (simploc *)Ats(v), *ww= (simploc *)Ats(w);
			ww->i= copy(vv->i); ww->e= vv->e; /* No copy */
		} break;
	case Tri:
		{trimloc *vv= (trimloc *)Ats(v), *ww= (trimloc *)Ats(w);
			ww->R= copy(vv->R); ww->B= vv->B; ww->C= vv->C;
		} break;
	case Tse:
		{tbseloc *vv= (tbseloc *)Ats(v), *ww= (tbseloc *)Ats(w);
			ww->R= copy(vv->R); ww->K= copy(vv->K);
		} break;
	case How:
		*((how *)Ats(w)) = *((how *)Ats(v));
		break;
	case For:
		*((formal *)Ats(w)) = *((formal *)Ats(v));
		break;
	case Glo:
		break;
	case Fun:
	case Prd:
		*((funprd *)Ats(w)) = *((funprd *)Ats(v));
		break;
	case Ref:
		*((ref *)Ats(w)) = *((ref *)Ats(v));
		break;
	default:
		syserr("ccopy called with unknown type");
	}
	return w;
}

rrelease(v) value v; {
	literal type= v->type; intlet len= v->len, k;
	switch (type) {
	case Num:
	case Tex:
		break;
	case Com:
	case Lis:
	case Tab:
	case ELT:
		{value *vp= Ats(v);
			Overall release(*vp++);
		} break;
	case Sim:
		{simploc *vv= (simploc *)Ats(v);
			release(vv->i); /* No release of vv->e */
		} break;
	case Tri:
		{trimloc *vv= (trimloc *)Ats(v);
			release(vv->R);
		} break;
	case Tse:
		{tbseloc *vv= (tbseloc *)Ats(v);
			release(vv->R); release(vv->K);
		} break;
	case How:
		{how *vv= (how *)Ats(v);
			free((ptr) vv->fux);
			release(vv->reftab);
		} break;
	case For:
	case Glo:
		break;
	case Fun:
	case Prd:
		{funprd *vv= (funprd *)Ats(v);
			if (vv->def == Use) {
				free((ptr) vv->fux);
				release(vv->reftab);
			}
		} break;
	case Ref:
		break;
	default:
		syserr("release called with unknown type");
	}
deNote((ptr) v);
	free((ptr) v); v->type= '\0';
	/* free is also called from popnd and pushdyator (in bexp.c) */
}

uniql(ll) value *ll; {
	if (*ll != Vnil && (*ll)->refcnt > 1) {
		value c= ccopy(*ll);
		release(*ll);
		*ll= c;
	}
}

xtndtex(a, d) value *a; intlet d; {
	intlet len= (*a)->len+d;
	Regrabber();
	regetmem(a, Adj((len+1)*sizeof(char)), "xtndtex");
	(*a)->len= len;
}

xtndlt(a, d) value *a; intlet d; {
	intlet len= (*a)->len; intlet l1= Len, l2;
	len+= d; l2= Len;
	if (l1 != l2) {
		Regrabber();
		regetmem(a, Adj(l2*sizeof(value)), "xtndlt");
	}
	(*a)->len= len;
}

value notel; bool noting=Yes;

#define I(p) mk_integer((int)(p))

note(p) {if(!noting)
	{value ip; noting= Yes; insertl(ip= I(p), &notel); release(ip);
	noting= No;}
}
denote(p) {if(!noting)
	{value ip; noting= Yes;
	if(!in(ip= I(p),notel))syserr("releasing illegally");
	removel(ip, &notel); release(ip); noting= No;}
}

initmem() {
	etxt= grab(Tex, 0);
	elis= grab(Lis, 0);
	etab= grab(Tab, 0);
	elt=  grab(ELT, 0);
notel= grab_lis(0); noting= No;
}
