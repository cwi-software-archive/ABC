/* SccsId: @(#)demo.c	1.14 1/2/84
 *
 * This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
 * Permission is granted to use and copy this software, but not for profit,
 * and provided that these same conditions are imposed on any person
 * receiving or using the software.
 */

/*
 * B editor -- Editor command processor.
 */

#include <stdio.h>
#include <ctype.h>

#include "curses.h"
#ifdef WINDOW
#ifdef bool
#undef bool
#endif
#endif

#include "b.h"
#include "bobj.h"
#include "node.h"
#include "gram.h"
#include "keys.h"
#include "supr.h"


/* Command line flags */
extern bool dflag;
extern bool hushbaby;
extern bool slowterminal;

bool lefttorite; /* Save some time in nosuggtoqueue() */

#define BELL '\7' /* Control-G */

#define COPYSAVEFILE ".Bed_buf"


Hidden bool
execute(ep, cmd)
	register environ *ep;
	register int cmd;
{
	register bool spflag = ep->spflag;
	register int i;
	environ env;
	char buf[2];
	register char *cp;

	if (ep->newmacro && cmd != USEMACRO && cmd != SAVEMACRO) {
		buf[0] = cmd;
		buf[1] = 0;
		concato(&ep->newmacro, buf);
	}
	ep->spflag = No;

	switch (cmd) {

	case SAVEMACRO:
		ep->spflag = spflag;
		if (ep->newmacro) { /* End definition */
			release(ep->oldmacro);
			ep->oldmacro = ep->newmacro;
			ep->newmacro = Vnil;
			message("Keystrokes recorded, use ^P to play back.");
		}
		else /* Start definition */
			ep->newmacro = mk_text("");
		return Yes;

	case USEMACRO:
		if (!ep->oldmacro || Length(ep->oldmacro) <= 0) {
			error("No keyboard macro defined.");
			return No;
		}
		cp = Str(ep->oldmacro);
		for (i = 0; i < Length(ep->oldmacro); ++i) {
			Ecopy(*ep, env);
			if (execute(ep, cp[i]&0377) && checkep(ep))
				Erelease(env);
			else {
				Erelease(*ep);
				Emove(env, *ep);
				if (!i)
					return No;
				error((char*)NULL); /* Just a bell */
				/* The error must be signalled here, because the overall
				   command (USEMACRO) succeeds, so the main loop
				   doesn't ring the bell; but we want to inform the
				   that not everything was done either. */
				return Yes;
			}
		}
		return Yes;

	case Ctl(_): /* 'Touch', i.e. set modified flag */
		ep->changed = Yes;
		return Yes;

	case GOTO:
		if (ep->newmacro) {
			error("You can't use control-G in recording mode\
 (it wouldn't work in playback)");
			return No;
		}
		return gotocursor(ep);

	case NEXT:
		return next(ep);

	case PREVIOUS:
		return previous(ep);

	case WIDEN:
		return widen(ep);

	case EXTEND:
		return extend(ep);

	case NARROW:
		return narrow(ep);

	case RNARROW:
		return rnarrow(ep);

	case UPARROW:
		return uparrow(ep);

	case DOWNARROW:
		return downarrow(ep);

	case COPY:
		ep->spflag = spflag;
		return copyinout(ep);

	case DELETE:
		return delete(ep);

	case ACCEPT:
		return tabulate(ep);

	/* Emacs-like functions: */

	/*****
	case Ctl(A):
		return em_startline(ep);
	case Ctl(E):
		return em_endline(ep);
	case Ctl(B):
		return em_back(ep);
	case Ctl(F):
		return em_forward(ep);
	case Ctl(P):
		return em_previous(ep);
	case Ctl(N):
		return em_next(ep);
	case Ctl(K):
		return em_kill(ep);
	case Ctl(Y):
		return em_yank(ep);
	case Ctl(V):
		return em_view(ep);
	*****/

	default:
		if (!isascii(cmd) || !isprint(cmd))
			return No;
		/* Fall through */
	case ' ':
		ep->spflag = spflag;
		return ins_char(ep, cmd, islower(cmd) ? toupper(cmd) : 0);

	case RETURN:
	case NEWLINE:
		return ins_newline(ep);
	}
}


#define MAXHIST 101 /* One more than the number of UNDO's allowed. */

#define Mod(k) (((k)+MAXHIST) % MAXHIST)
#define Succ(k) (((k)+1) % MAXHIST)
#define Pred(k) (((k)+MAXHIST-1) % MAXHIST)

Hidden environ *tobesaved;
Hidden string savewhere;

value editqueue();
Hidden bool errorset; /* Set by error message */

