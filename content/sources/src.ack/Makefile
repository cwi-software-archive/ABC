MAKE=make
SHELL=/bin/sh
CC= /mnt/em/bin/ack -.c
COMMON=bcom.o bcmd.o benv.o berr.o bexp.o bfil.o bfpr.o blis.o bloc.o \
    bmai.o bmem.o bobj.o \
    bscr.o bsig.o bsou.o bsyn.o btab.o \
    btar.o btes.o btex.o btlt.o btyp.o buni.o
BIG=bfun.o bnuA.o bnuC.o bnuG.o bnuI.o bnuQ.o bnuR.o bnum.o bval.o
SMALL=Bfun.o Bnum.o Bval.o
SMALLF=Bfun.c Bnum.h Bnum.c Bval.c
PART1=b.h bcon.h bmem.h bmem.c bval.c bobj.h bobj.c \
      btex.c blis.c btab.c btlt.c bcom.c\
      bnum.h bnuA.c bnuC.c bnuG.c bnuI.c bnuQ.c bnuR.c \
      bnum.c bfun.c 
PART2=benv.c \
      bfil.h bfil.c bsig.h bsig.c bscr.h bscr.c berr.h berr.c \
      bkey.h bsyn.h bsyn.c \
      bsem.h btyp.h btyp.c bloc.c \
      bexp.c btar.c btes.c bfpr.c \
      bsou.h bsou.c buni.c bcmd.c bmai.c
FILES=$(PART1) $(PART2)
ALL=$(FILES) $(SMALLF)
CFLAGS=-O
#LDFLAGS= -i (PDP-11/{44,45,50,70} only)
LDFLAGS=
#GFLAG= -g (mystery)
GFLAG=
PRINTER=nroff -T1650.12
all: bnew
bnew:$(COMMON) $(BIG)
	-mv bnew bnew.old 2>/dev/null
	$(CC) $(GFLAG) $(CFLAGS) $(LDFLAGS) $(COMMON) $(BIG) -lm -o bnew
	size bnew bnew.old ../bnew $HOME/b.lib/oldb
#	job make test&
bsmall:$(COMMON) $(SMALL)
	-mv bsmall bsmall.old 2>/dev/null
	$(CC) $(GFLAG) $(CFALGS) $(LDFLAGS) $(COMMON) $(SMALL) -lm -o bsmall
	size bsmall bsmall.old bnew /other/dick/lib/b
Bval.c:bval.c
	cp bval.c Bval.c
Bval.o:Bval.c
	$(CC) -c $(CFLAGS) -DSMALLNUMBERS Bval.c
listing:part1 part2
part1:$(PART1)
	bdruk $(PART1)|lpr -J "B Sources" -C "Part One"
	echo Touch file>part1
part2:$(PART2)
	bdruk $(PART2)|lpr -J "B Sources" -C "Part Two"
	echo Touch file>part2
lprint:$(FILES)
	bdruk $(FILES)|lpr
	echo Touch file>lprint
tape:
	@echo -n "For: "; read name; echo Tape for $$name `date`>>.log
	tar cv `ls | grep -v \.o$$ | grep -v \@*`
test:bnew
	$HOME/impl/testnew
	echo Touch file>test
check:$(FILES)
	lint -ah b*.c>LINT.OUT
comparison:$(ALL)
	date>>.log
#	compare $?
	-for i in $?;\
	do	if diff .$$i $$i>@log;\
		then rm @log;\
		else echo ==== $$i ====>>.log;\
		     cat @log>>.log;\
		     rm @log;\
		fi;\
	done
	echo Touch file>comparison
backup:$(ALL)
	for i in $?; do cp $$i .$$i ; done
	echo Touch file>backup
edit:comparison backup
	-emacs $(FILES)
	make comparison
version:bnew
	cp bnew bnew.junk
	mv bnew.junk ../bnew
	echo Version `date`>>.log
	echo Touch file>version
index:
	@grep -n Visible $(ALL)|sort +2
	@grep -n Hidden $(ALL)|sort +2
depend:$(ALL)
	/bin/grep '^#[ 	]*include' /dev/null ${ALL} | \
		sed	-e 's,<\(.*\)>,"/usr/include/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' \
			-e 's/\.c/.o/' >makedep.junk
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep.junk
	echo '$$r makedep.junk' >>eddep.junk
	echo 'w' >>eddep.junk
	cp Makefile Makefile.old
	ed - Makefile < eddep.junk
	rm eddep.junk makedep.junk
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# See make depend above' >> Makefile
	echo Touch file>depend

# DO NOT DELETE THIS LINE -- make depend uses it

