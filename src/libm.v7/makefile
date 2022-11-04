# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = asin.c atan.c hypot.c jn.c j0.c j1.c \
	pow.c fabs.c log.c  sin.c sqrt.c tan.c \
	tanh.c sinh.c exp.c floor.c

OBJECTS = asin.o atan.o hypot.o jn.o j0.o j1.o \
	pow.o fabs.o log.o  sin.o sqrt.o tan.o \
	tanh.o sinh.o exp.o floor.o

ARCHIVE = libm.a

CFLAGS = -O

all: ${ARCHIVE}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

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
