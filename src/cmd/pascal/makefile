# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

all:	eyacc pi px pxp

sources:
	(cd eyacc; make sources)
	(cd pi; make sources)
	(cd px; make sources)
	(cd pxp; make sources)

eyacc:	forceit
	(cd eyacc; make)

pi:	forceit
	(cd pi; make)

px:	forceit
	(cd px; make)

pxp:	forceit
	(cd pxp; make)

install: 
	(cd eyacc; make DESTDIR=${DESTDIR} install)
	(cd pi; make DESTDIR=${DESTDIR} install)
	(cd px; make DESTDIR=${DESTDIR} install)
	(cd pxp; make DESTDIR=${DESTDIR} install)

forceit:

clean:
	(cd eyacc; make clean)
	(cd pi; make clean)
	(cd px; make clean)
	(cd pxp; make clean)

clobber: clean
	sccs clean

tags:
	(cd eyacc; make tags)
	(cd pi; make tags)
	(cd px; make tags)
	(cd pxp; make tags)

sccsinfo:
	(cd eyacc; make sccsinfo)
	(cd pi; make sccsinfo)
	(cd px; make sccsinfo)
	(cd pxp; make sccsinfo)
