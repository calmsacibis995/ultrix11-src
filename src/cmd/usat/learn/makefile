# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/learn

SOURCES= learn.input learn.sout learntest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/learn directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/learn ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/learn; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/learn
	chog bin ${DESTDIR}/usr/lib/usat/learn

	cp learn.input learn.sout learntest ${DESTDIR}/usr/lib/usat/learn

	chmod 644 ${DESTDIR}/usr/lib/usat/learn/*
	chog bin ${DESTDIR}/usr/lib/usat/learn/*
# make learntest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/learn/learntest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
