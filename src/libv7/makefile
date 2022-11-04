# SCCSID = "@(#)makefile	3.0	4/22/86"
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for libv7.a
#
# printf and scanf routines from Version 7,
# which in most cases are smaller, and hence
# sometimes more useful for building larger
# programs which overflow the size restriction.

LIB=libv7.a

CFILES = doscan.c printf.c sprintf.c fprintf.c scanf.c strout.c

SOURCES = doprnt.s ${CFILES}

OBJECTS = doprnt.o doscan.o printf.o sprintf.o fprintf.o scanf.o strout.o

CFLAGS = -O

all:	archive

archive: ${OBJECTS}
	ar r ${LIB} ${OBJECTS}
	ranlib ${LIB}

${SOURCES}:
	sccs get $@

sources: ${SOURCES}

install: archive
# install a version on the local system, if desired
	cp ${LIB} ${DESTDIR}/usr/lib/${LIB}
	ranlib ${DESTDIR}/usr/lib/${LIB}
	chog bin ${DESTDIR}/usr/lib/${LIB}
	chmod 644 ${DESTDIR}/usr/lib/${LIB}

	-(if [ ! -d ${DESTDIR}/usr/orphan/lib ] ;\
	then \
	    echo mkdir ${DESTDIR}/usr/orphan/lib ;\
	         mkdir ${DESTDIR}/usr/orphan/lib ;\
	else \
	    true ;\
	fi)

	cp ${LIB} ${DESTDIR}/usr/orphan/lib/${LIB}
	ranlib ${DESTDIR}/usr/orphan/lib/${LIB}
	chog bin ${DESTDIR}/usr/orphan/lib/${LIB}
	chmod 644 ${DESTDIR}/usr/orphan/lib/${LIB}
	make clean

clean:
	rm -f *.o log core a.out ${LIB}

clobber: clean
	sccs clean

tags:	${CFILES}
	ctags ${CFILES}

sccsinfo:
	sccs info
