/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* General operations on objects */
/* plus Hows Funs and other odd types that don't fit anywhere else */

#include "b.h"
#include "bobj.h"
#include "btlt.h"

#define Sgn(d) (d)

Visible relation compare(v, w) value v, w; {
	literal vt= v->type, wt= w->type;
	register intlet vlen= Length(v), wlen= Length(w), len, k;
	value message;
	if (v == w) return 0;
	if (!(vt == wt && !(vt == Com && vlen != wlen) ||
			    vt == ELT && (wt == Lis || wt == Tab) ||
			    wt == ELT && (vt == Lis || vt == Tab))) {
		message= concat(mk_text("incompatible types: "),
			 concat(convert((value) valtype(v), No, No),
			 concat(mk_text(" and "),
			 convert((value) valtype(w), No, No))));
		error(Str(message)); /*doesn't return: so can't release message*/
	}
	if (vt != Num && (vlen == 0 || wlen == 0))
		return Sgn(vlen-wlen);
	switch (vt) {
	case Num: return numcomp(v, w);
	case Tex: return strcmp(Str(v), Str(w));

	case Com:
	case Lis:
	case Tab:
	case ELT:
		{value *vp= Ats(v), *wp= Ats(w);
		 relation c;
			len= vlen < wlen ? vlen : wlen;
			Overall if ((c= compare(*vp++, *wp++)) != 0) return c;
			return Sgn(vlen-wlen);
		}
	default:
		syserr("comparison of unknown types");
		return (intlet) Dummy;
	}
}

Visible double hash(v) value v; {
	literal t= v->type; intlet len= Length(v), k; double d= t+.404*len;
	switch (t) {
	case Num: return numhash(v);
	case Tex:
		{string vp= Str(v);
			Overall d= .987*d+.277*(*vp++);
			return d;
		}
	case Com:
	case Lis:
	case Tab:
	case ELT:
		{value *vp= Ats(v);
			if (len == 0) return .909;
			Overall d= .874*d+.310*hash(*vp++);
			return d;
		}
	default:
		syserr("hash called with unknown type");
		return (double) Dummy;
	}
}

Visible bool found(elem, v, probe, where)
	value (*elem)(), v, probe; intlet *where;
	/* think of elem(v,lo-1) as -Infinity and elem(v,hi+1) as +Infinity.
	   found and where at the end satisfy:
	   SELECT:
	       SOME k IN {lo..hi} HAS probe = elem(v,k):
	           found = Yes AND where = k
	       ELSE: found = No AND elem(v,where-1) < probe < elem(v,where).
	*/
{relation c; intlet lo=0, hi= Length(v)-1;
	if (lo > hi) { *where= lo; return No; }
	if ((c= compare(probe, (*elem)(v, lo))) == 0) {*where= lo; return Yes; }
	if (c < 0) { *where=lo; return No; }
	if (lo == hi) { *where=hi+1; return No; }
	if ((c= compare(probe, (*elem)(v, hi))) == 0) { *where=hi; return Yes; }
	if (c > 0) { *where=hi+1; return No; }
	/* elem(lo) < probe < elem(hi) */
	while (hi-lo > 1) {
		if ((c= compare(probe, (*elem)(v, (lo+hi)/2))) == 0) {
			*where= (lo+hi)/2; return Yes;
		}
		if (c < 0) hi= (lo+hi)/2; else lo= (lo+hi)/2;
	}
	*where= hi; return No;
}

Visible loc mk_simploc(i, e) basidf i; env e; {
	loc l= grab_sim();
	Cts(l)= copy(i); Dts(l)= (value) e;
	return l;
}

Visible loc mk_trimloc(R, B, C) loc R; intlet B, C; {
	loc l= grab_tri(); trimloc *ll= (trimloc *)Ats(l);
	ll->R= copy(R); ll->B= B; ll->C= C;
	return l;
}

Visible loc mk_tbseloc(R, K) loc R; value K; {
	loc l= grab_tse(); tbseloc *ll= (tbseloc *)Ats(l);
	ll->R= copy(R); ll->K= copy(K);
	return l;
}

Visible fun mk_fun(L, H, adic, def, fux, lux, reftab, filed)
 intlet L, H; literal adic, def; txptr fux, lux; value reftab; bool filed; {
	fun f= grab_fun(); funprd *ff= (funprd *)Ats(f);
	ff->L= L; ff->H= H; ff->adic= adic; ff->def= def; ff->fux= fux;
	ff->lux= lux; ff->reftab= reftab; ff->filed= filed;
	return f;
}

Visible prd mk_prd(adic, def, fux, lux, reftab, filed)
 literal adic, def; txptr fux, lux; value reftab; bool filed; {
	prd p= grab_prd(); funprd *pp= (funprd *)Ats(p);
	pp->adic= adic; pp->def= def; pp->fux= fux;
	pp->lux= lux; pp->reftab= reftab; pp->filed= filed;
	return p;
}

Visible value mk_how(fux, lux, reftab, filed)
 txptr fux, lux; value reftab; bool filed; {
	value h= grab_how(); how *hh= (how *)Ats(h);
	hh->fux= fux; hh->lux= lux; hh->reftab= reftab; hh->filed= filed;
	return h;
}

Visible value mk_ref(rp, rlino) txptr rp; intlet rlino; {
	value r= grab_ref();
	((ref *)Ats(r))->rp= rp;
	((ref *)Ats(r))->rlino= rlino;
	return r;
}

Visible value mk_per(v) value v; {
	value p= grab_per();
	*Ats(p)= copy(v);
	return p;
}
