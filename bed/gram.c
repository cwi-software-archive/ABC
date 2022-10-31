/* SccsId: @(#)gram.c	1.9 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- All routines referencing the grammar table are in this file.
 */

#include "b.h"
#include "bobj.h"
#include "node.h"
#include "gram.h"
#include "supr.h"
#include "tabl.h"

#include <ctype.h>


/*
 * Test whether sym is in the given class.
 */

Visible bool
isinclass(sym, ci)
	register int sym;
	struct classinfo *ci;
{
	register classptr cp;

	Assert(ci && ci->c_class);
	if (sym == Hole)
		return !isinclass(Optional, ci);
	for (cp = ci->c_class; *cp; ++cp)
		if (sym == *cp)
			return Yes;
	return No;
}


/*
 * Deliver the representation array for the given node.
 * If the node is actually just a "text" value, construct
 * one in static storage -- which is overwritten at each call.
 * In this case there are two deficiencies: the next call to
 * noderepr which uses the same feature overwrites the reply
 * value of the previous call, AND if the text value itself
 * is changed, the representation may change, too.
 * In practical use this is no problem at all, however.
 */

Visible string *
noderepr(n)
	register node n;
{
	register int sym;

	if (n && Type(n) == Tex) {
		static string buf[2];
		buf[0] = Str((value)n);
		return buf;
	}
	sym = symbol(n);
	return table[sym].r_repr;
}


/*
 * Deliver the prototype node for the given symbol.
 */

Visible node
gram(sym)
	register int sym;
{
	Assert(sym == 0 || sym > 0 && sym < TABLEN && table[sym].r_symbol);
	return table[sym].r_node;
}


/*
 * Deliver the name of a symbol.
 */

Visible string
symname(sym)
	int sym;
{
	static char buf[20];

	if (sym >= 0 && sym < TABLEN && table[sym].r_name)
		return table[sym].r_name;
	sprintf(buf, "%d", sym);
	return buf;
}


/*
 * Find the symbol corresponding to a given name.
 * Return -1 if not found.
 */

Visible int
nametosym(str)
	register string str;
{
	register int sym;
	register string name;

	for (sym = 0; sym < TABLEN; ++sym) {
		name = table[sym].r_name;
		if (name && Strequ(name, str))
			return sym;
	}
	return -1;
}


/*
 * Test whether `sym' may replace the node in the path `p'.
 */

Visible bool
allowed(p, sym)
	register path p;
	register int sym;
{
	register path pa = parent(p);
	register int ich = ichild(p);
	register int sympa = pa ? symbol(tree(pa)) : Rootsymbol;

	Assert(sympa >= 0 && sympa < TABLEN && ich > 0 && ich <= MAXCHILD);
	return isinclass(sym, table[sympa].r_class[ich-1]);
}


/*
 * Initialize (and verify) the grammar table.
 */

Visible Procedure
initgram()
{
	register int sym;
	register int nch;
	register struct classinfo **cp;
	register struct classinfo *sp;
	node ch[MAXCHILD];

	/* Set the node pointers in the table and check the representations.
	   The code assumes Optional and Hole are the last
	   symbols in the table, i.e. the first processed by the loop. */

	for (sym = TABLEN-1; sym >= 0; --sym) {
		if (table[sym].r_symbol != sym) {
			if (sym != Hole && sym != Optional && table[sym].r_symbol == 0)
				continue; /* Disabled table entry */
			syserr("initgram: table order (%s=%d, should be %d)",
				table[sym].r_name, table[sym].r_symbol, sym);
		}
		cp = table[sym].r_class;
		for (nch = 0; nch < MAXCHILD && (sp = cp[nch]); ++nch)
			ch[nch] =
				table[sp->c_class[0] == Optional ? Optional : Hole].r_node;
		ckrepr(table[sym].r_repr, nch);
		table[sym].r_node = newnode(nch, sym, ch);
		fix((value) table[sym].r_node);
	}

	initclasses();
}


/*
 * Check a representation array (subroutine for initgram).
 */

Hidden Procedure
ckrepr(rp, nch)
	register string *rp;
	register int nch;
{
	register string cp;
	register int i;
	register int ich;

	for (ich = 0; ich <= nch; ++ich) {
		cp = rp[ich];
		if (!cp)
			continue;
		for (i = 0; cp[i]; ++i) {
			switch (cp[i]) {
			case '\n':
			case '\r':
				if (i || ich)
					syserr("initgram (ckrepr): badly placed \\n/\\r");
				break;
			case '\t':
			case '\b':
				if (cp[i+1])
					syserr("initgram (ckrepr): badly placed \\t/\\b");
				break;
			default:
				if (cp[i] < ' ' || cp[i] >= 0177)
					syserr("initgram (ckrepr): illegal control char");
			}
		}
	}
}


