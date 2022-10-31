/*
   This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
   Permission is granted to use and copy this software, but not for profit,
   and provided that these same conditions are imposed on any person
   receiving or using the software.
*/
/* B commands */
#include "b.h"
#include "bcon.h"
#include "bobj.h"
#include "bscr.h"
#include "berr.h"
#include "bkey.h"
#include "bsyn.h"
#include "bsem.h"
#include "btyp.h"

#define Nex if (!xeq) {tx= ceol; return Yes;}

Hidden char rdbuf[RDBUFSIZE];
Hidden txptr rdbufend= &rdbuf[RDBUFSIZE];

#define USE_QUIT "\r*** use QUIT or interrupt to abort READ command\n"

Hidden Procedure read_line();
Hidden bool sim_com();
Hidden bool con_com();
Hidden bool term_com();
Hidden bool secret_com();
Hidden Procedure chckvtc();
Hidden bool expr_s();
Hidden Procedure alt_suite();

Hidden Procedure read_line(l, t, eg) loc l; btype t; bool eg; {
	context c; txptr tx0= tx, rp; intlet k; value r; btype rt;
	envtab svprmnvtab= Vnil; bool must_sv= eg, got;
	sv_context(&c);
	if (active_reads >= MAX_NMB_ACT_READS)
		error("too many READs simultaneously active");
	if (setjmp(reading[active_reads++]) != 0) /* long jump occurred */
		set_context(&c);
	if (cntxt != In_read) sv_context(&read_context);
	if (must_sv) svprmnvtab= prmnvtab == Vnil ? Vnil : prmnv->tab;
	/* save scratch-pad copy because of following setprmnv() */
	if (eg) setprmnv(); must_sv= No;
	cntxt= In_read;
	got= No;
	while (!got) {
		tx= rp= rdbuf;
		if (read_interactive) {
			/* if (outeractive) { line(); nwl= No; } */
			/* Experimental interactive prompt */
			fprintf(stderr, "?\b");
		}
		got= Yes;
		while ((k= getchar()) != EOF && k != '\n') {
			*rp++= k;
			if (rp > rdbufend-1) syserr("read buffer overflow");
		}
		if (k == EOF) {
			if (read_interactive) {
				fprintf(stderr, USE_QUIT);
				if (outeractive) nwl= Yes;
				got= No;
			} else error("End of file encountered during READ command");
		}
	}
	if (read_interactive && outeractive && k == '\n') nwl= Yes;
	*rp= '\n';
	Skipsp(tx);
	if (atkw(QUIT)) int_signal(Yes);
	if (eg) {
		r= expr(rp); rt= valtype(r);
		if (svprmnvtab != Vnil) {
			prmnvtab= prmnv->tab;
			prmnv->tab= svprmnvtab;
		}
		must_sv= Yes;
		set_context(&c);
		must_agree(t, rt,
		    "type of expression does not agree with that of EG sample");
		release(rt);
	} else {
		*rp= '\0';
		r= mk_text(rdbuf);
		set_context(&c);
	}
	put(r, l);
	active_reads--;
	release(r);
	tx= tx0;
}

