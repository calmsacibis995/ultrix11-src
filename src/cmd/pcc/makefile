# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
M=/usr/src/cmd/mip
CFLAGS=-O
OBJ70 =  cgram70.o xdefs70.o scan70.o pftn170.o pftn270.o trees70.o \
	optim70.o code70.o local70.o reader70.o local270.o order70.o \
	match70.o allo70.o comm170.o table70.o
OBJ140 = code40.o scan40.o comm140.o xdefs40.o cgram40.o pftn140.o \
	pftn240.o local40.o trees40.o optim40.o malloc.o
OBJ240 = local240.o order40.o reader40.o match40.o allo40.o table40.o comm240.o
FOBJ = fort.o freader.o fallo.o fmatch.o ftable.o forder.o flocal2.o fcomm2.o
GETP =	$M/cgram.y $M/xdefs.c $M/scan.c $M/pftn1.c $M/pftn2.c $M/trees.c \
	$M/optim.c $M/reader.c $M/match.c $M/allo.c $M/comm1.c $M/mfile2 \
	$M/manifest $M/mfile1 $M/common $M/comm2.c

SOURCES= SHELL SHELL40 macdefs mac2defs order.c local2.c code.c local.c\
	table.c

all:	ccom70 ccom140 ccom240

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

head:  ccom70 ccom140 ccom240 ;

install: ccom70 ccom140 ccom240 SHELL40 SHELL
	cp ccom70 ${DESTDIR}/usr/lib/ccom
	cp ccom70 ${DESTDIR}/usr/lib/ccom70
	-strip ${DESTDIR}/usr/lib/ccom ${DESTDIR}/usr/lib/ccom70
	chmod 755 ${DESTDIR}/usr/lib/ccom ${DESTDIR}/usr/lib/ccom70
	chog bin ${DESTDIR}/usr/lib/ccom ${DESTDIR}/usr/lib/ccom70
	cp ccom140 ccom240 ${DESTDIR}/usr/lib
	cp ccom140 ${DESTDIR}/usr/lib/ccom1
	cp ccom240 ${DESTDIR}/usr/lib/ccom2
	-strip ${DESTDIR}/usr/lib/ccom140 ${DESTDIR}/usr/lib/ccom240
	-strip ${DESTDIR}/usr/lib/ccom1 ${DESTDIR}/usr/lib/ccom2
	chmod 755 ${DESTDIR}/usr/lib/ccom140 ${DESTDIR}/usr/lib/ccom240
	chmod 755 ${DESTDIR}/usr/lib/ccom1 ${DESTDIR}/usr/lib/ccom2
	chog bin ${DESTDIR}/usr/lib/ccom140 ${DESTDIR}/usr/lib/ccom240
	chog bin ${DESTDIR}/usr/lib/ccom1 ${DESTDIR}/usr/lib/ccom2
	cp SHELL ${DESTDIR}/usr/bin/pcc
	cp SHELL ${DESTDIR}/usr/bin/pcc70
	cp SHELL40 ${DESTDIR}/usr/bin/pcc40
	chmod 755 ${DESTDIR}/usr/bin/pcc ${DESTDIR}/usr/bin/pcc40 ${DESTDIR}/usr/bin/pcc70
	chog bin ${DESTDIR}/usr/bin/pcc ${DESTDIR}/usr/bin/pcc40 ${DESTDIR}/usr/bin/pcc70
	make clean

cmp: ccom70 ccom140 ccom240
	cmp ccom70 ${DESTDIR}/usr/lib/ccom
	cmp ccom140 ${DESTDIR}/usr/lib/ccom1
	cmp ccom240 ${DESTDIR}/usr/lib/ccom2

${GETP}:
	(cd $M; make getp DESTDIR=${DESTDIR})

ccom70: $(OBJ70)
	cc -i -o ccom70 $(OBJ70)

