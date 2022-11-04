# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
SOURCES = opser.c shutdown.c restart
BAKS = daily.bak m11_day.bak m11_week.bak monthly.bak tk_daily.bak
CFLAGS= -O

all: opser shutdown

sources: ${SOURCES}

${SOURCES} ${BAKS}:
	sccs get $@

cmp: all
	cmp opser ${DESTDIR}/opr/opser
	cmp shutdown ${DESTDIR}/opr/shutdown
	rm opser
	rm shutdown

install: all ${BAKS}
	cp opser ${DESTDIR}/opr/opser
	-strip ${DESTDIR}/opr/opser
	cp shutdown ${DESTDIR}/opr/shutdown
	-strip ${DESTDIR}/opr/shutdown
	cp restart ${DESTDIR}/opr/restart
	cp ${BAKS} ${DESTDIR}/opr
	(cd ${DESTDIR}/opr; chmod 700 opser shutdown restart ${BAKS})
	(cd ${DESTDIR}/opr; chog root opser shutdown restart ${BAKS})
	make clean

opser: opser.c
	${CC} ${CFLAGS} opser.c -o opser -ljobs

shutdown: shutdown.c
	${CC} ${CFLAGS} shutdown.c -o shutdown

clean:
	rm -f *.o core a.out log opser shutdown

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
