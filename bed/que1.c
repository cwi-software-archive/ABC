/* SccsId: @(#)que1.c	1.12 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Manipulate queues of nodes, lower levels.
 */

#include "b.h"
#include "bobj.h"
#include "node.h"
#include "supr.h"
#include "queu.h"
#include "gram.h"


value grab_com();


/*
 * Append queue 2 to the end of queue 1.
 */

Visible Procedure
joinqueues(pq, q)
	register queue *pq;
	register queue q;
{
	if (emptyqueue(q))
		return;
	while (*pq) {
		if (Refcnt(*pq) > 1)
			uniql((value*)pq);
		pq = &(*pq)->q_link;
	}
	*pq = q;
}


/*
 * Prepend a node to a queue ("push").
 * Empty strings and Optional holes are silently discarded;
 * trailing \t or \b on strings are stripped.
 */

Visible Procedure
preptoqueue(n, pq)
	node n;
	register queue *pq;
{
	register queue q;

	if (Type(n) == Tex) { /* Discard trailing tab/bs, avoid empty strings */
		string str = Str((value)n);
		int len = Length((value)n);
		if (Fw_zero(str))
			return;
		n = nodecopy(n);
		if (str[len-1] == '\b' || str[len-1] == '\t')
			putintrim((value *) &n, len-1, 0, "");
	}
	else { /* Avoid Optional holes */
		if (symbol(n) == Optional)
			return;
		n = nodecopy(n);
	}
	q = (queue) grab_com(2);
	q->q_data = n;
	q->q_link = *pq;
	*pq = q;
}


/*
 * Append a node to the end of a queue (same extras as preptoqueue).
 */

Visible Procedure
addtoqueue(pq, n)
	register queue *pq;
	register node n;
{
	auto queue q = Qnil;

	preptoqueue(n, &q);
	joinqueues(pq, q);
}


/*
 * Push a string onto a queue.
 */

Visible Procedure
stringtoqueue(str, pq)
	register string str;
	register queue *pq;
{
	register value v = mk_text(str);

	preptoqueue((node) v, pq);
	release(v);
}


/*
 * Append a string to a queue.
 */

Visible Procedure
addstringtoqueue(pq, str)
	register queue *pq;
	register string str;
{
	register value v = mk_text(str);

	addtoqueue(pq, (node) v);
	release(v);
}


/*
 * Get the first node of a queue and delink it ("pop").
 */

Visible node
queuebehead(pq)
	register queue *pq;
{
	register node n;
	register queue q = *pq;

	Assert(q);

	n = nodecopy(q->q_data);
	*pq = qcopy(q->q_link);
	qrelease(q);
	return n;
}


/*
 * Split a node in successive queue elements which are pushed
 * on the queue using preptoqueue.
 * 'Atomic' nodes (texts and holes) are pushed unadorned.
 */

Visible Procedure
splitnode(n, pq)
	register node n;
	register queue *pq;
{
	register node nn;
	register string *rp;
	register int i;
	register int sym;

	if (Type(n) == Tex) {
		preptoqueue(n, pq);
		return;
	}
	sym = symbol(n);
	if (sym == Optional)
		return;
	if (sym == Hole) {
		if (sym != Optional)
			preptoqueue(n, pq);
		return;
	}

	rp = noderepr(n);
	for (i = nchildren(n); i >= 0; --i) {
		if (!Fw_zero(rp[i]))
			stringtoqueue(rp[i], pq);
		if (i) {
			nn = child(n, i);
			if (Type(nn) == Tex || symbol(nn) != Optional)
				preptoqueue(nn, pq);
		}
	}
}


/*
 * Substitute the focus for its parent, appending the remainder of
 * the parent to the queue.
 * The focus must be the first child and not preceded by fixed text.
 * The focus must be allowed in the place of its parent.
 * If any of these conditions is not met, No is returned and nothing
 * is changed.
 */

