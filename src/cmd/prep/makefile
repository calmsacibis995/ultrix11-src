# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS = -n -O

SOURCES= prep0.c prep1.c prep2.c prep.h

all:	prep

sources:	${SOURCES}

${SOURCES}:
	sccs get $@

install:	all
	cp prep ${DESTDIR}/bin/prep
	-strip ${DESTDIR}/bin/prep
	chmod 755 ${DESTDIR}/bin/prep
	chog bin ${DESTDIR}/bin/prep
	make clean

cmp:	all
	cmp prep ${DESTDIR}/bin/prep

prep:	prep0.o prep1.o prep2.o; cc -o prep -n *.o

prep0.o: prep0.c prep.h
prep1.o: prep1.c prep.h
prep2.o: prep2.c

clean:
	rm -f *.o prep

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
