# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/pcc

SOURCES= pcc.in.c pcc.sout pcctest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/pcc directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/pcc ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/pcc; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/pcc
	chog bin ${DESTDIR}/usr/lib/usat/pcc

	cp pcc.in.c pcc.sout pcctest ${DESTDIR}/usr/lib/usat/pcc

	chmod 644 ${DESTDIR}/usr/lib/usat/pcc/*
	chog bin ${DESTDIR}/usr/lib/usat/pcc/*
# make pcctest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/pcc/pcctest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
