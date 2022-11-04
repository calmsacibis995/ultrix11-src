# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = fp1.s fp2.s fp3.s fpx.s

all:	fp.o

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

fp.o:
	as -o fp.o fp1.s fp2.s fp3.s fpx.s

clean:
	rm -f *.o

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
