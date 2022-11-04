# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for liby.a
#
ULIBDIR=${DESTDIR}/usr/lib
LORDER=lorder

CC=cc
CFLAGS=

LIBOBJS=libmai.o libzer.o

all:     liby.a

liby.a: ${LIBOBJS}
	rm -f liby.a
	ar rv liby.a `$(LORDER) *.o | tsort`;

libmai.o:	libmai.c
		$(CC) -c $(CFLAGS) libmai.c

libzer.o:	libzer.c
		$(CC) -c $(CFLAGS) libzer.c

install:  all
	cp liby.a ${ULIBDIR}/liby.a
	ranlib  ${ULIBDIR}/liby.a
	chog bin ${ULIBDIR}/liby.a
	chmod 644 ${ULIBDIR}/liby.a
	make clean

clean:
	rm -f *.o log core a.out liby.a 

clobber: clean
	sccs clean

sccsinfo:
	sccs info
tags:
	true
