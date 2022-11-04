# SCCSID="@(#)makefile	3.0	4/21/86"
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for more command
TLIB= -ltermlib
CFLAGS= -O
OBJS= more.o regex.o
SOURCES= more.c regex.c more.help

all:	more

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

more: ${OBJS} ${SOURCES}
	cc -o more -n ${OBJS} ${TLIB}

install: all
	cp more ${DESTDIR}/usr/bin/more
	-strip ${DESTDIR}/usr/bin/more
	chmod 755 ${DESTDIR}/usr/bin/more 
	chog bin ${DESTDIR}/usr/bin/more
	rm -f ${DESTDIR}/usr/ucb/more
	rm -f ${DESTDIR}/usr/ucb/page
	ln ${DESTDIR}/usr/bin/more ${DESTDIR}/usr/ucb/more
	ln ${DESTDIR}/usr/bin/more ${DESTDIR}/usr/ucb/page
	cp more.help ${DESTDIR}/usr/lib/more.help
	chmod 644 ${DESTDIR}/usr/lib/more.help
	chog bin ${DESTDIR}/usr/lib/more.help
	make clean

clean:
	rm -f ${OBJS}
	rm -f more

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
