# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# reorder gives an editor command for most common terminals
# (in reverse order from n'th to 1'st most commonly used)
# to move them to the front of termcap
#
SOURCES= termcap.src reorder

all:	termcap.obj

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

termcap.obj: termcap.src
	ed - termcap.src < reorder

install: termcap.obj
	cp termcap.obj ${DESTDIR}/etc/termcap
	chmod 644 ${DESTDIR}/etc/termcap
	chown root ${DESTDIR}/etc/termcap
	make clean

tabset:	forceit
	mkdir ${DESTDIR}/usr/lib/tabset
	chmod 755 ${DESTDIR}/usr/lib/tabset
	cp ./tabset/* ${DESTDIR}/usr/lib/tabset

clean:
	rm -f termcap.obj

clobber: clean
	sccs clean

tags:
	@true

sccsinfo:
	sccs info

forceit:
	@true
