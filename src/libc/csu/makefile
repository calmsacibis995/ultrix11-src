# SCCSID: @(#)makefile	3.0	(ULTRIX-11)	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = crt0.s fcrt0.s fmcrt0.s mcrt0.s v7csv.s

OBJECTS = crt0.o fcrt0.o fmcrt0.o mcrt0.o v7csv.o

all: ${OBJECTS}

sources: ${SOURCES}

install: ${OBJECTS}
	cp ${OBJECTS} ${DESTDIR}/lib
	(cd ${DESTDIR}/lib; chown bin ${OBJECTS}; \
		chgrp bin ${OBJECTS}; chmod 664 ${OBJECTS})

${SOURCES}:
	sccs get $@

clean:
	rm -f *.o log

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
