# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES= roff1.s roff2.s roff3.s roff4.s roff5.s roff7.s roff8.s

all:	roff

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

install:	roff
	cp roff ${DESTDIR}/usr/bin/roff
	-strip ${DESTDIR}/usr/bin/roff
	chmod 755 ${DESTDIR}/usr/bin/roff
	chog bin ${DESTDIR}/usr/bin/roff
	make clean

cmp:	roff
	cmp roff ${DESTDIR}/usr/bin/roff

roff:	${SOURCES} /usr/include/sys.s
	as -o roff.o /usr/include/sys.s ${SOURCES}
	ld roff.o -lc -o roff

clean:
	rm -f *.o log roff

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
