# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/pascal

SOURCES= pascaltest test1.p test2.p test1.sout test2.sout test4.sout

all: ${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/pascal directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/pascal ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/pascal; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/pascal
	chog bin ${DESTDIR}/usr/lib/usat/pascal

	cp pascaltest test1.p test1.sout test2.p test2.sout test4.sout \
		${DESTDIR}/usr/lib/usat/pascal

	chmod 644 ${DESTDIR}/usr/lib/usat/pascal/*
	chog bin ${DESTDIR}/usr/lib/usat/pascal/*
# make pascaltest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/pascal/pascaltest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
