
# SCCSID=@(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/dc

SOURCES= dc.input dc.sout dctest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/dc directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/dc ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/dc; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/dc
	chog bin ${DESTDIR}/usr/lib/usat/dc

	cp dc.input dc.sout dctest ${DESTDIR}/usr/lib/usat/dc

	chmod 644 ${DESTDIR}/usr/lib/usat/dc/*
	chog bin ${DESTDIR}/usr/lib/usat/dc/*
# make dctest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/dc/dctest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
