# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
SOURCES = arc.c box.c circle.c close.c cont.c \
	dot.c erase.c label.c line.c linmod.c \
	move.c open.c point.c putsi.c space.c
OBJECTS = arc.o box.o circle.o close.o cont.o \
	dot.o erase.o label.o line.o linmod.o \
	move.o open.o point.o putsi.o space.o
ARCHIVE = libplot.a
CFLAGS = -O

all: ../${ARCHIVE}

../${ARCHIVE}: forceit ${OBJECTS}
	rm -f ../${ARCHIVE}
	ar cr ../${ARCHIVE} ${OBJECTS}
	ranlib ../${ARCHIVE}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: ../${ARCHIVE}
	cp ../${ARCHIVE} ${DESTDIR}/usr/lib/${ARCHIVE}
	chown bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chgrp bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/usr/lib/${ARCHIVE}
	ranlib ${DESTDIR}/usr/lib/${ARCHIVE}
	make clean

forceit:

clean:
	rm -f *.o ../${ARCHIVE} a.out core log

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
