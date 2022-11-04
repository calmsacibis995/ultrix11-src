# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS=	-O -DCM_N -DCM_GT -DCM_B -DCM_D

SOURCES = termcap.c tgoto.c tputs.c
MISC = tc1.c tc2.c tc3.c
OBJECTS = termcap.o tgoto.o tputs.o
ARCHIVE = libtermlib.a

all: ${ARCHIVE}

${ARCHIVE}: forceit ${OBJECTS}
	rm -f ${ARCHIVE}
	ar cr ${ARCHIVE} ${OBJECTS}
	ranlib ${ARCHIVE}

sources: ${SOURCES} ${MISC}

${SOURCES} ${MISC}:
	sccs get $@

forceit:

clean:
	rm -f *.o ${ARCHIVE} a.out core log

clobber: clean
	sccs clean

install: all
	cp ${ARCHIVE} ${DESTDIR}/usr/lib
	chown bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chgrp bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/usr/lib/${ARCHIVE}
	ranlib ${DESTDIR}/usr/lib/${ARCHIVE}
	make clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
