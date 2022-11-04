# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
INCLUDE= /usr/include
CFLAGS= -O
AS=	as -V
SOURCES = getwd.c killpg.s setpgrp.s signal.s sigset.c sleep.c wait2.s
OBJECTS = getwd.o sleep.o killpg.o setpgrp.o signal.o sigset.o wait2.o
ARCHIVE = libjobs.a

all: ${ARCHIVE}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

${ARCHIVE}: ${OBJECTS}
	rm -f ${ARCHIVE}
	ar cr ${ARCHIVE} ${OBJECTS}
	ranlib ${ARCHIVE}

install: ${ARCHIVE}
	cp ${ARCHIVE} ${DESTDIR}/usr/lib
	ranlib ${DESTDIR}/usr/lib/${ARCHIVE}
	chown bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chgrp bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/usr/lib/${ARCHIVE}
	rm -f *.o core a.out log ${ARCHIVE}

clean:
	rm -f *.o core a.out log ${ARCHIVE}

clobber: clean
	sccs clean

.s.o:
	${AS} -o $*.o ${INCLUDE}/sys.s $*.s

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
