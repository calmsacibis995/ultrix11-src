# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = c0.h c1.h c2.h cvopt.c \
	c00.c c01.c c02.c c03.c c04.c c05.c \
	c10.c c11.c c12.c c13.c \
	c20.c c21.c

# use Version 7 printf routines due to a bug in new printf
# printf("%c",r), r=NULL will actually print a null in the file
LIBES = -lv7

# use old compiler to compile old compiler (-B in CFLAGS)
CFLAGS = -B -O -n

all:	c0 c1 c2_id c2_ov

sources: ${SOURCES} table.s

${SOURCES} table.s:
	sccs get $@

cmp:	all
	cmp c0 ${DESTDIR}/usr/c/oc0
	cmp c1 ${DESTDIR}/usr/c/oc1
	cmp c2_id ${DESTDIR}/usr/c/oc2_id
	cmp c2_ov ${DESTDIR}/usr/c/oc2_ov

install: all
	cp c0 ${DESTDIR}/usr/c/oc0
	cp c1 ${DESTDIR}/usr/c/oc1
	-strip ${DESTDIR}/usr/c/oc0 ${DESTDIR}/usr/c/oc1
	chmod 755 ${DESTDIR}/usr/c/oc0 ${DESTDIR}/usr/c/oc1
	chog bin ${DESTDIR}/usr/c/oc0 ${DESTDIR}/usr/c/oc1
	cp c2_id ${DESTDIR}/usr/c/oc2_id
	cp c2_ov ${DESTDIR}/usr/c/oc2_ov
	-strip ${DESTDIR}/usr/c/oc2_id ${DESTDIR}/usr/c/oc2_ov
	chmod 755 ${DESTDIR}/usr/c/oc2_id ${DESTDIR}/usr/c/oc2_ov
	chog bin ${DESTDIR}/usr/c/oc2_id ${DESTDIR}/usr/c/oc2_ov
# link oc2_id to oc2 for Bedrock
	rm -f ${DESTDIR}/usr/c/oc2
	ln ${DESTDIR}/usr/c/oc2_id ${DESTDIR}/usr/c/oc2
	make clean

c0: c0.h c00.o c01.o c02.o c03.o c04.o c05.o
	cc $(CFLAGS) -o c0 c00.o c01.o c02.o c03.o c04.o c05.o $(LIBES)

c1: c10.o c11.o c12.o c13.o table.o
	cc $(CFLAGS) -o c1 c10.o c11.o c12.o c13.o table.o $(LIBES)

#fc1: c10.o c11.o c12.o c13.o table.o
#	cc $(CFLAGS) -f -o fc1 c10.o c11.o c12.o c13.o table.o $(LIBES)

c10.o:	c10.c c1.h
c11.o:	c11.c c1.h
c12.o:	c12.c c1.h
c13.o:	c13.c c1.h

table.o: table.s cvopt
	cvopt <table.s >table.i
	-as -o table.o table.i
	rm table.i

c2_id: c20.o c21.o
	cc -i -O -o c2_id c20.o c21.o $(LIBES)

c2_ov: c20.o c21.o
	cc -n -O -o c2_ov c20.o c21.o $(LIBES)

c20.o:	c20.c c2.h
c21.o:	c21.c c2.h

cvopt:	cvopt.c
	cc $(CFLAGS) -o cvopt cvopt.c $(LIBES)

clean:
	rm -f *.o
	rm -f c0 c1 c2_id c2_ov cvopt

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
