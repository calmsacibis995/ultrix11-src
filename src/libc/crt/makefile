# SCCSID: @(#)makefile	3.0	(ULTRIX-11)	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = aldiv.s almul.s alrem.s auldiv.s aulrem.s cerror.s \
	ldiv.s lmul.s lrem.s  mcount.s csv.s ovhndlr.s \
	udiv.s uldiv.s ulrem.s

OBJECTS = aldiv.o almul.o alrem.o auldiv.o aulrem.o cerror.o \
	ldiv.o lmul.o lrem.o  mcount.o csv.o ovhndlr.o \
	udiv.o uldiv.o ulrem.o

CFLAGS = -O

all: ${OBJECTS}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

archive: ${OBJECTS}
	ar r ../${LIB} ${OBJECTS}

clean:
	rm -f *.o

clobber:
	rm -f *.o; sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
