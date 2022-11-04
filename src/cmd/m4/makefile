# SCCSID: %W5	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS = -n -O
SOURCES= m4.c m4y.y
OBJECTS = m4.o m4y.o

all:	m4

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

m4:	${OBJECTS}
	cc -n -o m4 m4.o m4y.o

${OBJECTS}: ${SOURCES}

cmp:	m4
	cmp m4 ${DESTDIR}/usr/bin/m4

install:	m4
	cp m4 ${DESTDIR}/usr/bin/m4
	-strip ${DESTDIR}/usr/bin/m4
	chmod 755 ${DESTDIR}/usr/bin/m4
	chog bin ${DESTDIR}/usr/bin/m4 
	make clean

clean:
	rm -f *.o m4

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