Hidden bool sim_com() {
	txptr ftx, ttx;
	switch (Char(tx)) {
case 'C': if (atkw(CHECK)) {
		env e0= curnv; outcome o;
		Nex;
		o= test(ceol);
		if (xeq) {
			if (o == Fail) checkerr();
			restore_env(e0);
		}
		return Yes;
	} else if (atkw(CHOOSE)) {
		loc l; value v; intlet r;
		reqkw(FROM_choose, &ftx, &ttx);
		Nex;
		l= targ(ftx);
		tx= ttx; v= expr(ceol);
		if (xeq) {
			value w;
			if (!is_tlt(v))
			    error("choosing from non-text, -list or -table");
			if (Length(v) == 0)
			    error("choosing from empty text, list or table");
			r= 1+floor(Length(v)*random());
			put(w= thof(r, v), l); release(w);
		}
		release(v); release(l);
		return Yes;
	  }
	  return No;
case 'D': if (atkw(DELETE)) {
		loc l;
		Nex;
		l= targ(ceol);
		if (xeq) delete(l);
		release(l);
		return Yes;
	} else if (atkw(DRAW)) {
		value r; loc l;
		Nex;
		l= targ(ceol);
		if (xeq) {
			r= mk_approx(random());
			put(r, l);
			release(r);
		}
		release(l);
		return Yes;
	  }
	  return No;
case 'E': if (atkw(ELSE)) {
	       pprerr("ELSE only allowed as alternative test after SELECT", "");
	  }
	  return No;
case 'I': if (atkw(INSERT)) {
		value v; loc l;
		reqkw(IN_insert, &ftx, &ttx);
		Nex;
		v= expr(ftx);
		tx= ttx; l= targ(ceol);
		if (xeq) insert(v, l);
		release(v); release(l);
		return Yes;
	  }
	  return No;
case 'P': if (atkw(PUT)) {
		value v; loc l;
		reqkw(IN_put, &ftx, &ttx);
		Nex;
		v= expr(ftx);
		tx= ttx; l= targ(ceol);
		if (xeq) put(v, l);
		release(v); release(l);
		return Yes;
	  }
	  return No;
case 'R': if (atkw(READ)) {
		value v; loc l; btype vt, lt; bool eg= Yes;
		if (find(RAW, ceol, &ftx, &ttx)) {
			eg= No;
			vt= mk_text("");
		} else reqkw(EG, &ftx, &ttx);
		Nex;
		l= targ(ftx); lt= loctype(l);
		tx= ttx;
		if (eg) {
			v= expr(ceol);
			vt= valtype(v); release(v);
		}
		must_agree(vt, lt,
		    eg ? "this sample could not lawfully be put in the target"
		       : "in READ x RAW, x must be a simple textual target");
		release(lt);
		if (xeq) read_line(l, vt, eg);
		release(l); release(vt);
		return Yes;
	} else if (atkw(REMOVE)) {
		value v; loc l;
		reqkw(FROM_remove, &ftx, &ttx);
		Nex;
		v= expr(ftx);
		tx= ttx; l= targ(ceol);
		if (xeq) remove(v, l);
		release(v); release(l);
		return Yes;
	  }
	  return No;
case 'S': if (atkw(SET_RANDOM)) {
		value v;
		Nex;
		v= expr(ceol);
		if (xeq) setrandom((double) hash(v));
		release(v);
		return Yes;
	  } else if (atkw(SHARE)) pprerr(
	  "SHARE only allowed following HOW'TO-, YIELD- or TEST-heading", "");
	  return No;
case 'W': if (atkw(WRITE)) {
		txptr tx0; value v; intlet nwlc;
		Nex;
		Skipsp(tx);
		while (Char(tx) == '/' && (Char(tx+1) == '/')) {
			if (xeq) newline();
			tx++;
		}
		tx0= tx;
	loop:	if (Char(tx++) != '/') {tx= tx0; goto postnl;}
		if (Char(tx++) == '*') goto loop;
		if (xeq) newline();
		tx= tx0+1;
	postnl: ftx= ceol;
		while (Space(Char(ftx-1))) ftx--;
		nwlc= 0;
		while (ftx > tx && Char(ftx-1) == '/') {
			nwlc++;
			ftx--;
		}
		if (ftx > tx) {
			v= expr(ftx);
			if (xeq) writ(v);
			release(v);
		}
		while (nwlc-- > 0) {
			if (xeq) newline();
		}
		return Yes;
	  }
	  return No;
default:  return No;
	}
}

#define Reqcol {req(":", ceol, &utx, &vtx); \
		if (!xeq) {tx= vtx; comm_suite(); return Yes;}}
#define Resetx(tx0) {tx= (tx0); lino= lino0; cur_ilev= cil;}

