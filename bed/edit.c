/* SccsId: @(#)edit.c	1.8 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Read unit from file.
 */

#include <stdio.h>
#include <ctype.h>

#include "b.h"
#include "bobj.h"
#include "node.h"
#include "tabl.h"
#include "gram.h"
#include "supr.h"
#include "queu.h"

string unixerror();

/*
 * The definition here of TABS is unrelated to the definition of TABS
 * in eval.h (used by show.c and eval.c).  The definition here only
 * defines how many spaces must be equivalenced to a tab stop when read
 * from a file; such spaces must be caused by editing a unit with another
 * editor (vi, ed, ex, emacs), since "save.c" always writes tabs,
 * not spaces.  The value '4' is best suited for people at the CWI who
 * may have age-old workspaces with units edited through a special version
 * of 'vi', where a tab was equivalent to 4 spaces.  Also, this gives the
 * prettiest look when printing units.
 *
 * The variables 'spacesused' and 'tabsused' are kept to see if mixed use
 * of spaces and tabs was made; this can cause indentation errors.
 */

#define TABS 4

Hidden bool spacesused;
Hidden bool tabsused;


/*
 * Read (edit) parse tree from file into the focus.
 * Rather ad hoc, we use ins_string for each line
 * and do some magic tricks to get the indentation right
 * (most of the time).
 * If line > 0, position the focus at that line, if possible;
 * otherwise the focus is left at the end of the inserted text.
 */

Visible bool
edit(ep, filename, line)
	register environ *ep;
	string filename;
	int line;
{
	int lines = 0;
	register FILE *fp = fopen(filename, "r");
	register int c;
	char buf[BUFSIZ];
	auto string cp;
	auto queue q = Qnil;

	if (!fp) {
		error("%s", unixerror(filename));
		return No;
	}

	do {
		do {
			for (cp = buf; cp < buf + sizeof buf - 1; ++cp) {
				c = getc(fp);
				if (c == EOF || c == '\n')
					break;
				if (c < ' ' || c >= 0177)
					c = ' ';
				*cp = c;
			}
			if (cp > buf) {
				*cp = 0;
				if (!ins_string(ep, buf, &q, 0) || !emptyqueue(q)) {
					qrelease(q);
					error("Trouble reading your unit, see last line.\
 Hit break if you don't want this.");
					return No;
				}
				qrelease(q);
			}
		} while (c != EOF && c != '\n');
		++lines;
		if (c != EOF && !editindentation(ep, fp))
			return No;
	} while (c != EOF);
	if (ep->mode == FHOLE || ep->mode == VHOLE && (ep->s1&1)) {
		cp = "";
		soften(ep, &cp, 0);
	}
	if (lines > 1 && line > 0) {
		gotoyx(ep, line-1, 0);
		oneline(ep);
	}
	if (spacesused && tabsused)
		error("Spaces and tabs mixed for indentation;\
 check your program layout.");
	return Yes;
}


/*
 * Do all the footwork required to get the indentation proper.
 */

Hidden Procedure
editindentation(ep, fp)
	register environ *ep;
	register FILE *fp;
{
	register int tabs = 0;
	auto int level;
	register int c;

	while ((c = getc(fp)) == ' ' || c == '\t') {
		if (c == ' ') {
			spacesused = Yes;
			++tabs;
		}
		else {
			tabsused = Yes;
			tabs = (tabs/TABS + 1)*TABS;
		}
	}
	ungetc(c, fp);
	if (c == EOF || c == '\n')
		return Yes;
	tabs = (tabs+(TABS/2))/TABS; /* Transform to tab stops */
	if (!ins_newline(ep)) {
		error("Burp! Can't insert a newline.");
		return No;
	}
	getcoord(ep->focus, (int*)NULL, (int*)NULL, &level);
	for (; tabs < level; --level) {
		if (!ins_newline(ep)) {
			error("Burp, burp! Can't decrease indentation.");
			return No;
		}
	}
	fixit(ep);
	return Yes;
}


/* ------------------------------------------------------------ */


/*
 * Read the next non-space character.
 */

Hidden int
skipsp(fp)
	register FILE *fp;
{
	register int c;

	do {
		c = getc(fp);
	} while (c == ' ');
	return c;
}


/*
 * Read a text in standard B format when the initial quote has already
 * been read.
 */

