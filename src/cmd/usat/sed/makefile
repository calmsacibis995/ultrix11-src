# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/sed

SOURCES= sed.input sed.sout sedtest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/sed directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/sed ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/sed; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/sed
	chog bin ${DESTDIR}/usr/lib/usat/sed

	cp sed.input sed.sout sedtest ${DESTDIR}/usr/lib/usat/sed

	chmod 644 ${DESTDIR}/usr/lib/usat/sed/*
	chog bin ${DESTDIR}/usr/lib/usat/sed/*
# make sedtest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/sed/sedtest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
