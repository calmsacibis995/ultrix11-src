# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# troff subsystem shells make file

SOURCES = mm.sh mmt.sh mvt.sh org.sh osdd.sh term text

all:  mm mmt mvt osdd term text

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

mm mmt osdd:
	cp $@.sh $@

# mvt is later linked to mmt
mvt:
	touch mvt

install: insmm insmmt insmvt insosdd insterm instext
	make clean

insmm:	mm
	cp mm ${DESTDIR}/usr/bin/mm
	chmod 755 ${DESTDIR}/usr/bin/mm
	chog bin ${DESTDIR}/usr/bin/mm

insmmt:	mmt
	cp mmt ${DESTDIR}/usr/bin/mmt
	chmod 755 ${DESTDIR}/usr/bin/mmt
	chog bin ${DESTDIR}/usr/bin/mmt

insmvt: mmt
	rm -f ${DESTDIR}/usr/bin/mvt
	ln ${DESTDIR}/usr/bin/mmt ${DESTDIR}/usr/bin/mvt
	chmod 755 ${DESTDIR}/usr/bin/mvt
	chog bin ${DESTDIR}/usr/bin/mvt

insosdd: osdd
	cp osdd ${DESTDIR}/usr/bin/osdd
	chmod 755 ${DESTDIR}/usr/bin/osdd
	chog bin ${DESTDIR}/usr/bin/osdd

insterm:
	cp term ${DESTDIR}/usr/lib/help/term
	chmod 444 ${DESTDIR}/usr/lib/help/term
	chog bin ${DESTDIR}/usr/lib/help/term

instext:
	cp text ${DESTDIR}/usr/lib/help/text
	chmod 444 ${DESTDIR}/usr/lib/help/text
	chog bin ${DESTDIR}/usr/lib/help/text

clean:
	rm -f mm mmt osdd

clobber:  clean
	sccs clean

sccsinfo:
	sccs info
