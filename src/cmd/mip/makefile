# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
INSTALL=./ccom
M=/usr/src/cmd/mip
CFLAGS=-O

GETALL=	allo.c cgram.y comm1.c common convert fallo.c fcomm2.c fconvert \
	flocal2.c fmatch.c forder.c fort.c freader.c ftable.c manifest \
	match.c mfile1 mfile2 optim.c pftn1.c pftn2.c reader.c scan.c trees.c \
	xdefs.c

GETL=	manifest mfile1 cgram.y xdefs.c scan.c common comm1.c pftn1.c \
	pftn2.c trees.c optim.c

GETP =	cgram.y xdefs.c scan.c pftn1.c pftn2.c trees.c optim.c reader.c \
	match.c allo.c comm1.c comm2.c mfile2 manifest mfile1 common

nocomp:
	@echo This is a prototype makefile only.

head:  comp ;

all cp cmp:
	@echo "This makefile is just a prototype for a portable"
	@echo "C compiler-- it isn't used for an existing command"

getl:	${GETL}

getp:	${GETP}

getall:	${GETALL}

comp: cgram.o xdefs.o scan.o pftn.o pftn1.o pftn2.o trees.o optim.o code.o local.o reader.o local2.o order.o match.o allo.o comm1.o table.o
	cc $(CFLAGS) -i cgram.o xdefs.o scan.o pftn.o trees.o optim.o code.o local.o  reader.o local2.o order.o match.o allo.o comm1.o table.o
	mv a.out comp
trees.o: $M/manifest macdefs $M/mfile1 $M/trees.c
	cc -c $(CFLAGS) -I$M -I. $M/trees.c
optim.o: $M/manifest macdefs $M/mfile1 $M/optim.c
	cc -c $(CFLAGS) -I$M -I. $M/optim.c
pftn.o: $M/manifest macdefs $M/mfile1 $M/pftn.c
	cc -c $(CFLAGS) -I$M -I. $M/pftn.c
code.o: $M/manifest macdefs $M/mfile1
	cc -c $(CFLAGS) -I$M -I. code.c
local.o: $M/manifest macdefs $M/mfile1
	cc -c $(CFLAGS) -I$M -I. local.c
scan.o: $M/manifest macdefs $M/mfile1 $M/scan.c
	cc -c $(CFLAGS) -I$M -I. $M/scan.c
xdefs.o: $M/manifest $M/mfile1 macdefs $M/xdefs.c
	cc -c $(CFLAGS) -I$M -I. $M/xdefs.c
cgram.o: $M/manifest $M/mfile1 macdefs $M/cgram.c
	cc -c $(CFLAGS) -I$M -I. $M/cgram.c
$M/cgram.c: $M/cgram.y
	yacc $M/cgram.y
	mv y.tab.c $M/cgram.c
comm1.o: $M/manifest $M/mfile1 $M/common macdefs $M/comm1.c
	cc -c $(CFLAGS) -I. -I$M $M/comm1.c
table.o: $M/manifest $M/mfile2 mac2defs macdefs table.c
	cc -c $(CFLAGS) -I$M -I. table.c
reader.o: $M/manifest $M/mfile2 mac2defs macdefs $M/reader.c
	cc -c $(CFLAGS) -I$M -I. $M/reader.c
local2.o: $M/manifest $M/mfile2 mac2defs macdefs
	cc -c $(CFLAGS) -I$M -I. local2.c
order.o: $M/manifest $M/mfile2 mac2defs macdefs
	cc -c $(CFLAGS) -I$M -I. order.c
match.o: $M/manifest $M/mfile2 mac2defs macdefs $M/match.c
	cc -c $(CFLAGS) -I$M -I. $M/match.c
allo.o: $M/manifest $M/mfile2 mac2defs macdefs $M/allo.c
	cc -c $(CFLAGS) -I$M -I. $M/allo.c
shrink:
	rm *.o comp
lintall:
	lint -hpv -I. -I$M  $M/cgram.c $M/xdefs.c $M/scan.c $M/pftn.c $M/trees.c $M/optim.c code.c local.c $M/reader.c local2.c order.c $M/match.c $M/allo.c $M/comm1.c table.c
fort: comp fort.o freader.o fallo.o fmatch.o ftable.o forder.o flocal2.o fcomm2.o
	cc -i $(CFLAGS) fort.o freader.o fallo.o fmatch.o ftable.o forder.o flocal2.o fcomm2.o
	mv a.out fort
fort.o: fort.h $M/fort.c
	cc -c $(CFLAGS) -I$M -I. $M/fort.c
freader.o: reader.o
	cc -c $(CFLAGS) -I$M -I. $M/freader.c
fallo.o: allo.o
	cc -c $(CFLAGS) -I$M -I. $M/fallo.c
fmatch.o: match.o
	cc -c $(CFLAGS) -I$M -I. $M/fmatch.c
ftable.o: table.o
	cc -c $(CFLAGS) -I$M -I. $M/ftable.c
forder.o: order.o
	cc -c $(CFLAGS) -I$M -I. $M/forder.c
flocal2.o: local2.o
	cc -c $(CFLAGS) -I$M -I. $M/flocal2.c
fcomm2.o: $M/common
	cc -c $(CFLAGS) -I$M -I. $M/fcomm2.c
fort.o freader.o fallo.o fmatch.o ftable.o forder.o flocal2.o fcomm2.o: $M/mfile2 $M/manifest macdefs mac2defs
install:
	cp comp $(INSTALL)

clean:
	rm -f *.o log

clobber:  clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info

.DEFAULT:
	sccs get $<
