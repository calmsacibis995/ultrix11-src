# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
SOURCES = fp1.s fp2.s fp3.s fpx.s
ARCHIVE = libfpsim.a

all: ${ARCHIVE}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

fpsim.o: ${SOURCES}
	${AS} -o fpsim.o ${SOURCES}

${ARCHIVE}: forceit fpsim.o
	rm -f ${ARCHIVE}
	ar r ${ARCHIVE} fpsim.o

install: ${ARCHIVE}
	cp ${ARCHIVE} ${DESTDIR}/lib/${ARCHIVE}
	chown bin ${DESTDIR}/lib/${ARCHIVE}
	chgrp bin ${DESTDIR}/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/lib/${ARCHIVE}
	make clean

forceit:

clean:
	rm -f ${ARCHIVE} fpsim.o a.out core log

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
