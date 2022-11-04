# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS = -n -O

SOURCES= sed0.c sed1.c sed.h

all:	sed

${SOURCES}:
	sccs get $@

sources: ${SOURCES}
	
install:	all
	cp sed ${DESTDIR}/bin/sed
	-strip ${DESTDIR}/bin/sed
	chmod 755 ${DESTDIR}/bin/sed
	chog bin ${DESTDIR}/bin/sed
	make clean

sed:	sed0.o sed1.o
	cc -o sed -n -O sed0.o sed1.o

cmp:	all
	cmp sed ${DESTDIR}/bin/sed

sed0.o: sed0.c sed.h
sed1.o: sed1.c sed.h

clean:
	rm -f *.o log sed

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
