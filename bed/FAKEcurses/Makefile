CFLAGS=-O -pg
crlib:	crlib(curses.o) crlib(termcap.o)
	ranlib crlib
crlib(curses.o):	curses.h
	cc -c -O curses.c
	ar rv crlib curses.o
	rm curses.o
crlib(termcap.o):
	cc -c -O termcap.c
	ar rv crlib termcap.o
	rm termcap.o
