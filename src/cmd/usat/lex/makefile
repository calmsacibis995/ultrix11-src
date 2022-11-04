# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/lex

SOURCES= lex.cmds lex.input lex.pgm.c lex.sout lextest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/lex directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/lex ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/lex; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/lex
	chog bin ${DESTDIR}/usr/lib/usat/lex

	cp lex.cmds lex.input lex.pgm.c lex.sout lextest \
		${DESTDIR}/usr/lib/usat/lex
	chmod 644 ${DESTDIR}/usr/lib/usat/lex/*
	chog bin ${DESTDIR}/usr/lib/usat/lex/*
# make lextest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/lex/lextest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
