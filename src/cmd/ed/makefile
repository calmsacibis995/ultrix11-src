# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS = -O
LDFLAGS = -n
# CRYPTIN = -DCRYPT for encryption
CRYPTIN=
SOURCES = ed.c ed_v7.c edfun regexp.h

all:	ed ed_v7

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

ed:	ed.s
	${CC} ${LDFLAGS} -o ed ed.s

ed.s:	ed.c regexp.h edfun
	${CC} -S ${CFLAGS} ${CRYPTIN} ed.c
# if pdp11 put error message strings in text space
	sh ./edfun ed.s

ed_v7:	ed_v7.c
	${CC} ${LDFLAGS} ${CFLAGS} -o ed_v7 ed_v7.c
	
install: all
	cp ed ${DESTDIR}/bin/ed
	-strip ${DESTDIR}/bin/ed
	chog bin ${DESTDIR}/bin/ed
	chmod 755 ${DESTDIR}/bin/ed

	cp ed_v7 ${DESTDIR}/usr/orphan/usr/bin/ed_v7
	-strip ${DESTDIR}/usr/orphan/usr/bin/ed_v7
	chog bin ${DESTDIR}/usr/orphan/usr/bin/ed_v7
	chmod 755 ${DESTDIR}/usr/orphan/usr/bin/ed_v7
	make clean

clean:
	  rm -f ed.s *.o ed ed_v7

clobber:  clean
	  sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
