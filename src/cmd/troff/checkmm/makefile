# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	checkmm make file

CFLAGS = -O

SOURCES = chekl.l chekmain.c
OBJS = chekl.o chekmain.o

all: checkmm

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

checkmm: ${OBJS}
	cc -n -o checkmm ${OBJS} -ll -lPW

install: all
	cp checkmm ${DESTDIR}/usr/bin/checkmm
	-strip ${DESTDIR}/usr/bin/checkmm
	chmod 755 ${DESTDIR}/usr/bin/checkmm
	chog bin ${DESTDIR}/usr/bin/checkmm
	make clean

clean:
	  rm -f *.o
	  rm -f checkmm

clobber:  clean
	  sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