Hidden bool con_com() {
	intlet lino0= lino, cil= cur_ilev;
	txptr ftx, ttx, utx, vtx;
	switch (Char(tx)) {
case 'I': if (atkw(IF)) {
		env e0= curnv; bool xeq0= xeq;
		outcome o;
		Reqcol;
		o= test(utx);
		xeq= o == Succ;
		tx= vtx; comm_suite();
		xeq= xeq0; restore_env(e0); 
		return Yes;
	  }
	  return No;
case 'S': if (atkw(SELECT)) {
		need(":");
		upto(ceol, "SELECT:");
		alt_suite();
		return Yes;
	  }
	  return No;
case 'W': if (atkw(WHILE)) {
		env e0= curnv; bool xeq0= xeq; txptr tx0= tx;
		outcome o;
		Reqcol;
	loop:	o= test(utx);
		if (xeq0) xeq= o == Succ;
		tx= vtx; comm_suite();
		xeq= xeq0; restore_env(e0);
		if (xeq && o == Succ && !terminated) {
			Resetx(tx0); goto loop;
		}
		return Yes;
	  }
	  return No;
case 'F': if (atkw(FOR)) {
		env e0= curnv; bool xeq0= xeq; loc l; value v, w;
		Reqcol;
		if (find(PARSING, utx, &ftx, &ttx)) {
			tx= ttx; pprerr("PARSING not allowed in FOR ...", "");
		}
		reqkw(IN_for, &ftx, &ttx);
		if (ttx > ceol) {
			tx= ceol;
			parerr("IN after colon", "");
		}
		l= targ(ftx);
		if (!Is_simploc(l) && !Is_compound(l)) /*to bloc.c?*/
			pprerr("inappropriate identifier after FOR", "");
		bind(l);
		tx= ttx; v= expr(utx);
		{intlet k, len= xeq ? Length(v) : 1;
			if (len == 0) {
				xeq= No; len= 1;
			}
			for (k= 1; k <= len && !terminated; k++) {
				Resetx(utx);
				if (xeq) {
					w= thof(k, v);
					put(w, l);
					release(w);
				}
				tx= vtx; comm_suite();
			}
		}
		xeq= xeq0; restore_env(e0);
		release(v); release(l);
		return Yes;
	  }
	  return No;
default:  return No;
	}
}

Hidden bool term_com() {
	switch (Char(tx)) {
case 'F': if (atkw(FAIL)) {
		upto(ceol, "FAIL");
		if (xeq) {
			chckvtc(Rep);
			resout= Fail;
			terminated= Yes;
		} else tx= ceol;
		return Yes;
	  }
	  return No;
case 'Q': if (atkw(QUIT)) {
		upto(ceol, "QUIT");
		if (xeq) {
			if (cur_ilev == 0) bye(0);
			chckvtc(Voi);
			terminated= Yes;
		}
		return Yes;
	  }
	  return No;
case 'R': if (atkw(RETURN)) {
		if (xeq) {
			chckvtc(Ret);
			resval= expr(ceol);
			terminated= Yes;
		} else tx= ceol;
		return Yes;
	  } else if (atkw(REPORT)) {
		if (xeq) {
			chckvtc(Rep);
			resout= test(ceol);
			terminated= Yes;
		} else tx= ceol;
		return Yes;
	  }
	  return No;
case 'S': if (atkw(SUCCEED)) {
		upto(ceol, "SUCCEED");
		if (xeq) {
			chckvtc(Rep);
			resout= Succ;
			terminated= Yes;
		} else tx= ceol;
		return Yes;
	  }
	  return No;
default:  return No;
	}
}

Hidden bool secret_com() {
	switch (Char(tx)) {
case 'D': if (atkw("DEBUG")) {
		Nex;
		bugs= Yes;
		return Yes;
	  }
	  return No;
case 'G': if (atkw("GR")) {
		Nex;
		prgr();
		return Yes;
	  }
	  return No;
case 'N': if (atkw("NO'DEBUG")) {
		Nex;
		bugs= No;
		return Yes;
	  } else if (atkw("NO'TRACE")) {
		Nex;
		tracing= No;
		return Yes;
	  }
	  return No;
case 'T': if (atkw("TRACE")) {
		Nex;
		tracing= Yes;
		return Yes;
	  }
	  return No;
default:  return No;
	}
}

