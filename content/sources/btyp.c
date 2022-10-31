/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* Type matching */
#include "b.h"
#include "bobj.h"
#include "bsem.h"
#include "btyp.h"

#define Tnil ((btype) Vnil)

Forward btype valtype();

Visible btype loctype(l) loc l; {
	value *ll;
	if (Is_simploc(l)) {
		simploc *sl= Simploc(l);
		if (!in_env(sl->e->tab, sl->i, &ll)) return Tnil;
		return valtype(*ll);
	} else if (Is_tbseloc(l)) {
		tbseloc *tl= Tbseloc(l);
		btype tt= loctype(tl->R), assoc;
		if (tt == Tnil) return Tnil;
		if (Length(tt) == 0) {
			release(tt);
			return Tnil;
		}
		assoc= thof(1, tt);
		release(tt);
		return assoc;
	} else if (Is_trimloc(l)) {
		return mk_text("");
	} else if (Is_compound(l)) {
		btype ct= mk_compound(Length(l)); intlet k;
		Over (l) put_in_field(loctype(Field(l, k)), &ct, k);
		return ct;
	} else {
		syserr("loctype asked of non-target");
		return Tnil;
	}
}

Visible btype valtype(v) value v; {
	intlet len= Length(v);
	if (Is_number(v)) return mk_integer(0);
	else if (Is_text(v)) return mk_text("");
	else if (Is_compound(v)) {
		btype ct= mk_compound(len); intlet k;
		Overall put_in_field(valtype(Field(v, k)), &ct, k);
		return ct;
	} else if (Is_ELT(v)) {
		return mk_elt();
	} else if (Is_list(v)) {
		btype tt= mk_elist(), vt, ve;
		if (len > 0) {
			insertl(vt= valtype(ve= min1(v)), &tt);
			release(vt); release(ve);
		}
		return tt;
	} else if (Is_table(v)) {
		btype tt= mk_etable(), vk, va;
		if (len > 0) {
			vk= valtype(Key(v, 0));
			va= valtype(Assoc(v, 0));
			replace(&tt, vk, va, Val);
			release(vk); release(va);
		}
		return tt;
	} else {
		syserr("valtype called with unknown type");
		return Tnil;
	}
}

Visible must_agree(t, u, m) btype t, u; string m; {
	intlet tlen, ulen, k;
	value vt, vu;
	if (t == Tnil || u == Tnil || t == u) return;
	tlen= Length(t); ulen= Length(u);
	if ((Is_number(t) && Is_number(u))
	 || (Is_text(t) && Is_text(u))
	 || (Is_ELT(u) && (Is_ELT(t) || Is_list(t) || Is_table(t)))
	 || (Is_ELT(t) && (Is_ELT(u) || Is_list(u) || Is_table(u)))) return;
	else if (Is_compound(t) && Is_compound(u)) {
		if (tlen != ulen) error(m);
		else Over(t) must_agree(Field(t, k), Field(u, k), m);
	} else if (Is_list(t) && Is_list(u)) {
		if (tlen>0 && ulen>0) {
			must_agree(vt= min1(t), vu= min1(u), m);
			release(vt); release(vu);
		}
	} else if (Is_table(t) && Is_table(u)) {
		if (tlen>0 && ulen>0) {
			must_agree(Key(t, 0), Key(u, 0), m);
			must_agree(Assoc(t, 0), Assoc(u, 0), m);
		}
	} else error(m);
}
