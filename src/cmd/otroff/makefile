# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

BIN=${DESTDIR}/usr/orphan/usr/bin
TMACDIR=${DESTDIR}/usr/orphan/usr/lib/tmac

MACROS=tmac.an

SOURCES=${MACROS}

all:	font term nroff troff

nmake tmake:
	sccs get $@

nroff:	nmake
	make -f nmake
	rm *.o

troff:	tmake
	make -f tmake
	rm *.o
font:	forceit
	(cd font; make)

term:	forceit
	(cd term; make)

macros: ${MACROS}

sources: ${SOURCES}
	(cd font; make sources)
	(cd term; make sources)

${SOURCES}:
	sccs get $@

forceit:

install: nroff troff macros
	(cd font; make install DESTDIR=${DESTDIR})
	(cd term; make install DESTDIR=${DESTDIR})	
	cp ${MACROS} ${TMACDIR}
	chog bin ${TMACDIR}/*
	chmod 644 ${TMACDIR}/*
	cp nroff troff ${BIN}
	-strip ${BIN}/nroff ${BIN}/troff
	chmod 755 ${BIN}/nroff ${BIN}/troff
	chog bin ${BIN}/nroff ${BIN}/troff
	make clean

cmp:	all
	cmp nroff ${BIN}/nroff
	cmp troff ${BIN}/troff

clean:
	rm -f *.o nroff troff
	(cd font; make clean)
	(cd term; make clean)

clobber:
	rm -f *.o nroff troff
	(cd font; make clobber)
	(cd term; make clobber)
	sccs clean

tags:	forceit
	make -f nmake tags
	make -f tmake tags

sccsinfo:
	sccs info
	make -f nmake sccsinfo
	make -f tmake sccsinfo