Visible bool
resttoqueue(pp, pq)
	register path *pp;
	register queue *pq;
{
	auto queue q = Qnil;
	register path pa = parent(*pp);
	register node n = tree(*pp);
	register int sym = symbol(n);
	register markbits x;

	if (!pa || ichild(*pp) != 1
		|| fwidth(noderepr(tree(pa))[0]) != 0 || !allowed(pa, sym))
		return No;

	n = nodecopy(n);
	x = marks(n);
	up(pp) || Abort();
	splitnode(tree(*pp), &q);
	noderelease(queuebehead(&q));
	replace(pp, n);
	if (x) {
		markpath(pp, x); /* Actually, should restore all n's marks? */
	}
	joinqueues(pq, q);
	return Yes;
}


/*
 * Like resttoqueue, but exactly from current position in fixed text.
 * Also, it cannot fail.
 */

Visible Procedure
nosuggtoqueue(ep, pq)
	register environ *ep;
	queue *pq;
{
	auto queue q = Qnil;
	register int i;
	register string *rp;
	register node n;
	register node nn;
	register int sym;
	string str;

	if (issuggestion(ep))
		return;
	Assert((ep->mode == FHOLE || ep->mode == VHOLE)
		&& (ep->s1&1));

	n = tree(ep->focus);
	rp = noderepr(n);
	for (i = nchildren(n); i > ep->s1/2; --i) {
		if (!Fw_zero(rp[i]))
			stringtoqueue(rp[i], &q);
		nn = child(n, i);
		sym = symbol(nn);
		if (sym != Optional) {
			preptoqueue(nn, &q);
			if (sym != Hole) {
				s_downi(ep, i);
				delfocus(&ep->focus);
				s_up(ep);
			}
		}
	}
	str = rp[i];
	if (str && str[ep->s2]) /* Push partial first text */
		stringtoqueue(str + ep->s2, &q);
	joinqueues(pq, q);
}


/*
 * Check whether the remainder of the current node is all suggestion.
 */

Visible bool
issuggestion(ep)
	register environ *ep;
{
	register node n;
	register int nch;
	register int sym;
	register int i;

	if (ep->mode != VHOLE && ep->mode != FHOLE || !(ep->s1&1))
		return No; /* Actually wrong call? */

	n = tree(ep->focus);
	nch = nchildren(n);
	for (i = ep->s1/2 + 1; i <= nch; ++i) {
		sym = symbol(child(n, i));
		if (sym != Hole && sym != Optional)
			return No;
	}
	return Yes;
}


/*
 * See if a node fits in a hole.
 */

Visible bool
fitnode(pp, n)
	register path *pp;
	register node n;
{
	if (!allowed(*pp, symbol(n)))
		return No;
	replace(pp, nodecopy(n));
	return Yes;
}


/*
 * Fit a string in a hole.
 * Returns the number of characters consumed.
 * (This does not have to be the maximum possible, but a reasonable attempt
 * is made.  If the internal buffer is exhausted, it leaves the rest for
 * another call.)
 */

Visible int
fitstring(pp, str, alt_c)
	register path *pp;
	register string str;
	int alt_c;
{
	environ dummyenv;
	register node n;
	register int ich;
	register int len;
	register string cp;
	char buf[1024];

	Assert(str);
	if (!str[0])
		return 0;
	if (!insguess(pp, str[0], &dummyenv)) {
		if (!alt_c)
			return 0;
		if (!insguess(pp, alt_c, &dummyenv))
			return 0;
	}
	if (Type(tree(*pp)) == Tex)
		up(pp) || Abort();
	if (dummyenv.mode == FHOLE) {
		cp = noderepr(tree(*pp))[0];
		len = 1;
		if (cp) {
			++str;
			++cp;
			while (*str >= ' ' && *str == *cp) {
				++len;
				++str;
				++cp;
			}
		}
		return len;
	}
	if (dummyenv.mode == VHOLE) {
		buf[0] = str[0];
		++str;
		len = 1;
		n = tree(*pp);
		ich = dummyenv.s1/2;
		while (*str && mayinsert(n, ich, len, *str) && len < sizeof buf - 1) {
			buf[len] = *str;
			++str;
			++len;
		}
		if (len > 1) {
			buf[len] = 0;
			downi(pp, ich) || Abort();
			replace(pp, (node) mk_text(buf));
			up(pp) || Abort();
		}
		return len;
	}
	return 1;
}


