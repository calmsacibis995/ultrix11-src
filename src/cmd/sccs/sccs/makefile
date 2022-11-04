# SCCSID @(#)makefile 3.0 4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS=-O
# LIBNDIR= -lndir  	libndir routines are now a part of libc.a
LIBNDIR=

SOURCES= sccs.c

all:	sccs

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	cp sccs ${DESTDIR}/usr/bin/sccs
	-strip ${DESTDIR}/usr/bin/sccs
	chmod 4755 ${DESTDIR}/usr/bin/sccs
	chog root ${DESTDIR}/usr/bin/sccs
	make clean

sccs: sccs.c sccs.o
	cc -n -o sccs sccs.o ${LIBNDIR}

#${LIBNDIR}:
#	@echo Making ${LIBNDIR}
#	(cd ../dir; make libndir)

clean:
	rm -f *.o sccs

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
