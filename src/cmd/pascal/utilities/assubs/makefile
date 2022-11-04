# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = pcrt0.s

all:	pcrt0.o

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

pcrt0.o: pcrt0.s
	as -o pcrt0.o /usr/include/sys.s pcrt0.s

clean:
	rm -f *.o

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
