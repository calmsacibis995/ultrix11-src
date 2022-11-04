# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/awk

SOURCES= awk.input awk.sout awktest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/awk directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/awk ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/awk; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/awk
	chog bin ${DESTDIR}/usr/lib/usat/awk

	cp awk.input awk.sout awktest ${DESTDIR}/usr/lib/usat/awk

	chmod 644 ${DESTDIR}/usr/lib/usat/awk/*
	chog bin ${DESTDIR}/usr/lib/usat/awk/*
# make awktest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/awk/awktest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
