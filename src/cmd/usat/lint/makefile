# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/lint

SOURCES= lint.sout lintfile.c linttest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/lint directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/lint ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/lint; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/lint
	chog bin ${DESTDIR}/usr/lib/usat/lint

	cp lint.sout lintfile.c linttest ${DESTDIR}/usr/lib/usat/lint

	chmod 644 ${DESTDIR}/usr/lib/usat/lint/*
	chog bin ${DESTDIR}/usr/lib/usat/lint/*
# make linttest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/lint/linttest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
