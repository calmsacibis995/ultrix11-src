# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#
#  This makefile creates the System V "C" library, libcV.a.
#  It contains routines which clash with ULTRIX-11 libraries.
#  These objects are archived in their own library which gets
#  loaded first, when compiled using the System V option.
#  (either "cc -Y prog.c" , or "setenv PROG_ENV SYSTEM_FIVE")
#
#  -Y flag below forces compilation using "-DSYSTEM_FIVE" (by /bin/cc).
#  This is because anyone using this library in the first
#  place will have specified the System V interface.

CFLAGS = -Y -O

LIBNAME = libcV.a

SOURCES = ctime.c ttyslot.c printf.c fprintf.c sprintf.c

OBJECTS = ctime.o ttyslot.o printf.o fprintf.o sprintf.o

all: archive

archive: ${OBJECTS}
	rm -f ${LIBNAME}
	ar r ${LIBNAME} ${OBJECTS}

install: all
	cp ${LIBNAME} ${DESTDIR}/lib/${LIBNAME}
	ranlib ${DESTDIR}/lib/${LIBNAME}
	chog bin ${DESTDIR}/lib/${LIBNAME}
	chmod 644 ${DESTDIR}/lib/${LIBNAME}
	make clean

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

clean:
	rm -f ${LIBNAME}
	rm -f *.o log a.out core

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
