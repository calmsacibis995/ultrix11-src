# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for lprsetup program
#

SOURCES= lprsetup.c globals.h misc.c printlib.c

OBJS= lprsetup.o misc.o printlib.o

#OPTIONS= -DDEBUG
OPTIONS=

CFLAGS= -O ${OPTIONS}

all:	lprsetup

lprsetup: ${OBJS}
	cc ${CFLAGS} ${OBJS} -o lprsetup

lprsetup.o: lprsetup.h globals.h
	cc -c ${CFLAGS} lprsetup.c

misc.o: lprsetup.h globals.h
	cc -c ${CFLAGS} misc.c

install: all
	cp lprsetup ${DESTDIR}/usr/etc/lprsetup
	-strip ${DESTDIR}/usr/etc/lprsetup
	chog bin ${DESTDIR}/usr/etc/lprsetup
	chmod 755 ${DESTDIR}/usr/etc/lprsetup
	make clean

clean:
	rm -f lprsetup *.o a.out core log

clobber:
	sccs clean

tags: ${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
