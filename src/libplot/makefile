# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

LIBS =	libplot.a libt4014.a libt300.a libt300s.a \
	libt450.a libtgigi.a libtla100.a libtla50.a \
	libtregis.a libvt0.a

all:	${LIBS}

libplot.a: forceit
	cd plot; make "CC=${CC}"
libt4014.a: forceit
	cd t4014; make "CC=${CC}"
libt300.a: forceit
	cd t300; make "CC=${CC}"
libt300s.a: forceit
	cd t300s; make "CC=${CC}"
libt450.a: forceit
	cd t450; make "CC=${CC}"
libtgigi.a: forceit
	cd tgigi; make "CC=${CC}"
libtla50.a: forceit
	cd tla50; make "CC=${CC}"
libtla100.a: forceit
	cd tla100; make "CC=${CC}"
libtregis.a: forceit
	cd tregis; make "CC=${CC}"
libvt0.a: forceit
	cd vt0; make "CC=${CC}"

sources: forceit
	cd plot; make sources
	cd t4014; make sources
	cd t300; make sources
	cd t300s; make sources
	cd t450; make sources
	cd tgigi; make sources
	cd tla100; make sources
	cd tla50; make sources
	cd tregis; make sources
	cd vt0; make sources

install: forceit
	cd plot; make install "DESTDIR=${DESTDIR}" "CC=${CC}"
	cd t4014; make install "DESTDIR=${DESTDIR}" "CC=${CC}"
	cd t300; make install "DESTDIR=${DESTDIR}" "CC=${CC}"
	cd t300s; make install "DESTDIR=${DESTDIR}" "CC=${CC}"
	cd t450; make install "DESTDIR=${DESTDIR}" "CC=${CC}"
	cd tgigi; make install "DESTDIR=${DESTDIR}" "CC=${CC}"
	cd tla100; make install "DESTDIR=${DESTDIR}" "CC=${CC}"
	cd tla50; make install "DESTDIR=${DESTDIR}" "CC=${CC}"
	cd tregis; make install "DESTDIR=${DESTDIR}" "CC=${CC}"
	cd vt0; make install "DESTDIR=${DESTDIR}" "CC=${CC}"

forceit:

clean:
	rm -f *.o core a.out log
	cd plot; make clean
	cd t4014; make clean
	cd t300; make clean
	cd t300s; make clean
	cd t450; make clean
	cd tgigi; make clean
	cd tla100; make clean
	cd tla50; make clean
	cd tregis; make clean
	cd vt0; make clean

clobber:
	rm -f *.o core a.out log
	sccs clean
	cd plot; make clobber
	cd t4014; make clobber
	cd t300; make clobber
	cd t300s; make clobber
	cd t450; make clobber
	cd tla100; make clobber
	cd tla50; make clobber
	cd tregis; make clobber
	cd tgigi; make clobber
	cd vt0; make clobber

tags:	forceit
	cd plot; make tags
	cd t4014; make tags
	cd t300; make tags
	cd t300s; make tags
	cd t450; make tags
	cd tla100; make tags
	cd tla50; make tags
	cd tregis; make tags
	cd tgigi; make tags
	cd vt0; make tags

sccsinfo:	forceit
	cd plot; make sccsinfo
	cd t4014; make sccsinfo
	cd t300; make sccsinfo
	cd t300s; make sccsinfo
	cd t450; make sccsinfo
	cd tla100; make sccsinfo
	cd tla50; make sccsinfo
	cd tregis; make sccsinfo
	cd tgigi; make sccsinfo
	cd vt0; make sccsinfo
