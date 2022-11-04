# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = y1a.c y1b.c y2.c y3.c y4.c
OBJ70 = y1a70.o y1b70.o y270.o y370.o y470.o
OBJ40 = y1a40.o y1b40.o y240.o y340.o y440.o
CFLAGS= -O
BIN = ${DESTDIR}/usr/bin

all: yacc70 yacc40

sources: dextern files ${SOURCES}

dextern files ${SOURCES}:
	sccs get $@

yacc: yacc70
	cp yacc70 yacc

install: all
	cp yacc40 yacc70 ${BIN}
	-strip ${BIN}/yacc40
	-strip ${BIN}/yacc70
	cp ${BIN}/yacc70 ${BIN}/yacc
	chog bin ${BIN}/yacc70 ${BIN}/yacc40 ${BIN}/yacc
	chmod 755 ${BIN}/yacc70 ${BIN}/yacc40 ${BIN}/yacc
	make clean

yacc70: $(OBJ70)
	cc -i -o yacc70 $(OBJ70)

y1a70.o: y1a.c dextern files
	cc -c $(CFLAGS) -DMEDIUM y1a.c
	mv y1a.o y1a70.o

y1b70.o: y1b.c dextern files
	cc -c $(CFLAGS) -DMEDIUM y1b.c
	mv y1b.o y1b70.o

y270.o: y2.c dextern files
	cc -c $(CFLAGS) -DMEDIUM y2.c
	mv y2.o y270.o

y370.o: y3.c dextern files
	cc -c $(CFLAGS) -DMEDIUM y3.c
	mv y3.o y370.o

y470.o: y4.c dextern files
	cc -c $(CFLAGS) -DMEDIUM y4.c
	mv y4.o y470.o


yacc40: $(OBJ40)
	cc -n -o yacc40 y1a40.o -Z y240.o -Z y1b40.o y340.o y440.o -L -lv7

y1a40.o: y1a.c dextern files
	cc -c $(CFLAGS) -DSMALL y1a.c
	mv y1a.o y1a40.o

y1b40.o: y1b.c dextern files
	cc -c $(CFLAGS) -DSMALL y1b.c
	mv y1b.o y1b40.o

y240.o: y2.c dextern files
	cc -c $(CFLAGS) -DSMALL y2.c
	mv y2.o y240.o

y340.o: y3.c dextern files
	cc -c $(CFLAGS) -DSMALL y3.c
	mv y3.o y340.o

y440.o: y4.c dextern files
	cc -c $(CFLAGS) -DSMALL y4.c
	mv y4.o y440.o

clean:
	rm -f *.o log yacc yacc40 yacc70

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
