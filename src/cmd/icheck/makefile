# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS= -O
SOURCES= icheck.c

all:	icheck

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

icheck:	icheck70 icheck40

icheck40:	icheck.c
	cc -O icheck.c -o icheck40

icheck70:	icheck.c
	cc -i -O icheck.c -o icheck70

install: all
	rm -f ${DESTDIR}/bin/sid/icheck ${DESTDIR}/bin/nsid/icheck
	cp icheck70 ${DESTDIR}/bin/sid/icheck
	cp icheck40 ${DESTDIR}/bin/nsid/icheck
	-strip ${DESTDIR}/bin/sid/icheck ${DESTDIR}/bin/nsid/icheck
	chmod 755 ${DESTDIR}/bin/sid/icheck ${DESTDIR}/bin/nsid/icheck
	chog bin ${DESTDIR}/bin/sid/icheck ${DESTDIR}/bin/nsid/icheck
	-(if [ -f ${DESTDIR}/bin/icheck ] ;\
	then \
		rm -f ${DESTDIR}/bin/icheck ;\
	else \
		true ;\
	fi)
	ln ${DESTDIR}/bin/sid/icheck ${DESTDIR}/bin/icheck
	make clean

clean:
	rm -f *.o
	rm -f icheck40 icheck70 icheck

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
