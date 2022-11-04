# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/cc

SOURCES= cc.in.c cc.sout cctest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/cc directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/cc ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/cc; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/cc
	chog bin ${DESTDIR}/usr/lib/usat/cc

	cp cc.in.c cc.sout cctest ${DESTDIR}/usr/lib/usat/cc

	chmod 644 ${DESTDIR}/usr/lib/usat/cc/*
	chog bin ${DESTDIR}/usr/lib/usat/cc/*
# make cctest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/cc/cctest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