ccom140: $(OBJ140)
	ld -X -n -o ccom140 /lib/crt0.o \
		-Z pftn140.o \
		-Z trees40.o \
		-Z cgram40.o local40.o xdefs40.o \
		-Z scan40.o pftn240.o optim40.o \
		-Z comm140.o code40.o malloc.o \
		-L -lc

ccom240: $(OBJ240)
	cc -n -o ccom240 $(OBJ240)

malloc.o: /lib/libc.a
	ar x /lib/libc.a malloc.o

trees70.o: $M/manifest macdefs $M/mfile1 $M/trees.c
	cc -c $(CFLAGS) -I$M -I. $M/trees.c
	mv trees.o trees70.o

optim70.o: $M/manifest macdefs $M/mfile1 $M/optim.c
	cc -c $(CFLAGS) -I$M -I. $M/optim.c
	mv optim.o optim70.o

pftn170.o: $M/manifest macdefs $M/mfile1 $M/pftn1.c
	cc -c $(CFLAGS) -I$M -I. $M/pftn1.c
	mv pftn1.o pftn170.o

pftn270.o: $M/manifest macdefs $M/mfile1 $M/pftn2.c
	cc -c $(CFLAGS) -I$M -I. $M/pftn2.c
	mv pftn2.o pftn270.o

code70.o: $M/manifest macdefs $M/mfile1 code.c
	cc -c $(CFLAGS) -I$M -I. code.c
	mv code.o code70.o

local70.o: $M/manifest macdefs $M/mfile1 local.c
	cc -c $(CFLAGS) -I$M -I. local.c
	mv local.o local70.o

scan70.o: $M/manifest macdefs $M/mfile1 $M/scan.c
	cc -c $(CFLAGS) -I$M -I. $M/scan.c
	mv scan.o scan70.o

xdefs70.o: $M/manifest $M/mfile1 macdefs $M/xdefs.c
	cc -c $(CFLAGS) -I$M -I. $M/xdefs.c
	mv xdefs.o xdefs70.o

cgram70.o: $M/manifest $M/mfile1 macdefs cgram.c
	cc -c $(CFLAGS) -I$M -I. cgram.c
	mv cgram.o cgram70.o

cgram.c: $M/cgram.y
	yacc $M/cgram.y
	mv y.tab.c cgram.c

comm170.o: $M/manifest $M/mfile1 $M/common macdefs $M/comm1.c
	cc -c $(CFLAGS) -I. -I$M $M/comm1.c
	mv comm1.o comm170.o

table70.o: $M/manifest $M/mfile2 mac2defs macdefs table.c
	cc -c $(CFLAGS) -I$M -I. table.c
	mv table.o table70.o

reader70.o: $M/manifest $M/mfile2 mac2defs macdefs $M/reader.c
	cc -c $(CFLAGS) -I$M -I. $M/reader.c
	mv reader.o reader70.o

local270.o: $M/manifest $M/mfile2 mac2defs macdefs local2.c
	cc -c $(CFLAGS) -I$M -I. local2.c
	mv local2.o local270.o

order70.o: $M/manifest $M/mfile2 mac2defs macdefs order.c
	cc -c $(CFLAGS) -I$M -I. order.c
	mv order.o order70.o

match70.o: $M/manifest $M/mfile2 mac2defs macdefs $M/match.c
	cc -c $(CFLAGS) -I$M -I. $M/match.c
	mv match.o match70.o

allo70.o: $M/manifest $M/mfile2 mac2defs macdefs $M/allo.c
	cc -c $(CFLAGS) -I$M -I. $M/allo.c
	mv allo.o allo70.o

trees40.o: $M/manifest macdefs $M/mfile1 $M/trees.c
	cc -c -V $(CFLAGS) -DPCC40 -I$M -I. $M/trees.c
	mv trees.o trees40.o

