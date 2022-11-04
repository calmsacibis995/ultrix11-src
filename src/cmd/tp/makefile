# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS=-O
CFILES = tp0.c tp1.c tp2.c tp3.c
HFILES = tp.h
SOURCES = ${HFILES} ${CFILES}


all:	tp

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: tp
	cp tp ${DESTDIR}/bin/tp
	-strip ${DESTDIR}/bin/tp
	chmod 755 ${DESTDIR}/bin/tp
	chog bin ${DESTDIR}/bin/tp
	rm -f *.o

cmp:	tp
	cmp tp /bin/tp

tp:	tp0.o tp1.o tp2.o tp3.o
	cc $(CFLAGS) -n tp0.o tp1.o tp2.o tp3.o -o tp

${CFILES}: ${HFILES}

clean:
	rm -f *.o
	rm -f tp

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
