# SCCSID: @(#)makefile	3.1	10/15/87
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
SOURCES=dc.c dc.h

all:	dc

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

cmp:	all
	cmp dc ${DESTDIR}/usr/bin/dc

install: all
	cp dc ${DESTDIR}/usr/bin/dc
	-strip ${DESTDIR}/usr/bin/dc 
	chmod 755 ${DESTDIR}/usr/bin/dc 
	chog bin ${DESTDIR}/usr/bin/dc 
	make clean

dc:	${SOURCES}
	cc -n -O dc.c -o dc

clean:
	rm -f *.o log dc

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