Hidden Procedure chckvtc(re) literal re; {
	if ((cntxt != In_unit && cntxt != In_ref) || resexp == Voi) {
		if (re == Ret)
			pprerr(
	"RETURN e only allowed inside YIELD-unit or\n\
    expression-refinement", "");
		else if (re == Rep)
			pprerr(
	"REPORT t only allowed inside TEST-unit or test-refinement", "");
	}
	if (re != resexp) {
		if (resexp == Ret)
			pprerr(
	"RETURN e must terminate YIELD-unit or expression-refinement", "");
		if (resexp == Rep)
			pprerr(
	"REPORT t must terminate TEST-unit or test-refinement", "");
	}
}

Hidden bool expr_s() {
	char c;
	Skipsp(tx);
	if (tx >= ceol) return No;
	c= Char(tx);
	return Letter(c) || Montormark(c) || Dig(c) || c == '.' || c == 'E' ||
		c == '(' || c == '{' || c == '\'' || c == '"';
}

Hidden intlet comcnt= 0;

Visible Procedure command() {
	if (++comcnt > 10000) {
		putprmnv();
		comcnt= 1;
	}
	if (Char(tx) == Eotc) getline();
debug("analyzing command");
	if (tracing) trace();
	if (Ceol(tx));
	else if (sim_com() || con_com() ||
		unit() || term_com() || ref_com() || udc() ||
		secret_com()) skipping= No;
	else if (Char(tx) == ':' || Char(tx) == '=' || Char(tx) == '!') {
		if (!interactive) parerr("special commands only interactively", "");
		if (!(cntxt == In_command && cur_ilev == 0)) parerr(
			"special commands only on outermost level (no indentation)", "");
		special();
	} else if (cntxt == In_command && cur_ilev == 0 && expr_s()) {
		value w= expr(ceol);
		writ(w);
		release(w);
	} else {txptr tx0= tx; value uc= keyword(ceol);
		tx= tx0; parerr("you have not told me HOW'TO ", Str(uc));
	}
	To_eol(tx);
debug("command treated");
}

Visible Procedure comm_suite() {
	intlet cil= cur_ilev;
	if (ateol()) {
		txptr tx0= tx; bool xeq0= xeq;
		if (Char(tx+1) == Eotc) xeq= No;
		while (ilev(No) > cil) {
			findceol();
			command();
			if (terminated) return;
			if (cur_ilev <= cil) goto brk1;
		}
		veli();
	brk1:	if (xeq0 && !xeq) {
			tx= tx0; xeq= Yes;
			cur_ilev= cil;
			while (ilev(No) > cil) {
				findceol();
				command();
				if (terminated) return;
				if (cur_ilev <= cil) goto brk2;
			}
			veli();
		brk2:	;
		}
	} else command();
}

Hidden Procedure alt_suite() {
	intlet cil= cur_ilev; env e0= curnv; txptr utx, vtx;
	bool xeq0= xeq, succ= !xeq, Else= No;
	if (!ateol()) syserr("alt_suite not at end of line");
	while (ilev(No) > cil) {
		findceol();
		if (Else)
		parerr("after ELSE: ... no more alternatives are allowed", "");
		req(":", ceol, &utx, &vtx);
		if (atkw(ELSE)) {
			succ= Else= Yes;
			upto(utx, "ELSE");
			tx= vtx; comm_suite();
			if (terminated) return;
		} else {
			if (xeq) succ= test(utx) == Succ;
			xeq= xeq && succ;
			tx= vtx; comm_suite();
			if (terminated) return;
			xeq= !succ;
		}
		if (cur_ilev <= cil) goto brk;
	}
	veli();
brk:	if (!succ) error("none of the alternative tests of SELECT succeeds");
	xeq= xeq0; if (xeq) restore_env(e0);
}

Visible Procedure initcom() {
	setrandom((double) SEED);
}