/*
 * Set the focus position (some VHOLE/FHOLE setting, probably)
 * at the 'len'th character from the beginning of the current node.
 * This may involve going to a child or moving beyond the current subtree.
 * Negative 'len' values may be given to indicate negative widths;
 * this is implemented incomplete.
 */

Visible Procedure
fixfocus(ep, len)
	register environ *ep;
	register int len;
{
	node nn;
	register node n = tree(ep->focus);
	register string *rp;
	register int i = 0;
	register int nch;
	register int w;

	if (Type(n) == Tex) {
		w = Length((value)n);
		Assert(w >= len && len >= 0);
		if (w > len)
			ep->spflag = No;
		ep->mode = VHOLE;
		ep->s1 = ichild(ep->focus) * 2;
		ep->s2 = len;
		s_up(ep);
		return;
	}
	nch = nchildren(n);
	w = width(n);
	if (len > w && w >= 0) {
		i = ichild(ep->focus); /* Change initial condition for for-loop */
		if (!up(&ep->focus)) {
			ep->mode = ATEND;
			return;
		}
		higher(ep);
		n = tree(ep->focus);
	}

	rp = noderepr(n);
	for (; i <= nch; ++i) {
		if (i) {
			nn = child(n, i);
			w = width(nn);
			if (w < 0 || w >= len && len >= 0) {
				s_downi(ep, i);
				fixfocus(ep, len);
				return;
			}
			if (len >= 0)
				len -= w;
		}
		w = Fwidth(rp[i]);
		if (w >= len && len >= 0) {
			if (w > len)
				ep->spflag = No;
			ep->mode = FHOLE;
			ep->s1 = 2*i + 1;
			ep->s2 = len;
			return;
		}
		else if (w < 0)
			len = 0;
		else
			len -= w;
	}
	ep->mode = ATEND;
}


/*
 * Apply, if possible, a special fix relating to spaces:
 * when a space has been interpreted as joining character
 * and we end up in the following hole, but we don't succeed
 * in filling the hole; it is then tried to delete the hole
 * and the space.
 * Usually this doesn't occur, but it may occur when inserting
 * after a space that was already fixed on the screen but now
 * deserves re-interpretation.
 */

Visible bool
spacefix(ep)
	environ *ep;
{
	path pa;
	node n;
	string *rp;

	if (ichild(ep->focus) != 2 || symbol(tree(ep->focus)) != Hole)
		return No;
	pa = parent(ep->focus);
	n = tree(pa);
	rp = noderepr(n);
	if (!Fw_zero(rp[0]) || Fwidth(rp[1]) != 1 || rp[1][0] != ' ')
		return No;
	n = child(n, 1);
	if (!allowed(pa, symbol(n)))
		return No;
	s_up(ep);
	replace(&ep->focus, nodecopy(n));
	ep->mode = ATEND;
	ep->spflag = Yes;
	return Yes;
}


/*
 * Prepend a subset of a node to a queue.
 */

Visible Procedure
subsettoqueue(n, s1, s2, pq)
	register node n;
	register int s1;
	register int s2;
	register queue *pq;
{
	register string *rp = noderepr(n);

	for (; s2 >= s1; --s2) {
		if (s2&1)
			stringtoqueue(rp[s2/2], pq);
		else
			preptoqueue(child(n, s2/2), pq);
	}
}
