# Make small numeric package

CFLAGS=-i -O -I.. -DSMALLNUMBERS

all: bfun.o bnum.o bval.o

bfun.o: ../b.h ../bnum.h ../bobj.h ../bfun.h ../bsem.h bnuM.h
bnum.o: ../b.h ../bnum.h ../bobj.h ../bsyn.h bnuM.h
bval.o: ../b.h ../bobj.h ../bmem.h ../bscr.h ../bsem.h ../bnum.h
