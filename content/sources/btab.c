/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* B tables */
#include "b.h"
#include "bobj.h"
#include "btlt.h"

Visible value mk_etable() { return grab_tab(0); }

Visible value keys(ta) value ta; {
	intlet k; value li= grab_lis(Length(ta)), *le, *te= (value *)Ats(ta);
	if(ta->type != Tab && ta->type != ELT)
		error("in keys t, t is not a table");
	le= (value *)Ats(li);
	Over (ta) *le++= copy(Cts(*te++));
	return li;
}

Hidden value key_elem(t, i) value t; intlet i; { /*The key of the i-th entry*/
	return Key(t, i);
}

/* adrassoc returns a pointer to the associate, rather than
   the associate itself, so that the caller can decide if a copy
   should be taken or not. If the key is not found, Pnil is returned. */
Visible value* adrassoc(t, key, valorenv) value t, key; literal valorenv; {
	intlet where;
	if (t->type != Tab && t->type != ELT) {
		if (valorenv == Val) error("selection on non-table");
		else syserr("selection on non-environment");
	}
	return found(key_elem, t, key, &where) ? &Assoc(t, where) : Pnil;
}

Visible value* envassoc(t, key) value t, key; {
	return adrassoc(t, key, Env);
}

Visible bool in_env(tab, key, aa) value tab, key, **aa; {
	*aa= envassoc(tab, key);
	return (*aa != Pnil);
}

Visible Procedure uniq_assoc(ta, key) value ta, key; {
	intlet k;
	if (found(key_elem, ta, key, &k)) {
		uniql(Ats(ta)+k);
		uniql(&Assoc(ta,k));
	} else syserr("uniq_te called for non-existent table entry");
}

Visible Procedure replace(ta, key, v, valorenv) value *ta, key, v; literal valorenv; {
	intlet len= Length(*ta); value *tp, *tq;
	intlet k, kk;
	uniql(ta);
	if ((*ta)->type == ELT) (*ta)->type = Tab;
	else if ((*ta)->type != Tab) {
		if (valorenv == Val) error("replacing in non-table");
		else syserr("replacing in non-environment");
	}
	if (found(key_elem, *ta, key, &k)) {
		value *a;
		uniql(Ats(*ta)+k);
		a= &Assoc(*ta, k);
		uniql(a);
		release(*a);
		*a= copy(v);
		return;
	} else {
		xtndlt(ta, 1);
		tq= Ats(*ta)+len; tp= tq-1;
		for (kk= len; kk > k; kk--) *tq--= *tp--;
		*tq= grab_com(2);
		Cts(*tq)= copy(key);
		Dts(*tq)= copy(v);
	}
}

Visible Procedure tte_delete(key, tl, valorenv) value key; value *tl; literal valorenv; {
	intlet len, k; value *tp;
	if ((*tl)->type == ELT) {
		if (valorenv == Val)
			error("deleting table entry from empty table");
		else error("deleting from empty environment");
	}
	if ((*tl)->type != Tab) {
		if (valorenv == Val)
			error("deleting table entry from non-table");
		else syserr("deleting from non-environment");
	}
	tp= Ats(*tl); len= Length(*tl);
	if (!found(key_elem, *tl, key, &k)) error(valorenv == Env?
			"deleting non-existent target":
			"deleting non-existent table entry");
	if (Unique(*tl)) {
		release(*(tp+=k));
		for (k= k; k < len; k++) {*tp= *(tp+1); tp++;}
		xtndlt(tl, -1);
	} else {
		intlet kk; value *tq= Ats(*tl);
		release(*tl);
		*tl= grab_tab(--len);
		tp= Ats(*tl);
		for (kk= 0; kk < len; kk++) {
			*tp++= copy (*tq++);
			if (kk == k) tq++;
		}
	}
}

