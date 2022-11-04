# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#
# makefile for the loader (ld)

SOURCES= ld.c fixit.sh

all:	ld

ld:	ld.c
	rm -f strings x.c xs.c
	xstr -c ld.c
	cc -c x.c
	mv x.o ld.o
	/usr/ucb/xstr

# put strings (~1350 bytes) into text space
	CC=$(CC) AS="$(AS)" /bin/sh fixit.sh xs

# alternate line to NOT put strings into text space
#	cc -c xs.c

	mv xs.o str.o
	cc -n -o ld ld.o str.o

sources: ${SOURCES}
	sccs get $@

install: all
	cp ld ${DESTDIR}/bin/ld
	-strip ${DESTDIR}/bin/ld
	chog bin ${DESTDIR}/bin/ld
	chmod 1755 ${DESTDIR}/bin/ld
	make clean

clean:
	rm -f a.out x.c xs.c strings *.o ld

clobber: clean
	sccs clean

sccsinfo: ${SOURCES}
	sccs info

tags:
	ctags ld.c
