# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for bc and /usr/lib/lib.b

SOURCES= bc.y

all:	bc lib.b

bc: ${SOURCES}
	yacc bc.y
	mv y.tab.c bc.c
	cc -n -O bc.c -o bc
	rm bc.c

lib.b:
	sccs get lib.b

${SOURCES}:
	sccs get $@

install: all
	cp bc ${DESTDIR}/usr/bin/bc
	-strip ${DESTDIR}/usr/bin/bc
	chog bin ${DESTDIR}/usr/bin/bc
	chmod 755 ${DESTDIR}/usr/bin/bc

	cp lib.b ${DESTDIR}/usr/lib/lib.b
	chog bin ${DESTDIR}/usr/lib/lib.b
	chmod 644 ${DESTDIR}/usr/lib/lib.b
	make clean

clean:	forceit
	rm -f *.o bc bc.c log

clobber: clean
	sccs clean

sccsinfo:
	sccs info
tags:
	true

forceit:
