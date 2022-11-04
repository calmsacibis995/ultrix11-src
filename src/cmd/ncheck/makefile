# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS= -O
SOURCES= ncheck.c

all:	ncheck

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

ncheck: ncheck70 ncheck40

ncheck40:	ncheck.c
	cc -O -DNCHECK40 ncheck.c -o ncheck40

ncheck70:	ncheck.c
	cc -i -O ncheck.c -o ncheck70

install: all
	rm -f ${DESTDIR}/bin/sid/ncheck ${DESTDIR}/bin/nsid/ncheck
	cp ncheck70 ${DESTDIR}/bin/sid/ncheck
	cp ncheck40 ${DESTDIR}/bin/nsid/ncheck
	-strip ${DESTDIR}/bin/sid/ncheck ${DESTDIR}/bin/nsid/ncheck
	chmod 755 ${DESTDIR}/bin/sid/ncheck ${DESTDIR}/bin/nsid/ncheck
	chog bin ${DESTDIR}/bin/sid/ncheck ${DESTDIR}/bin/nsid/ncheck
	-(if [ -f ${DESTDIR}/bin/ncheck ] ;\
	then \
		rm -f ${DESTDIR}/bin/ncheck ;\
	else \
		true ;\
	fi)
	ln ${DESTDIR}/bin/sid/ncheck ${DESTDIR}/bin/ncheck
	make clean
clean:
	rm -f *.o
	rm -f ncheck40 ncheck70 ncheck

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
