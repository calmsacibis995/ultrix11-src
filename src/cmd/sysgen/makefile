# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
OBJECTS = sg_main.o sg_subr.o sg_ccf.o
TABLES = sg_tab.o sg_str.o
SOURCES = sysgen.h sg_main.c sg_subr.c sg_ccf.c \
	sg_tab.c sg_str.c
CFLAGS = -O

all: sysgen sg_help

sg_help: sg_help.o
	cc sg_help.o -o sg_help

sg_help.o:
	cc -c sg_help.c

sysgen: ${OBJECTS} ${TABLES}
	cc ${OBJECTS} ${TABLES} -n -o sysgen

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	cp sysgen ${DESTDIR}/usr/sys/conf/sysgen
	-strip  ${DESTDIR}/usr/sys/conf/sysgen
	chmod 744 ${DESTDIR}/usr/sys/conf/sysgen
	chog sys ${DESTDIR}/usr/sys/conf/sysgen

	cp sg_help ${DESTDIR}/usr/sys/conf/sg_help
	-strip ${DESTDIR}/usr/sys/conf/sg_help
	chmod 744 ${DESTDIR}/usr/sys/conf/sg_help
	chog sys ${DESTDIR}/usr/sys/conf/sg_help
	make clean

${OBJECTS}: sysgen.h
${TABLES}: sysgen.h

clean:
	rm -f *.o core a.out log
	rm -f sysgen sg_help

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