Visible bool
demo(filename, lineno)
	string filename;
	int lineno;
{
	int k;
	int h;
	int first = 0;
	int last = 0;
	int current = 0;
	int onscreen = -1;
	bool needupdate = No;
	bool reverse = No;
	environ newenv;
	environ *ep;
	int cmd;
	bool errors = No;
	int undoage = 0;
	bool done = No;

	environ history[MAXHIST];

	savewhere = filename;
	ep = &history[0];
	ep->focus = newpath(Pnil, gram(Optional), 1);
	ep->mode = WHOLE;
	ep->copyflag = ep->spflag = ep->changed = No;
	ep->s1 = ep->s2 = ep->s3 = 0;
	ep->highest = Maxintlet;
	ep->copybuffer = Vnil;
	ep->oldmacro = ep->newmacro = Vnil;
	ep->generation = 0;

	lefttorite = Yes;
	errors = !edit(ep, filename, lineno);
	lefttorite = No;
	ep->changed = No;
	ep->copybuffer = editqueue(COPYSAVEFILE);
	if (ep->copybuffer)
		ep->copyflag = Yes;

	for (;;) { /* Command interpretation loop */
		if (onscreen != current)
			virtupdate(onscreen < 0 ? (environ*)NULL : &history[onscreen],
				&history[current],
				reverse && onscreen >= 0 ?
					history[onscreen].highest : history[current].highest);
		onscreen = current;
		if (!moreinput()) {
			actupdate(No);
			if (!errorset && !moreinput())
				stsmess(&history[current]);
		}
		cmd = inchar();
		if (errorset) {
			message(NULL);
			errorset = No;
		}
		errors = No;

		switch (cmd) {

#ifndef NDEBUG
		case Ctl(@): /* Debug exit with variable dump */
			tobesaved = (environ*)NULL;
			return No;
#endif NDEBUG

		case Ctl(^): /* Debug status message */
			dbmess(&history[current]);
			errors = Yes; /* Causes clear after new keystroke seen */
			continue;

		case UNDO:
			if (current == first)
				errors = Yes;
			else {
				if (onscreen == current)
					reverse = Yes;
				current = Pred(current);
				undoage = Mod(last-current);
			}
			break;

		case REDO:
			if (current == last)
				errors = Yes;
			else {
				if (current == onscreen)
					reverse = No;
				if (history[Succ(current)].generation <
						history[current].generation)
					error("This redo brought you to an older version.\
 Type backspace to undo.");
				current = Succ(current);
				undoage = Mod(last-current);
			}
			break;

		case REDRAW:
			if (onscreen >= 0) {
				onscreen = -1;
			}
			break;

		case EOF:
		case EXIT:
			if (history[current].generation > 0)
				save(history[current].focus, filename);
			if (history[current].copyflag)
				savequeue(history[current].copybuffer, COPYSAVEFILE);
			else
				savequeue(Vnil, COPYSAVEFILE);
			actupdate(Yes);
			lineno = -focoffset(&history[current]);
			if (lineno < 0)
				lineno = 0;
			lineno += Ycoord(history[current].focus);
			savepos(savewhere, lineno+1);
			tobesaved = (environ*)NULL;
			for (current = first; current != last;
					current = Succ(current))
				Erelease(history[current]);
			Erelease(history[last]);
			return Yes;

		default:
			Ecopy(history[current], newenv);
			newenv.highest = Maxintlet;
			newenv.changed = No;
			errors = !execute(&newenv, cmd) || !checkep(&newenv);
			if (errors)
				Erelease(newenv);
			else {
				if (newenv.changed)
					++newenv.generation;
				last = Succ(last);
				current = Succ(current);
				if (last == first) {
					/* Array full (always after a while). Discard "oldest". */
					if (current == last
						|| undoage < Mod(current-first)) {
						Erelease(history[first]);
						first = Succ(first);
						if (undoage < MAXHIST)
							++undoage;
					}
					else {
						last = Pred(last);
						Erelease(history[last]);
					}
				}
				if (current != last
					&& newenv.highest < history[current].highest)
					history[current].highest = newenv.highest;
				/* Move entries beyond current one up. */
				for (k = last; k != current; k = Pred(k)) {
					if (Pred(k) == onscreen)
						onscreen = k;
					history[k] = history[Pred(k)];
				}
				Ecopy(newenv, history[current]);
				Erelease(history[current]);
				if (onscreen == first || onscreen == last)
					needupdate = onscreen >= 0; /* Take no risks */
			}
			break; /* default */

		} /* switch */

		if (errors && !errorset) {
			if (!slowterminal && (isprint(cmd) || cmd == ' '))
				error("Cannot insert '%c'", cmd);
			else
				error((char*)NULL);
		}
		tobesaved = &history[current];
	} /* for (;;) */
}


