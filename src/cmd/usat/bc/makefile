# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/bc

SOURCES= bc.input bc.sout bctest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/bc directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/bc ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/bc; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/bc
	chog bin ${DESTDIR}/usr/lib/usat/bc

	cp bc.input bc.sout bctest ${DESTDIR}/usr/lib/usat/bc

	chmod 644 ${DESTDIR}/usr/lib/usat/bc/*
	chog bin ${DESTDIR}/usr/lib/usat/bc/*
# make bctest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/bc/bctest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