/*
 * Compaction scheme for characters to save space in grammar tables
 * by combining characters with similar properties (digits, l.c. letters).
 */

#define RANGE 128 /* ASCII characters are in {0 .. RANGE-1} */

Visible char code_array[RANGE];
Visible char invcode_array[RANGE];
Visible int lastcode;

Hidden Procedure
initcodes()
{
	register int c;

	code_array['\n'] = ++lastcode;
	invcode_array[lastcode] = '\n';
	for (c = ' '; c <= '0'; ++c) {
		code_array[c] = ++lastcode;
		invcode_array[lastcode] = c;
	}
	for (; c <= '9'; ++c)
		code_array[c] = lastcode;
	for (; c <= 'a'; ++c) {
		code_array[c] = ++lastcode;
		invcode_array[lastcode] = c;
	}
	for (; c <= 'z'; ++c)
		code_array[c] = lastcode;
	for (; c < RANGE; ++c) {
		code_array[c] = ++lastcode;
		invcode_array[lastcode] = c;
	}
}


/*
 * Initialization routine for the 'struct classinfo' stuff.
 */

Hidden Procedure
initclasses()
{
	register int i;
	register int j;
	register struct table *tp;

	initcodes();
	for (i = 0; i < TABLEN; ++i) {
		tp = &table[i];
		if (tp->r_symbol != i)
			continue; /* Dead entry */
		for (j = 0; j < MAXCHILD && tp->r_class[j]; ++j) {
			if (!tp->r_class[j]->c_insert)
				defclass(tp->r_class[j]);
		}
	}
}

classptr makelist(); /* Forward */

Hidden Procedure
defclass(p)
	struct classinfo *p;
{
	register int c;
	struct table *tp;
	register classptr cp;
	register classptr cp1;
	classelem insert[1024];
	classelem append[1024];
	classelem join[1024];
	register int inslen = 0;
	register int applen = 0;
	register int joinlen = 0;
	register string *rp;
	int fw1;

	cp = p->c_class;
	Assert(cp);

	for (; *cp; ++cp) {
		if (*cp == Optional)
			continue;
		if (*cp >= TABLEN) { /* Insert direct lexical item */
			for (c = 1; c <= lastcode; ++c) {
				if (maystart(Invcode(c), *cp)) {
					Assert(inslen+3 < sizeof insert / sizeof insert[0]);
					insert[inslen] = c;
					insert[inslen+1] = *cp;
					inslen += 2;
				}
			}
			continue;
		}
		tp = &table[*cp];
		rp = tp->r_repr;
		if (!Fw_zero(rp[0])) { /* Insert fixed text */
			c = Code(rp[0][0]);
			Assert(inslen+3 < sizeof insert / sizeof insert[0]);
			insert[inslen] = c;
			insert[inslen+1] = *cp;
			inslen += 2;
			continue;
		}
		Assert(tp->r_class[0]);
		cp1 = tp->r_class[0]->c_class;
		Assert(cp1);
		for (; *cp1; ++cp1) {
			if (*cp1 < TABLEN)
				continue;
			for (c = 1; c <= lastcode; ++c) { /* Insert indir. lex. items */
				if (maystart(Invcode(c), *cp1)) {
					Assert(inslen+3 < sizeof insert / sizeof insert[0]);
					insert[inslen] = c;
					insert[inslen+1] = *cp;
					inslen += 2;
				}
			}
		}
		fw1 = Fwidth(rp[1]);
		if (fw1) { /* Append */
			c = rp[1][0];
			Assert(c > 0 && c < RANGE);
			if (c == ' ') {
				c = rp[1][1];
				if (!c || c == '\b' || c == '\t')
					c = ' ';
				else
					c |= 0200;
			}
			Assert(applen+3 < sizeof append / sizeof append[0]);
			append[applen] = c;
			append[applen+1] = *cp;
			applen += 2;
		}
		if ((!fw1 || fw1 == 1 && rp[1][0] == ' ')
			&& tp->r_class[1]) { /* Join */
			Assert(joinlen+3 < sizeof join / sizeof join[0]);
			join[joinlen] = 1 + fw1;
			join[joinlen+1] = *cp;
			joinlen += 2;
		}
	}

	Assert(inslen); /* Dead alley */
	insert[inslen] = 0;
	p->c_insert = makelist(insert, inslen + 1);
	if (applen) {
		append[applen] = 0;
		p->c_append = makelist(append, applen + 1);
	}
	if (joinlen) {
		join[joinlen] = 0;
		p->c_join = makelist(join, joinlen + 1);
	}
}

Hidden classptr
makelist(list, len)
	register classptr list;
	register int len;
{
	register classptr cp =
		(classptr) malloc((unsigned) (len*sizeof(classelem)));
	register int i;

	if (!cp)
		syserr("makelist: malloc");
	for (i = 0; i < len; ++i, ++list)
		cp[i] = *list;
	return cp;
}
