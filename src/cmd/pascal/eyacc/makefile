# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

CFLAGS = -O

SOURCES = ey0.c ey1.c ey2.c ey3.c ey4.c ey5.c

OBJS = ey0.o ey1.o ey2.o ey3.o ey4.o ey5.o

all:	eyacc
	-strip eyacc

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

eyacc:	${OBJS}
	cc ${CFLAGS} -i -o eyacc ey0.o ey1.o ey2.o ey3.o ey4.o ey5.o

install: all

clean:
	rm -f *.o

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info

ey1.o:	ey.h
ey2.o:	ey.h
ey3.o:	ey.h
ey4.o:	ey.h
