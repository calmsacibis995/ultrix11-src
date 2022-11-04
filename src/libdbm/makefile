# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
SOURCES = dbm.c dbm.h
ARCHIVE = libdbm.a

all: ${ARCHIVE}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	cp ${ARCHIVE} ${DESTDIR}/usr/lib/${ARCHIVE}
	chown bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chgrp bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/usr/lib/${ARCHIVE}
	make clean

libdbm.a: dbm.c dbm.h
	${CC} -O -c dbm.c
	mv dbm.o libdbm.a

clean:
	rm -f ${ARCHIVE} *.o a.out core log

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
