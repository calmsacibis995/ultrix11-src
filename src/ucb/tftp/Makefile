
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

# SCCSID: @(#)Makefile	3.0	4/22/86
# Based on: @(#)Makefile	1.4	ULTRIX-32	12/4/84
#
# Makefile 6/12/83
SOURCES = main.c tftp.c
CFLAGS=-O

all:	tftp

tftp:	main.o tftp.o
	${CC} main.o tftp.o -o tftp -ljobs

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: Install clean

Install: tftp
	cp tftp ${DESTDIR}/usr/ucb/tftp
	strip ${DESTDIR}/usr/ucb/tftp
	chmod 755 ${DESTDIR}/usr/ucb/tftp
	chog bin ${DESTDIR}/usr/ucb/tftp

clean:
	rm -f tftp *.o *.s errs core a.out t.?

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
