# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
SOURCES = arc.c box.c ccurve.c circle.c close.c \
	color.c crevid.c curve.c dot.c erase.c \
	frame.c hcopy.c label.c line.c linemod.c \
	move.c movscr.c offsets.c ofmove.c open.c \
	pen.c point.c rescur.c savcur.c slimit.c \
	space.c srevid.c subr.c time.c wrtng.c
OBJECTS = arc.o box.o ccurve.o circle.o close.o \
	color.o crevid.o curve.o dot.o erase.o \
	frame.o hcopy.o label.o line.o linemod.o \
	move.o movscr.o offsets.o ofmove.o open.o \
	pen.o point.o rescur.o savcur.o slimit.o \
	space.o srevid.o subr.o time.o wrtng.o
ARCHIVE = libtgigi.a
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
