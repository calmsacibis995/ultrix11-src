# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
SOURCES = arc.c box.c circle.c close.c con.h \
	dot.c erase.c label.c line.c linmod.c \
	move.c open.c point.c space.c subr.c
OBJECTS = arc.o box.o circle.o close.o \
	dot.o erase.o label.o line.o linmod.o \
	move.o open.o point.o space.o subr.o
ARCHIVE = libt300s.a
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

erase.o label.o line.o point.o space.o subr.o: con.h

forceit:

clean:
	rm -f *.o ../${ARCHIVE} a.out core log

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
