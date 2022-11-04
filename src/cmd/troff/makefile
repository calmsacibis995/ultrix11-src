# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	troff sub-system makefile

all:	roff macros shells checkmm

roff macros shells checkmm: forceit
	(cd $@; make)

install:
	(cd roff; make DESTDIR=${DESTDIR} install)
	(cd macros; make DESTDIR=${DESTDIR} install)
	(cd shells; make DESTDIR=${DESTDIR} install)
	(cd checkmm; make DESTDIR=${DESTDIR} install)

forceit:

clean:
	(cd roff; make clean)
	(cd macros; make clean)
	(cd shells; make clean)
	(cd checkmm; make clean)

clobber:
	(cd roff; make clobber)
	(cd macros; make clobber)
	(cd shells; make clobber)
	(cd checkmm; make clobber)
	sccs clean

tags:
	(cd roff; make tags)
	(cd checkmm; make tags)

sccsinfo:
	sccs info
	(cd roff; make sccsinfo)
	(cd macros; make sccsinfo)
	(cd shells; make sccsinfo)
	(cd checkmm; make sccsinfo)
