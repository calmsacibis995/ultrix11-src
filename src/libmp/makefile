# SCCSID="@(#)makefile	3.0	4/22/86"
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = gcd.c madd.c mdiv.c mout.c msqrt.c mult.c pow.c util.c
OBJECTS = gcd.o madd.o mdiv.o mout.o msqrt.o mult.o pow.o util.o
ARCHIVE = libmp.a
CFLAGS = -O

all: ${ARCHIVE}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: ${ARCHIVE}
	cp ${ARCHIVE} ${DESTDIR}/usr/lib
	chog bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/usr/lib/${ARCHIVE}
	ranlib ${DESTDIR}/usr/lib/${ARCHIVE}
	make clean

${ARCHIVE}: forceit ${OBJECTS}
	rm -f ${ARCHIVE}
	ar rc ${ARCHIVE} ${OBJECTS}
	ranlib ${ARCHIVE}

forceit:

clean:
	rm -f ${ARCHIVE} *.o log a.out core

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
