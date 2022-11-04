# SCCSID: @(#)Makefile	3.0	(ULTRIX-11)	4/22/86

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	Based on @(#)Makefile	5.1 (Berkeley) 6/4/85
#
SOURCES= query.c trace.c
ALL=	../protocol.h ${SOURCES}
CFLAGS=	-O
DESTDIR=
LDFLAGS=

all:	query trace

# test programs and utilities
query:	query.c ../protocol.h
	${CC} -o query ${LDFLAGS} query.c

trace:	trace.c ../protocol.h
	${CC} -o trace ${LDFLAGS} trace.c

sources: ${ALL}

${SOURCES}:
	sccs get $@

../protocol.h:
	cd ..; sccs get protocol.h

install:
	@echo Nothing to install

clean:
	rm -f *.o core a.out t.c query linterrs trace typescript errs

clobber: clean sccsclean

sccsinfo:
	sccs info

sccsclean:
	sccs clean
tags:
	true
