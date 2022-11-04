# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for new user account program (/etc/nu)

SOURCES= nu.c
#CFLAGS= -O -DDEBUG
CFLAGS= -O

all:	nu nu2

nu:	${SOURCES}
	cc ${CFLAGS} -o nu nu.c

# version of /etc/nu with ALIASES code turned on
nu2:	${SOURCES}
	cc ${CFLAGS} -o nu2 -DDBM nu.c -lv7 -ldbm

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

install: all
	cp nu.cf ${DESTDIR}/etc/nu.cf
	chmod 644 ${DESTDIR}/etc/nu.cf
	chog bin ${DESTDIR}/etc/nu.cf
	-(if [ ! -d ${DESTDIR}/etc/nulib ] ; \
	then \
		mkdir ${DESTDIR}/etc/nulib; \
	else \
		true; \
	fi)
	cp nu ${DESTDIR}/etc/nu
	-strip ${DESTDIR}/etc/nu
	chmod 744 ${DESTDIR}/etc/nu
	chog bin ${DESTDIR}/etc/nu
	cp nulib/nu?.sh ${DESTDIR}/etc/nulib
	chmod 744 ${DESTDIR}/etc/nulib/nu?.sh
	chog bin ${DESTDIR}/etc/nulib/nu?.sh
	make clean
	@(echo "Make sure you edit ${DESTDIR}/etc/nu.cf for your site")

clean:
	rm -f *.o nu core log

clobber: clean
	sccs clean

tags:	sources
	ctags ${SOURCES}

sccsinfo:
	sccs info
