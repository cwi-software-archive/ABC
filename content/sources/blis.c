/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* B lists */
#include "b.h"
#include "bobj.h"
#include "btlt.h"
#include "bcon.h"

Visible value mk_elist() { return grab_lis(0); }

Visible value list_elem(l, i) value l; intlet i; {
	return List_elem(l, i);
}

Visible insertl(v, ll) value v, *ll; {
	intlet len= Length(*ll); register value *lp, *lq;
	intlet k; register intlet kk;
	if ((*ll)->type != Lis) error("inserting in non-list");
	VOID found(list_elem, *ll, v, &k);
	if (Unique(*ll)) {
		xtndlt(ll, 1);
		lq= Ats(*ll)+len; lp= lq-1;
		for (kk= len; kk > k; kk--) *lq--= *lp--;
		*lq= copy(v);
	} else {
		lp= Ats(*ll);
		release(*ll);
		*ll= grab_lis(++len);
		lq= Ats(*ll);
		for (kk= 0; kk < len; kk++) *lq++= copy (kk == k ? v : *lp++);
	}
}

Visible removel(v, ll) value v; value *ll; {
	register value *lp, *lq;
	intlet len; intlet k;
	if ((*ll)->type != Lis && (*ll)->type != ELT)
		error("removing from non-list");
	lp= Ats(*ll); len= Length(*ll);
	if (len == 0) error("removing from empty list");
	if (!found(list_elem, *ll, v, &k))
		error("removing non-existing list entry");
	/* lp[k] = v */
	if (Unique(*ll)) {
		release(*(lp+=k));
		for (k= k; k < len; k++) {*lp= *(lp+1); lp++;}
		xtndlt(ll, -1);
	} else {
		intlet kk= k;
		lq= Ats(*ll);
		release(*ll);
		*ll= grab_lis(--len);
		lp= Ats(*ll);
		Overall {
			*lp++= copy (*lq++);
			if (k == kk) lq++;
		}
	}
}

Visible value mk_numrange(a, z) value a, z; {
	value e= mk_elist(), m= copy(a), n, one= mk_integer(1);

	while (compare(m, z)<=0) {
		insertl(m, &e);
		m= sum(n=m, one);
		release(n);
	}
	release(m); release(one);
	return e;
}

Visible value mk_charrange(av, zv) value av, zv; {
	char a= charval(av), z= charval(zv);
	value e= grab_lis((intlet) (z-a+1)); register value *ep= Ats(e);
	char m[2];
	m[1]= '\0';
	for (m[0]= a; m[0] <= z; m[0]++) {
		*ep++= mk_text(m);
	}
	return e;
}
