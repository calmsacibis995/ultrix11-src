#	SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	nroff terminal driving tables make file

TFILES = code.300 code.lp tab2631.c tab2631-c.c tab2631-e.c tab300.c \
	 tab300-12.c tab300s.c tab300s-12.c tab37.c tab382.c \
	 tab4000a.c tab450.c tab450-12.c tab832.c taba1.c tablp.c tabtn300.c \
	 tabln01.c tabln03.c tablqp02.c tablqp02-12.c \
	 tabX.c
PFILES = maketerms.c terms.hd
FILES = tab2631 tab2631-c tab2631-e tab300 tab300-12 tab300s \
	tab300s-12 tab37 tab382 tab4000a tab450 \
	tabln01 tabln03 tablqp02 tablqp02-12 \
	tab450-12 tab832 taba1 tablp tabtn300 tabX
SOURCES = ${TFILES} ${PFILES}

all:	maketerms

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

maketerms:  terms.hd
	cc -DPART1 -o maketerms maketerms.c
	./maketerms
	cc -DPART2 -o maketerms maketerms.c
	./maketerms
	cc -DPART3 -o maketerms maketerms.c
	./maketerms
	cc -DPART4 -o maketerms maketerms.c
	./maketerms

install:  all
	for i in ${FILES}; do \
		cp $$i ${DESTDIR}/usr/lib/term/$$i; \
		chmod 444 ${DESTDIR}/usr/lib/term/$$i; \
		chog bin ${DESTDIR}/usr/lib/term/$$i; \
	done
	rm -f ${DESTDIR}/usr/lib/term/tab300S
	ln ${DESTDIR}/usr/lib/term/tab300s ${DESTDIR}/usr/lib/term/tab300S
	chmod 444 ${DESTDIR}/usr/lib/term/tab300S
	chog bin ${DESTDIR}/usr/lib/term/tab300S
	rm -f ${DESTDIR}/usr/lib/term/tab300S-12
	ln ${DESTDIR}/usr/lib/term/tab300s-12 ${DESTDIR}/usr/lib/term/tab300S-12
	chmod 444 ${DESTDIR}/usr/lib/term/tab300S-12
	chog bin ${DESTDIR}/usr/lib/term/tab300S-12
	rm -f ${DESTDIR}/usr/lib/term/tab4000A
	ln ${DESTDIR}/usr/lib/term/tab4000a ${DESTDIR}/usr/lib/term/tab4000A
	chmod 444 ${DESTDIR}/usr/lib/term/tab4000A
	chog bin ${DESTDIR}/usr/lib/term/tab4000A
	make clean

clean:
	rm -f maketerms ${FILES}

clobber:  clean
	sccs clean

tags:

sccsinfo:
	sccs info
