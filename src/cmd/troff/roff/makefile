# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	roff make file (troff subsystem)

SOURCES = suftabmake.c suftab.c

all:  nroff troff suftab term font

sources: ${SOURCES}
	make -f nmake sources
	make -f tmake sources
	(cd terms; make sources)
	(cd fonts; make sources)

${SOURCES}:
	sccs get $@

nroff:
	make -f nmake

troff:
	make -f tmake

suftab:	suftabmake.c suftab.c
	cc suftabmake.c
	./a.out

term:
	(cd terms; make)
font:
	(cd fonts; make)

install: all
	cp nroff ${DESTDIR}/usr/bin/nroff
	-strip ${DESTDIR}/usr/bin/nroff
	chmod 755 ${DESTDIR}/usr/bin/nroff
	chog bin ${DESTDIR}/usr/bin/nroff
	cp troff ${DESTDIR}/usr/bin/troff
	-strip ${DESTDIR}/usr/bin/troff
	chmod 755 ${DESTDIR}/usr/bin/troff
	chog bin ${DESTDIR}/usr/bin/troff
	cp suftab ${DESTDIR}/usr/lib/suftab
	chmod 644 ${DESTDIR}/usr/lib/suftab
	chog bin ${DESTDIR}/usr/lib/suftab
	(cd terms; make install)
	(cd fonts; make install)
	make clean

clean:
	rm -f *.o a.out nroff troff suftab bldtab nrtab.dt rqtab.dt
	(cd terms; make clean)
	(cd fonts; make clean)

clobber: clean
	sccs clean
	(cd terms; make clobber)
	(cd fonts; make clobber)

tags:	${SOURCES}
	ctags *.c
	(cd terms; make tags)
	(cd fonts; make tags)

sccsinfo:
	sccs info
	(cd terms; make sccsinfo)
	(cd fonts; make sccsinfo)
