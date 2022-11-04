# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
OBJECTS = mkconf.o subr.o globl.o
TABLES = mkc_btab.o mkc_table.o mkc_ovt.o mkc_str.o
SOURCES = mkconf.h mkconf.c subr.c globl.c \
	mkc_btab.c mkc_table.c mkc_ovt.c mkc_str.c
CFLAGS = -O

mkconf: ${OBJECTS} ${TABLES}
	cc ${OBJECTS} ${TABLES} -o mkconf

all:	mkconf

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	cp mkconf ${DESTDIR}/usr/sys/conf/mkconf
	-strip ${DESTDIR}/usr/sys/conf/mkconf
	chog sys ${DESTDIR}/usr/sys/conf/mkconf
	chmod 775 ${DESTDIR}/usr/sys/conf/mkconf
	make clean

${OBJECTS}: mkconf.h
${TABLES}: mkconf.h

clean:
	rm -f *.o core a.out log mkconf

clobber: clean
	rm -f mkconf
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