optim40.o: $M/manifest macdefs $M/mfile1 $M/optim.c
	cc -c -V $(CFLAGS) -DPCC40 -I$M -I. $M/optim.c
	mv optim.o optim40.o

pftn140.o: $M/manifest macdefs $M/mfile1 $M/pftn1.c
	cc -c -V $(CFLAGS) -DPCC40 -I$M -I. $M/pftn1.c
	mv pftn1.o pftn140.o

pftn240.o: $M/manifest macdefs $M/mfile1 $M/pftn2.c
	cc -c -V $(CFLAGS) -DPCC40 -I$M -I. $M/pftn2.c
	mv pftn2.o pftn240.o

code40.o: $M/manifest macdefs $M/mfile1 code.c
	cc -c -V $(CFLAGS) -DPCC40 -I$M -I. code.c
	mv code.o code40.o

local40.o: $M/manifest macdefs $M/mfile1 local.c
	cc -c -V $(CFLAGS) -DPCC40 -I$M -I. local.c
	mv local.o local40.o

scan40.o: $M/manifest macdefs $M/mfile1 $M/scan.c
	cc -c -V $(CFLAGS) -DPCC40 -I$M -I. $M/scan.c
	mv scan.o scan40.o

xdefs40.o: $M/manifest $M/mfile1 macdefs $M/xdefs.c
	cc -c -V $(CFLAGS) -DPCC40 -I$M -I. $M/xdefs.c
	mv xdefs.o xdefs40.o

cgram40.o: $M/manifest $M/mfile1 macdefs cgram.c
	cc -c -V $(CFLAGS) -DPCC40 -I$M -I. cgram.c
	mv cgram.o cgram40.o

comm140.o: $M/manifest $M/mfile1 $M/common macdefs $M/comm1.c
	cc -c -V $(CFLAGS) -DPCC40 -I. -I$M $M/comm1.c
	mv comm1.o comm140.o

comm240.o: $M/manifest $M/mfile2 $M/common macdefs $M/comm2.c
	cc -c $(CFLAGS) -DPCC40 -I. -I$M $M/comm2.c
	mv comm2.o comm240.o

table40.o: $M/manifest $M/mfile2 mac2defs macdefs table.c
	cc -c $(CFLAGS) -DPCC40 -I$M -I. table.c
	mv table.o table40.o

reader40.o: $M/manifest $M/mfile2 mac2defs macdefs $M/reader.c
	cc -c $(CFLAGS) -DPCC40 -I$M -I. $M/reader.c
	mv reader.o reader40.o

local240.o: $M/manifest $M/mfile2 mac2defs macdefs local2.c
	cc -c $(CFLAGS) -DPCC40 -I$M -I. local2.c
	mv local2.o local240.o

order40.o: $M/manifest $M/mfile2 mac2defs macdefs order.c
	cc -c $(CFLAGS) -DPCC40 -I$M -I. order.c
	mv order.o order40.o

match40.o: $M/manifest $M/mfile2 mac2defs macdefs $M/match.c
	cc -c $(CFLAGS) -DPCC40 -I$M -I. $M/match.c
	mv match.o match40.o

allo40.o: $M/manifest $M/mfile2 mac2defs macdefs $M/allo.c
	cc -c $(CFLAGS) -DPCC40 -I$M -I. $M/allo.c
	mv allo.o allo40.o

lintall:
	lint -hpv -I. -I$M  cgram.c xdefs.c $M/scan.c $M/pftn.c $M/trees.c \
	$M/optim.c code.c local.c $M/reader.c local2.c order.c $M/match.c \
	$M/allo.c $M/comm1.c table.c

fort: ccom $(FOBJ)
	cc -i $(CFLAGS) $(FOBJ)
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

$(FOBJ): $M/mfile2 $M/manifest macdefs mac2defs

clean:
	rm -f *.o cgram.c
	rm -f ccom70 ccom140 ccom240 log

clobber: clean
	(cd $M; sccs clean)
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