b.h: /usr/include/stdio.h
b.h: /usr/include/math.h
b.h: /usr/include/setjmp.h
bmem.o: b.h
bmem.o: bobj.h
bmem.o: bmem.h
bval.o: b.h
bval.o: bcon.h
bval.o: bobj.h
bval.o: bmem.h
bval.o: bscr.h
bval.o: bsem.h
bval.o: bnum.h
bval.o: Bnum.h
bobj.o: b.h
bobj.o: bobj.h
bobj.o: btlt.h
btex.o: b.h
btex.o: bobj.h
blis.o: b.h
blis.o: bobj.h
blis.o: btlt.h
btab.o: b.h
btab.o: bobj.h
btab.o: btlt.h
btlt.o: b.h
btlt.o: bobj.h
btlt.o: btlt.h
bcom.o: b.h
bcom.o: bobj.h
bnuA.o: b.h
bnuA.o: bcon.h
bnuA.o: bobj.h
bnuA.o: bnum.h
bnuC.o: b.h
bnuC.o: bcon.h
bnuC.o: bobj.h
bnuC.o: bmem.h
bnuC.o: bnum.h
bnuC.o: bsyn.h
bnuG.o: b.h
bnuG.o: bobj.h
bnuG.o: bnum.h
bnuI.o: b.h
bnuI.o: bobj.h
bnuI.o: bnum.h
bnuQ.o: b.h
bnuQ.o: bobj.h
bnuQ.o: bnum.h
bnuR.o: b.h
bnuR.o: bcon.h
bnuR.o: bobj.h
bnuR.o: bnum.h
bnum.o: b.h
bnum.o: bcon.h
bnum.o: bobj.h
bnum.o: bnum.h
bfun.o: /usr/include/math.h
bfun.o: b.h
bfun.o: bcon.h
bfun.o: bobj.h
bfun.o: bnum.h
benv.o: b.h
benv.o: bobj.h
bfil.o: b.h
bfil.o: bcon.h
bfil.o: /usr/include/sys/types.h
bfil.o: /usr/include/sys/stat.h
bfil.o: bobj.h
bfil.o: bfil.h
bsig.o: b.h
bsig.o: bobj.h
bsig.o: /usr/include/signal.h
bsig.o: berr.h
bscr.o: b.h
bscr.o: bobj.h
bscr.o: bscr.h
bscr.o: bfil.h
berr.o: b.h
berr.o: bobj.h
berr.o: berr.h
berr.o: bscr.h
berr.o: bsyn.h
berr.o: bsig.h
berr.o: bfil.h
bsyn.o: b.h
bsyn.o: bobj.h
bsyn.o: bscr.h
bsyn.o: bsyn.h
btyp.o: b.h
btyp.o: bobj.h
btyp.o: bsem.h
btyp.o: btyp.h
bloc.o: b.h
bloc.o: bobj.h
bloc.o: bsem.h
bexp.o: b.h
bexp.o: bcon.h
bexp.o: bobj.h
bexp.o: bmem.h
bexp.o: bsyn.h
bexp.o: bsem.h
bexp.o: bsou.h
btar.o: b.h
btar.o: bobj.h
btar.o: bsyn.h
btar.o: bsem.h
btes.o: b.h
btes.o: bobj.h
btes.o: bkey.h
btes.o: bsyn.h
btes.o: bsem.h
bfpr.o: b.h
bfpr.o: bobj.h
bfpr.o: bfil.h
bfpr.o: bsem.h
bfpr.o: bsyn.h
bfpr.o: bsou.h
bsou.o: b.h
bsou.o: bmem.h
bsou.o: bobj.h
bsou.o: bscr.h
bsou.o: berr.h
bsou.o: bkey.h
bsou.o: bsyn.h
bsou.o: bsem.h
bsou.o: bfil.h
bsou.o: bsou.h
buni.o: b.h
buni.o: bobj.h
buni.o: bmem.h
buni.o: bfil.h
buni.o: bscr.h
buni.o: berr.h
buni.o: bkey.h
buni.o: bsyn.h
buni.o: bsou.h
buni.o: bsem.h
bcmd.o: b.h
bcmd.o: bcon.h
bcmd.o: bobj.h
bcmd.o: bscr.h
bcmd.o: berr.h
bcmd.o: bkey.h
bcmd.o: bsyn.h
bcmd.o: bsem.h
bcmd.o: btyp.h
bmai.o: b.h
bmai.o: /usr/include/sys/types.h
bmai.o: bobj.h
bmai.o: bmem.h
bmai.o: bscr.h
bmai.o: berr.h
bmai.o: bfil.h
bmai.o: bsig.h
bmai.o: bsyn.h
bmai.o: bsem.h
bmai.o: bsou.h
Bfun.o: b.h
Bfun.o: bobj.h
Bfun.o: bsem.h
Bfun.o: Bnum.h
Bnum.o: b.h
Bnum.o: bcon.h
Bnum.o: bobj.h
Bnum.o: bsyn.h
Bnum.o: Bnum.h
Bval.o: b.h
Bval.o: bcon.h
Bval.o: bobj.h
Bval.o: bmem.h
Bval.o: bscr.h
Bval.o: bsem.h
Bval.o: bnum.h
Bval.o: Bnum.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# See make depend above
