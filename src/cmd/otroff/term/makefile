# SCCSID: @(#)makefile	3.0	4/24/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

OBJECTS= tab300-12.o tab300.o tab300s-12.o tab300s.o tab37.o tab450-12-8.o \
	tab450-12.o tab450.o tab832.o taba1.o tablp.o tabtn300.o
CFILES= tab300-12.c tab300.c tab300s-12.c tab300s.c tab37.c tab450-12.c \
	tab450.c tab832.c taba1.c tablp.c tabtn300.c
INCS= code.300
SOURCES = ${CFILES} ${INCS}


all:	$(OBJECTS)

install:	all
	for i in *.o; \
	do \
		d=`basename $$i .o`; \
		cp $$i ${DESTDIR}/usr/orphan/usr/lib/term/$$d; \
		chog bin ${DESTDIR}/usr/orphan/usr/lib/term/$$d; \
		chmod 444 ${DESTDIR}/usr/orphan/usr/lib/term/$$d; \
	done

cmp:	all
	for i in *.o; do cmp $$i ${DESTDIR}/usr/orphan/usr/lib/term/`basename $$i .o`; done

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

tab450-12-8.c: tab450128.c
	cp tab450128.c tab450-12-8.c
tab450128.c:
	sccs get $@

.c.o:
	cc -c $<; strip $@

clean:
	rm -f *.o

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
