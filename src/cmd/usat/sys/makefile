# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/sys

SOURCES= systest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/sys directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/sys ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/sys; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/sys
	chog bin ${DESTDIR}/usr/lib/usat/sys

	cp systest ${DESTDIR}/usr/lib/usat/sys

	chmod 644 ${DESTDIR}/usr/lib/usat/sys/*
	chog bin ${DESTDIR}/usr/lib/usat/sys/*
# make systest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/sys/systest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