Hidden value
readtext(fp, quote)
	register FILE *fp;
	register char quote;
{
	auto value v = Vnil;
	char buf[BUFSIZ];
	register string cp = buf;
	register int c;
	auto int i;

	for (; ; ++cp) {
		c = getc(fp);
		if (!isascii(c) || c != ' ' && !isprint(c)) {
			if (c == EOF)
				debug("readtext: EOF");
			else
				debug("readtext: bad char (0%02o)", c);
			release(v);
			return Vnil; /* Bad character or EOF */
		}
		if (c == quote) {
			c = getc(fp);
			if (c != quote) {
				ungetc(c, fp);
				break;
			}
		}
		else if (c == '`') {
			c = skipsp(fp);
			if (c == '$') {
				i = 0;
				if (fscanf(fp, "%d", &i) != 1
					|| i == 0 || !isascii(i)) {
					debug("readtext: error in conversion");
					release(v);
					return Vnil;
				}
				c = skipsp(fp);
			}
			else
				i = '`';
			if (c != '`') {
				if (c == EOF)
					debug("readtext: EOF in conversion");
				else
					debug("readtext: bad char in conversion (0%o)", c);
				release(v);
				return Vnil;
			}
			c = i;
		}
		if (cp >= &buf[sizeof buf - 1]) {
			*cp = 0;
			if (v)
				concato(&v, buf);
			else
				v = mk_text(buf);
			cp = buf;
		}
		*cp = c;
	}
	*cp = 0;
	if (!v)
		return mk_text(buf);
	concato(&v, buf);
	return v;
}


Hidden int
readsym(fp)
	register FILE *fp;
{
	register int c;
	char buf[100];
	register string bufp;

	for (bufp = buf; ; ++bufp) {
		c = getc(fp);
		if (c == EOF)
			return -1;
		if (!isascii(c) || !isalnum(c) && c != '_') {
			if (ungetc(c, fp) == EOF)
				syserr("readsym: ungetc failed");
			break;
		}
		*bufp = c;
	}
	*bufp = 0;
	if (isdigit(buf[0]))
		return atoi(buf);
	if (strcmp(buf, "Required") == 0) /***** Compatibility hack *****/
		return Hole;
	return nametosym(buf);
}


/*
 * Read a node in internal format (recursively).
 * Return nil pointer if EOF or error.
 */

Hidden node
readnode(fp)
	FILE *fp;
{
	int c;
	int nch;
	node ch[MAXCHILD];
	node n;
	int sym;

	c = skipsp(fp);
	switch (c) {
	case EOF:
		debug("readnode: EOF");
		return Nnil; /* Unexpected EOF */

	case '(':
		sym = readsym(fp);
		if (sym < 0) {
			debug("readnode: missing symbol");
			return Nnil; /* No number as first item */
		}
		if (sym < 0 || sym > Hole) {
			debug("readnode: bad symbol (%d)", sym);
			return Nnil;
		}
		nch = 0;
		while ((c = skipsp(fp)) == ',' && nch < MAXCHILD) {
			n = readnode(fp);
			if (!n) {
				for (; nch > 0; --nch)
					noderelease(ch[nch-1]);
				return Nnil; /* Error encountered in child */
			}
			ch[nch] = n;
			++nch;
		}
		if (c != ')') {
			if (c == ',')
				debug("readnode: node too long (sym=%d)", sym);
			else
				debug("readnode: no ')' where expected (sym=%d)", sym);
			for (; nch > 0; --nch)
				noderelease(ch[nch-1]);
			return Nnil; /* Not terminated with ')' or too many children */
		}
		if (nch == 0)
			return gram(sym); /* Saves space for Optional/Hole nodes */
		return newnode(nch, sym, ch);

	case '\'':
	case '"':
		return (node) readtext(fp, c);

	default:
		debug("readnode: bad initial character");
		return Nnil; /* Bad initial character */
	}
}


/*
 * Read a node written in a more or less internal format.
 */

Visible value
editqueue(filename)
	string filename;
{
	register FILE *fp = fopen(filename, "r");
	auto queue q = Qnil;
	register node n;

	if (!fp)
		return Vnil;
	do {
		n = readnode(fp);
		if (!n)
			break;
		addtoqueue(&q, n);
		noderelease(n);
	} while (skipsp(fp) == '\n');
	fclose(fp);
	return (value)q;
}
