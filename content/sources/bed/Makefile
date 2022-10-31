# SccsId: @(#)Makefile	@(#)Makefile	1.16 1/4/84
#
# This software is copyright (c) Mathematical Centre, Amsterdam, 1983.
# Permission is granted to use and copy this software, but not for profit,
# and provided that these same conditions are imposed on any person
# receiving or using the software.

CFLAGS=-O
LDFLAGS=
OBJECTS= \
	bobj.o	demo.o	getc.o	ins2.o	line.o	node.o	que2.o	supr.o	unix.o \
	cell.o	edit.o	goto.o	inse.o	main.o	outp.o	save.o	tabl.o	wide.o \
	deco.o	eval.o	gram.o	lexi.o	move.o	que1.o	scrn.o	term.o

a.out: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -lcurses -ltermcap

bobj.o: b.h bobj.h bnum.h node.h
cell.o: b.h bobj.h node.h eval.h cell.h
deco.o: b.h bobj.h node.h gram.h supr.h queu.h
demo.o: b.h bobj.h node.h gram.h keys.h supr.h
edit.o: b.h bobj.h node.h tabl.h gram.h supr.h queu.h
eval.o: b.h node.h gram.h eval.h
getc.o: b.h keys.h
goto.o: b.h node.h gram.h supr.h
gram.o: b.h bobj.h node.h gram.h supr.h tabl.h
ins2.o: b.h bobj.h node.h supr.h queu.h gram.h tabl.h
inse.o: b.h bobj.h node.h gram.h supr.h tabl.h
lexi.o: b.h bobj.h node.h gram.h
line.o: b.h bobj.h node.h gram.h supr.h
main.o: b.h bobj.h
move.o: b.h bobj.h node.h supr.h gram.h
node.o: b.h bobj.h bnum.h node.h
outp.o: b.h bobj.h node.h supr.h gram.h cell.h
que1.o: b.h bobj.h node.h supr.h queu.h gram.h
que2.o: b.h bobj.h node.h supr.h queu.h gram.h tabl.h
save.o: b.h bobj.h node.h gram.h queu.h
scrn.o: b.h bobj.h node.h supr.h gram.h cell.h
supr.o: b.h node.h supr.h gram.h
tabl.o: b.h node.h gram.h tabl.h
term.o: b.h
unix.o: b.h
wide.o: b.h bobj.h node.h supr.h gram.h
