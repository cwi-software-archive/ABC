/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* B locations and environments */
#include "b.h"
#include "bobj.h"
#include "bsem.h"

Hidden value* location();
Hidden Procedure te_delete();
Hidden Procedure uniquify();

Hidden value* location(l) loc l; {
	value *ll;
	if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
		if (!in_env(sl->e->tab, sl->i, &ll)) error("target still empty");
		return ll;
	} else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);
		ll= adrassoc(*location(tl->R), tl->K, Val);
		if (ll == Pnil) error("key not in table");
		return ll;
	} else {
		syserr("call of location with improper type");
		return (value *) Dummy;
	}
}

Hidden Procedure uniquify(l) loc l; {
	if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
		value *ta= &(sl->e->tab), key= sl->i;
		uniql(ta);
		check_location(l);
		uniq_assoc(*ta, key);
	} else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);
		value t, key;
		uniquify(tl->R);
		t= *location(tl->R); key= tl->K;
		if (!Is_table(t)) error("selection on non-table");
		if (Length(t) == 0) error("selection on empty table");
		check_location(l);
		uniq_assoc(t, key);
	} else if (Is_trimloc(l)) { syserr("uniquifying trimloc");
	} else if (Is_compound(l)) { syserr("uniquifying comploc");
	} else syserr("uniquifying non-location");
}

Visible Procedure check_location(l) loc l; {
	value *ll= location(l);
	/* location may produce an error message */
}

Visible value content(l) loc l; {
	return copy(*location(l));
}

Visible loc trim_loc(R, B, C) loc R; intlet B, C; {
	if (Is_trimloc(R)) {
		trimloc *rr= Trimloc(R);
		return mk_trimloc(rr->R, B, C);
	} else if (Is_simploc(R) || Is_tbseloc(R)) {
		return mk_trimloc(R, B, C);
	} else {
		error("trim (@ or |) on target of improper type");
		/* NOTREACHED */
	}
}

Visible loc tbsel_loc(R, K) loc R; value K; {
	if (Is_simploc(R) || Is_tbseloc(R)) return mk_tbseloc(R, K);
	else error("selection on target of improper type");
	/* NOTREACHED */
}

Visible loc local_loc(i) basidf i; { return mk_simploc(i, curnv); }

Visible loc global_loc(i) basidf i; { return mk_simploc(i, prmnv); }

Visible Procedure put(v, l) value v; loc l; {
	if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
		replace(&(sl->e->tab), sl->i, v, Env);
	} else if (Is_trimloc(l)) {
		trimloc *tl= Trimloc(l);
		value rr, nn, head, tail, part; intlet B= tl->B, C= tl->C;
		rr= *location(tl->R);
		if (!Is_text(rr)) error("trim target contains no text");
		if (!Is_text(v))
		    error("putting non-text in trim(@ or|) on text location");
		if (B < 0 || C < 0 || B+C > Length(rr))
		    error("trim (@ or |) on text location out of bounds");
		head= trim(rr, 0, Length(rr)-B);
		tail= trim(rr, Length(rr)-C, 0);
		part= concat(head, v);
		nn= concat(part, tail);
		put(nn, tl->R);
		release(nn); release(head); release(tail); release(part);
	} else if (Is_compound(l)) {
		intlet k;
		if (!Is_compound(v))
		    error("putting non-compound in compound location");
		if (Length(v) != Length(l))
		    error("putting compound in compound location of different length");
		Over (l) put(Field(v, k), Field(l, k));
	} else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);
		uniquify(tl->R);
		replace(location(tl->R), tl->K, v, Val);
	} else error("putting in non-target");
}

Visible Procedure delete(l) loc l; {
	/* It should be checked here that they all exist */
	if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
		uniql(&(sl->e->tab)); /*no need: see te_delete*/
		tte_delete(sl->i, &(sl->e->tab), Env);
	} else if (Is_trimloc(l)) {
		error("deleting trimmed (@ or |) target");
	} else if (Is_compound(l)) {
		intlet k;
		Over (l) delete(Field(l, k));
	} else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);
		te_delete(tl->K, tl->R);
	} else error("deleting non-target");
}

Visible Procedure insert(v, l) value v; loc l; {
	value *ll;
	uniquify(l);
	ll= location(l);
	if (Is_ELT(*ll)) {
		release(*ll);
		*ll= mk_elist();
	}
	insertl(v, ll);
}

Visible Procedure remove(v, l) value v; loc l; {
	uniquify(l);
	removel(v, location(l));
}

Hidden Procedure te_delete(key, ta) value key; loc ta; {
	uniquify(ta);
	tte_delete(key, location(ta), Val);
}

Visible Procedure bind(l) loc l; {
	if (Is_simploc(l)) {
		simploc *ll= Simploc(l);
		if (!in(ll->i, *bndtgs)) /* kludge */
			insertl(ll->i, bndtgs);
	} else if (Is_compound(l)) {
		intlet k;
		Over (l) bind(Field(l, k));
	} else if (Is_trimloc(l)) {
		pprerr("t@p or t|p not allowed in ranger", "");
	} else if (Is_tbseloc(l)) {
		pprerr("t[e] not allowed in ranger", "");
	} else error("binding non-identifier");
}