/*
 * Issue bottom line status message.
 */

Hidden Procedure
stsmess(ep)
	environ *ep;
{
	message("%s%s",
		ep->copyflag ? "[Copy buffer] " : "",
		ep->newmacro ? "[Recording (^R to end)] " : "");
}


/*
 * Save parse tree and copy buffer.
 */

Visible Procedure
enddemo()
{
	register environ *ep = tobesaved;
	register int lineno;

	tobesaved = (environ*)NULL;
		/* To avoid loops if saving is interrupted. */
	if (ep) {
		if (ep->generation > 0)
			save(ep->focus, savewhere);
		if (ep->copyflag)
			savequeue(ep->copybuffer, COPYSAVEFILE);
		else
			savequeue(Vnil, COPYSAVEFILE);
		lineno = -focoffset(ep);
		if (lineno < 0)
			lineno = 0;
		lineno += Ycoord(ep->focus);
		savepos(savewhere, lineno+1);
	}
}


/*
 * Find out if the current position is higher in the tree
 * than `ever' before (as remembered in ep->highest).
 * The algorithm of pathlength() is repeated here to gain
 * some efficiency by stopping as soon as it is clear
 * no change can occur.
 * (Higher() is called VERY often, so this pays).
 */

Visible Procedure
higher(ep)
	register environ *ep;
{
	register path p = ep->focus;
	register int pl = 0;
	register int max = ep->highest;

	while (p) {
		++pl;
		if (pl >= max)
			return;
		p = parent(p);
	}
	ep->highest = pl;
}


/*
 * Issue an error message and set a flag ('errorset')
 * that a message was issued.  The bell is also rung.
 * If 'fmt' is the NULL string, the bell is rung but
 * no message is issued and the flag is not set.
 */

/* VARARGS 1 */
Visible Procedure
error(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
	string fmt;
{
	if (!hushbaby)
		putchar(BELL);
	fflush(stdout);
	if (fmt) {
		message(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		errorset = Yes;
	}
}


/*
 * Issue debug status line.
 */

Visible Procedure
dbmess(ep)
	register environ *ep;
{
	char stuff[80];
	register string str = stuff;

	switch (ep->mode) {
	case VHOLE:
		sprintf(stuff, "VHOLE:%d.%d", ep->s1, ep->s2);
		break;
	case FHOLE:
		sprintf(stuff, "FHOLE:%d.%d", ep->s1, ep->s2);
		break;
	case ATBEGIN:
		str = "ATBEGIN";
		break;
	case ATEND:
		str = "ATEND";
		break;
	case WHOLE:
		str = "WHOLE";
		break;
	case SUBRANGE:
		sprintf(stuff, "SUBRANGE:%d.%d-%d", ep->s1, ep->s2, ep->s3);
		break;
	case SUBSET:
		sprintf(stuff, "SUBSET:%d-%d", ep->s1, ep->s2);
		break;
	case SUBLIST:
		sprintf(stuff, "SUBLIST...%d", ep->s3);
		break;
	default:
		sprintf(stuff, "UNKNOWN:%d,%d,%d,%d",
			ep->mode, ep->s1, ep->s2, ep->s3);
	}
	message("%s, %s, wi=%d, hi=%d, (y,x,l)=(%d,%d,%d) %s",
		symname(symbol(tree(ep->focus))),
		str, width(tree(ep->focus)), ep->highest,
		Ycoord(ep->focus), Xcoord(ep->focus), Level(ep->focus),
			ep->spflag ? "spflag on" : "");
	errorset = Yes;
}


/*
 * Print a message (printf-format + arguments) on the bottom line.
 * If a nil pointer is passed as format, the bottom line is only cleared.
 * The cursor is moved back to where it was originally, and the buffer
 * is flushed (the screen is updated).
 */

/* VARARGS 1 */
Hidden Procedure
message(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
	register string fmt;
{
	register int c;
	register char *str;
	auto int savey;
	auto int savex;
	char buf[200];

	getyx(stdscr, savey, savex);
	move(LINES-1, 0);
	if (fmt) {
		sprintf(buf, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		buf[COLS] = 0;
		for (str = buf; c = *str; ++str)
			if (c == ' ' || isprint(c))
				addch(c);
	}
	clrtoeol();
	move(savey, savex);
	refresh();
}

/* VARARGS1 */
Visible Procedure
debug(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)
	string fmt;
{
#ifdef NDEBUG
	if (dflag)
#endif NDEBUG
		message(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}
