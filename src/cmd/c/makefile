# SCCSID: @(#)makefile	3.0	(ULTRIX-11) 4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# Based on:	@(#)cc.mk	1.4	(System V)
SOURCES = c0.h c00.c c01.c c02.c c03.c c04.c c05.c \
	c1.h c10.c c11.c c12.c c13.c \
	c2.h c20.c c21.c \
	cvopt.c optable
C0_OBJ = c00.o c01.o c02.o c03.o c04.o c05.o
C1_OBJ = c10.o c11.o c12.o c13.o
C2_OBJ = c20.o c21.o
CFLAGS = -O -USYSTEM_V
LDFLAGS = -n
IFLAG = -i
LIB = ${DESTDIR}/lib

all:	c0 c1 c2_id c2_ov

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

c0:	${C0_OBJ}
	$(CC) $(CFLAGS) $(LDFLAGS) -o c0 ${C0_OBJ}

${C0_OBJ}: c0.h

c1:	${C1_OBJ} table.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o c1 ${C1_OBJ} table.o

${C1_OBJ}: c1.h
table.o: optable cvopt
	./cvopt <optable >junk.c
	$(CC) -S junk.c
	sed 's/\.data/\.text/' <junk.s >junk.i
	as -o table.o junk.i
	rm junk.i junk.c junk.s


c2_id:	${C2_OBJ}
	$(CC) $(CFLAGS) $(LDFLAGS) $(IFLAG) -o c2_id ${C2_OBJ}

c2_ov:	${C2_OBJ}
	$(CC) $(CFLAGS) $(LDFLAGS) -o c2_ov ${C2_OBJ}

${C2_OBJ}: c2.h
cvopt:	cvopt.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o cvopt cvopt.c

install:	all
	cp c0 c1 c2_id c2_ov ${LIB}
	-strip ${LIB}/c0 ${LIB}/c1 ${LIB}/c2_id ${LIB}/c2_ov
	chmod 1755 ${LIB}/c0 ${LIB}/c1 ${LIB}/c2_id ${LIB}/c2_ov
	chog bin ${LIB}/c0 ${LIB}/c1 ${LIB}/c2_id ${LIB}/c2_ov
	make clean

clean:
	-rm -f *.o cvopt c0 c1 c2_id c2_ov

clobber:	clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
