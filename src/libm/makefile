# SCCSID="@(#)makefile	3.0	4/22/86"
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for System V libm.a
#
# Note that the object list given in OBJECTS is in the order
# in which the files should be added to the archive when it is
# created.  If the files are not ordered correctly, linking of
# libm fails on the pdp11.

SOURCES = asin.c atan.c erf.c fabs.c floor.c fmod.c \
	gamma.c hypot.c jn.c j0.c j1.c pow.c log.c \
	sin.c sinh.c sqrt.c tan.c tanh.c exp.c matherr.c ranm.s f77ranm.s

OBJECTS = asin.o atan.o erf.o fabs.o floor.o fmod.o \
	gamma.o hypot.o jn.o j0.o j1.o pow.o log.o \
	sin.o sinh.o sqrt.o tan.o tanh.o exp.o matherr.o ranm.o

ARCHIVE = libm.a

CFLAGS = -O

all: ${ARCHIVE}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

# have to special case this one
ranm.o:	ranm.s f77ranm.s
	cat f77ranm.s ranm.s > tmp.s
	as -o ranm.o tmp.s
	rm tmp.s

install: ${ARCHIVE}
	cp ${ARCHIVE} ${DESTDIR}/lib
	chown bin ${DESTDIR}/lib/${ARCHIVE}
	chgrp bin ${DESTDIR}/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/lib/${ARCHIVE}
	ranlib ${DESTDIR}/lib/${ARCHIVE}
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
