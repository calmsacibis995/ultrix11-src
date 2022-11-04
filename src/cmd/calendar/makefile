# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES= calendar.sh calendar.c

all:	calendar calendar.sh

sources:	${SOURCES}

${SOURCES}:
		sccs get $@

calendar:	calendar.c
	cc -n -o calendar -O calendar.c

install:	all
	cp calendar.sh ${DESTDIR}/bin/calendar
	chmod 755 ${DESTDIR}/bin/calendar
	chog bin ${DESTDIR}/bin/calendar
	cp calendar ${DESTDIR}/usr/lib/calendar
	-strip ${DESTDIR}/usr/lib/calendar
	chmod 755 ${DESTDIR}/usr/lib/calendar
	chog bin ${DESTDIR}/usr/lib/calendar
	make clean

clean:
	rm -f *.o calendar

clobber:	clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
