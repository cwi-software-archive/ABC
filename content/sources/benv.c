/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* Environments */
#include "b.h"
#include "bobj.h"

envtab prmnvtab;
Hidden envchain prmnvchain;
env prmnv;

/* context: */
env curnv; value *bndtgs; value bndtglist;
literal cntxt, resexp; value uname;
intlet cur_ilev, lino; txptr tx, ceol;

context read_context;
context how_context;

bool xeq= Yes;

Visible Procedure sv_context(sc) context *sc; {
	sc->curnv= curnv;
	sc->bndtgs= bndtgs;
	sc->cntxt= cntxt;
	sc->resexp= resexp;
	sc->uname= uname;
	sc->cur_ilev= cur_ilev;
	sc->lino= lino;
	sc->tx= tx;
	sc->ceol= ceol;
}

Visible Procedure set_context(sc) context *sc; {
	curnv= sc->curnv;
	bndtgs= sc->bndtgs;
	cntxt= sc->cntxt;
	resexp= sc->resexp;
	uname= sc->uname;
	cur_ilev= sc->cur_ilev;
	lino= sc->lino;
	tx= sc->tx;
	ceol= sc->ceol;
}

Visible Procedure initenv() {
	/* The following invariant must be maintained:
	   EITHER:
	      prmnvtab == Vnil AND the original permanent-environment
		  table resides in prmnv->tab
	   OR:
	      prmnvtab != Vnil AND the original permanent-environment
		  table resides in prmnvtab (and prmnv->tab contains
		  a scratch-pad copy)
	*/
	prmnv= &prmnvchain;
	prmnv->tab= mk_etable(); prmnvtab= Vnil;
	prmnv->inv_env= Enil;
	bndtglist= mk_elist();
}

Visible Procedure re_env() {
	setprmnv(); bndtgs= &bndtglist;
}

Visible Procedure setprmnv() {
	/* the current and permanent environment are reset
	   to the original permanent environment */
	if (prmnvtab != Vnil) {
		prmnv->tab= prmnvtab;
		prmnvtab= Vnil;
	}
	curnv= prmnv;
}

Visible Procedure extbnd_tags(btl, e, et) value btl; envtab *e, et; {
	intlet blen= Length(btl), bk;
	value *aa, vb;
	for (bk= 1; bk <= blen; bk++) {
		vb= thof(bk, btl);
		if (in_env(et, vb, &aa)) replace(e, vb, *aa, Env);
		release(vb);
	}
}

Visible Procedure restore_env(e0) env e0; {
/* syserr("restore_env not yet implemented") */ ;
}

Visible value* lookup(t) value t; {
	return envassoc(curnv->tab, t);
}

