# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/yacc

SOURCES= yacc.input yacc.sout yacctest

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

# install in /usr/lib/usat/yacc directory
install: all
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/yacc ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/yacc; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/yacc
	chog bin ${DESTDIR}/usr/lib/usat/yacc

	cp yacc.input yacc.sout yacctest \
		${DESTDIR}/usr/lib/usat/yacc

	chmod 644 ${DESTDIR}/usr/lib/usat/yacc/*
	chog bin ${DESTDIR}/usr/lib/usat/yacc/*
# make yacctest executable
	chmod 755 ${DESTDIR}/usr/lib/usat/yacc/yacctest

clean:
	rm -f log
clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
