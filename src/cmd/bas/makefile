# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

all:
	@echo Sorry, the bas source will not compile
	@echo the object because it calls for old-style
	@echo library routines.
	@echo It\'s here just in case you want to play.

install: bas
	cp bas ${DESTDIR}/usr/orphan/bin/bas
	-strip ${DESTDIR}/usr/orphan/bin/bas
	chog bin ${DESTDIR}/usr/orphan/bin/bas
	chmod 755 ${DESTDIR}/usr/orphan/bin/bas

bas:	bas.saveme
	cp bas.saveme bas

sccsinfo:
	sccs info

clean clobber tags:
	true
