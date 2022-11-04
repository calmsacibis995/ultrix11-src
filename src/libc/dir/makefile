# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES= closedir.c opendir.c readdir.c seekdir.c telldir.c

OBJECTS= closedir.o opendir.o readdir.o seekdir.o telldir.o

CFLAGS=	-O

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

sccsinfo:
	sccs info
tags:
	true
