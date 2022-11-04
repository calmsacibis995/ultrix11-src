# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS = -n -O

SOURCES= r0.c r1.c r2.c r.h rio.c rlook.c rlex.c r.g

all:	ratfor

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install:	all
	cp ratfor ${DESTDIR}/usr/bin/ratfor
	-strip ${DESTDIR}/usr/bin/ratfor
	chmod 755 ${DESTDIR}/usr/bin/ratfor
	chog bin ${DESTDIR}/usr/bin/ratfor
	make clean

cmp:	ratfor
	cmp ratfor ${DESTDIR}/usr/bin/ratfor

ratfor:	r0.o r1.o r2.o rio.o rlook.o rlex.o y.tab.o
	cc $(CFLAGS) r*.o y.tab.o -o ratfor

r0.o:	r.h y.tab.h r0.c
r1.o:	r.h y.tab.h r1.c
r2.o:	r.h y.tab.h r2.c
rio.o:	r.h y.tab.h rio.c
rlook.o:	r.h y.tab.h rlook.c
rlex.o:	r.h y.tab.h rlex.c
y.tab.c:	r.g
	yacc -d r.g
y.tab.h:	r.g
	yacc -d r.g

clean:
	rm -f *.o y.tab.c y.tab.h ratfor

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
