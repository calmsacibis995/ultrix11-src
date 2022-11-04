# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for the man command
SOURCES=man.c

OBJECTS=man.o

all:	man

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

man:	man.c
	cc -n -O man.c -o man

install: all
	cp man ${DESTDIR}/bin/man
	-strip ${DESTDIR}/bin/man
	chog bin ${DESTDIR}/bin/man
	chmod 755 ${DESTDIR}/bin/man
	rm -f ${DESTDIR}/bin/whatis ${DESTDIR}/bin/apropos
	ln ${DESTDIR}/bin/man ${DESTDIR}/bin/whatis
	ln ${DESTDIR}/bin/man ${DESTDIR}/bin/apropos
	-${DESTDIR}/usr/lib/makewhatis
	make clean

clean:
	rm -f *.o man a.out core log

clobber: clean
	sccs clean

tags:
	@true

sccsinfo:
	sccs info
