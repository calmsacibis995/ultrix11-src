# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

CFLAGS= -O -DUCB_NTTY
SOURCES= tset.c  tset.del.h

all:	tset

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

tset: tset.o
	cc -n -o tset tset.o -ltermlib

tset.o: tset.c tset.del.h
	cc ${CFLAGS} -c tset.c

install: all
	cp tset ${DESTDIR}/usr/ucb/tset
	-strip ${DESTDIR}/usr/ucb/tset
	chog bin ${DESTDIR}/usr/ucb/tset
	chmod 755 ${DESTDIR}/usr/ucb/tset
	make clean

clean:
	rm -f *.o tset

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
