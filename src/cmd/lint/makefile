# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
M=/usr/src/cmd/mip
CFLAGS = -O
SOURCES= lint.c SHELL llib-lc llib-lm llib-port lmanifest lpass2.c macdefs

MIPS= $M/cgram.y $M/comm1.c $M/common $M/makefile $M/manifest $M/mfile1 \
	$M/optim.c $M/pftn1.c $M/pftn2.c $M/scan.c $M/trees.c $M/xdefs.c

OBJ70 = cgram70.o xdefs70.o scan70.o comm170.o pftn170.o pftn270.o \
	trees70.o optim70.o lint70.o

OBJ40 = cgram40.o xdefs40.o scan40.o comm140.o pftn140.o pftn240.o \
	trees40.o optim40.o lint40.o

all:	head
	@echo DONE

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

cmp:	head
	cmp lpass140 ${DESTDIR}/usr/lib/lint140
	cmp lpass170 ${DESTDIR}/usr/lib/lint170
	cmp lpass2 ${DESTDIR}/usr/lib/lint2

head: lpass140 lpass170 lpass2

lint40: lpass140 lpass2

lint70: lpass170 lpass2

lpass2: lpass2.o
	cc -o lpass2 $(LFLAGS) lpass2.o

lpass2.o: $M/manifest lmanifest lpass2.c
	cc $(CFLAGS) -c -I$M -I. lpass2.c

lintall:	cgram.c
	lint -hpv -I. -I$M  cgram.c $M/xdefs.c $M/scan.c $M/pftn1.c $M/pftn2.c \
	$M/trees.c $M/optim.c lint.c $M/reader.c local2.c order.c \
	$M/match.c $M/allo.c $M/comm1.c table.c

install: head SHELL llib-lc llib-lm llib-port
	cp lpass140 ${DESTDIR}/usr/lib/lint140
	cp lpass170 ${DESTDIR}/usr/lib/lint170
	-strip ${DESTDIR}/usr/lib/lint140  ${DESTDIR}/usr/lib/lint170
	chmod 755 ${DESTDIR}/usr/lib/lint140  ${DESTDIR}/usr/lib/lint170
	chog bin ${DESTDIR}/usr/lib/lint140  ${DESTDIR}/usr/lib/lint170
	cp lpass170 ${DESTDIR}/usr/lib/lint1
	cp lpass2 ${DESTDIR}/usr/lib/lint2
	-strip ${DESTDIR}/usr/lib/lint1  ${DESTDIR}/usr/lib/lint2
	chmod 755 ${DESTDIR}/usr/lib/lint1  ${DESTDIR}/usr/lib/lint2
	chog bin ${DESTDIR}/usr/lib/lint1  ${DESTDIR}/usr/lib/lint2
	cp llib-lm llib-lc llib-port ${DESTDIR}/usr/lib
	chmod 644 ${DESTDIR}/usr/lib/llib-lm  ${DESTDIR}/usr/lib/llib-lc \
		${DESTDIR}/usr/lib/llib-port
	chog bin ${DESTDIR}/usr/lib/llib-lm  ${DESTDIR}/usr/lib/llib-lc\
		${DESTDIR}/usr/lib/llib-port
	cp SHELL ${DESTDIR}/usr/bin/lint
	chmod 755 ${DESTDIR}/usr/bin/lint
	chog bin ${DESTDIR}/usr/bin/lint
	make clean

lpass170: $(OBJ70)
	$(CC) -i -o lpass170 $(OBJ70)

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

lint70.o: $M/manifest macdefs $M/mfile1 lmanifest lint.c
	cc -c $(CFLAGS) -I$M -I. lint.c
	mv lint.o lint70.o

scan70.o: $M/manifest macdefs $M/mfile1 $M/scan.c
	cc -c $(CFLAGS) -I$M -I. $M/scan.c
	mv scan.o scan70.o

xdefs70.o: $M/manifest $M/mfile1 macdefs $M/xdefs.c
	cc -c $(CFLAGS) -I$M -I. $M/xdefs.c
	mv xdefs.o xdefs70.o

cgram70.o: $M/manifest $M/mfile1 macdefs cgram.c
	cc -c $(CFLAGS) -I$M -I. cgram.c
	mv cgram.o cgram70.o

comm170.o: $M/manifest $M/mfile1 $M/common macdefs $M/comm1.c
	cc -c $(CFLAGS) -I. -I$M $M/comm1.c
	mv comm1.o comm170.o

${MIPS}:
	(cd $M; make getl)

lpass140: $(OBJ40)
	ld -X -n /lib/crt0.o -o lpass140 \
	  -Z lint40.o \
	  -Z trees40.o xdefs40.o \
	  -Z cgram40.o optim40.o pftn240.o \
	  -Z pftn140.o \
	  -Z scan40.o comm140.o -L -lc

trees40.o: $M/manifest macdefs $M/mfile1 $M/trees.c
	cc -c $(CFLAGS) -V -I$M -I. $M/trees.c
	mv trees.o trees40.o

optim40.o: $M/manifest macdefs $M/mfile1 $M/optim.c
	cc -c $(CFLAGS) -V -I$M -I. $M/optim.c
	mv optim.o optim40.o

pftn140.o: $M/manifest macdefs $M/mfile1 $M/pftn1.c
	cc -c $(CFLAGS) -V -I$M -I. $M/pftn1.c
	mv pftn1.o pftn140.o

pftn240.o: $M/manifest macdefs $M/mfile1 $M/pftn2.c
	cc -c $(CFLAGS) -V -I$M -I. $M/pftn2.c
	mv pftn2.o pftn240.o

lint40.o: $M/manifest macdefs $M/mfile1 lmanifest lint.c
	cc -c $(CFLAGS) -V -I$M -I. lint.c
	mv lint.o lint40.o

scan40.o: $M/manifest macdefs $M/mfile1 $M/scan.c
	cc -c $(CFLAGS) -V -I$M -I. $M/scan.c
	mv scan.o scan40.o

xdefs40.o: $M/manifest $M/mfile1 macdefs $M/xdefs.c
	cc -c $(CFLAGS) -V -I$M -I. $M/xdefs.c
	mv xdefs.o xdefs40.o

cgram40.o: $M/manifest $M/mfile1 macdefs cgram.c
	cc -c $(CFLAGS) -V -I$M -I. cgram.c
	mv cgram.o cgram40.o

cgram.c: $M/cgram.y
	yacc $M/cgram.y
	mv y.tab.c cgram.c

comm140.o: $M/manifest $M/mfile1 $M/common macdefs $M/comm1.c
	cc -c $(CFLAGS) -V -I. -I$M $M/comm1.c
	mv comm1.o comm140.o

clean:
	rm -f *.o cgram.c log
	rm -f lpass140 lpass170 lpass2
	(cd $M; make clean)

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
