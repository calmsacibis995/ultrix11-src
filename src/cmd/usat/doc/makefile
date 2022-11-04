# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/doc

SOURCES= doctest nro.dif.sout nro.file nro.file.sout nro.sp.sout

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/doc directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/doc ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/doc; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/doc
	chog bin ${DESTDIR}/usr/lib/usat/doc

	cp doctest nro.dif.sout nro.file nro.file.sout nro.sp.sout \
		${DESTDIR}/usr/lib/usat/doc

	chmod 644 ${DESTDIR}/usr/lib/usat/doc/*
	chog bin ${DESTDIR}/usr/lib/usat/doc/*
# make doctest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/doc/doctest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
