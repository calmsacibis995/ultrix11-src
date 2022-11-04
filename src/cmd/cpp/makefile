# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
SOURCES = cpp.c cpy.y yylex.c
CFLAGS=-O -n -DFLEXNAMES=1

all:	cpp ocpp

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: cpp ocpp
	cp cpp ${DESTDIR}/lib/cpp
	strip ${DESTDIR}/lib/cpp
	chmod 755 ${DESTDIR}/lib/cpp
	chog bin ${DESTDIR}/lib/cpp
	cp ocpp ${DESTDIR}/usr/c/ocpp
	strip ${DESTDIR}/usr/c/ocpp
	chmod 755 ${DESTDIR}/usr/c/ocpp
	chog bin ${DESTDIR}/usr/c/ocpp
	make clean

cmp:	cpp
	cmp cpp ${DESTDIR}/lib/cpp
	rm *.o cpp

cpp: cpp.o cpy.o
	${CC} $(CFLAGS) cpp.o cpy.o -o cpp

cpp.o: cpp.c
	${CC} $(CFLAGS) -c -Dunix=1 -Dpdp11=1 -UOLDCC cpp.c

ocpp: ocpp.o ocpy.o
	${CC} $(CFLAGS) ocpp.o ocpy.o -o ocpp

ocpp.o: cpp.c
	@rm -f ocpp.c
	ln cpp.c ocpp.c
	${CC} -B ${CFLAGS} -c -Dunix=1 -Dpdp11=1 -DOLDCC=1 ocpp.c
	rm -f ocpp.c

cpy.o: cpy.y yylex.c

ocpy.o: cpy.y yylex.c
	yacc  cpy.y
	cc -B ${CFLAGS} -c y.tab.c
	rm y.tab.c
	mv y.tab.o ocpy.o

clean:
	rm -f *.o cpp ocpp log a.out core

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
