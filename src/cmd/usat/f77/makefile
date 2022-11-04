# SCCSID=@(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/f77

SOURCES= f77.in.f f77.sout f77test

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/f77 directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/f77 ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/f77; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/f77
	chog bin ${DESTDIR}/usr/lib/usat/f77

	cp f77.in.f f77.sout f77test ${DESTDIR}/usr/lib/usat/f77

	chmod 644 ${DESTDIR}/usr/lib/usat/f77/*
	chog bin ${DESTDIR}/usr/lib/usat/f77/*
# make f77test executable
	chmod 755 ${DESTDIR}/usr/lib/usat/f77/f77test

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
