# SCCSID: @(#)makefile	3.1	10/15/87
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

MACFILES = ntar.c mmn.src mmt.src vmca.src osdd.src an.src ptx.src
TMACFILES = tmac.an1 tmac.m tmac.org tmac.ptx tmac.osd tmac.v
SOURCES = ${MACFILES} ${TMACFILES}

all:	ntar mmn mmt vmca osdd man ptx tmac me

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

ntar:	ntar.c
	cc -O -o ntar ntar.c

mmn:	ntar
	sh ./macrunch -n m -d ${DESTDIR}/usr mmn

mmt:	ntar
	@echo cannot do macrunch for troff macros, continuing
#	sh ./macrunch -t m -d ${DESTDIR}/usr mmt

vmca:	ntar
	sh ./macrunch -d ${DESTDIR}/usr vmca

org:	ntar
	sh ./macrunch -d ${DESTDIR}/usr org

osdd:	ntar
	sh ./macrunch -d ${DESTDIR}/usr osdd

man:	ntar
	sh ./macrunch -n an -d ${DESTDIR}/usr an
	@echo cannot do macrunch for troff macros, continuing
#	sh ./macrunch -t an -d ${DESTDIR}/usr an

ptx:	ntar
	sh ./macrunch -d ${DESTDIR}/usr ptx

tmac:

me:	forceit

install: ntar insmmn insmmt insvmca insosdd insman insptx instmac insme
	make clean

insmmn:	ntar
	sh ./macrunch -n m -m -d ${DESTDIR}/usr mmn

insmmt:	ntar
	@echo cannot do macrunch for troff macros, continuing
#	sh ./macrunch -t m -m -d ${DESTDIR}/usr mmt

insvmca:	ntar
	sh ./macrunch -m -d ${DESTDIR}/usr vmca

insorg:	ntar
	sh ./macrunch -m -d ${DESTDIR}/usr org

insosdd:	ntar
	sh ./macrunch -m -d ${DESTDIR}/usr osdd

insman:	ntar
	sh ./macrunch -n an -m -d ${DESTDIR}/usr an
	@echo cannot do macrunch for troff macros, continuing
#	sh ./macrunch -t an -m -d ${DESTDIR}/usr an

insptx:	ntar
	sh ./macrunch -m -d ${DESTDIR}/usr ptx

instmac:
	for i in ${TMACFILES}; do \
		cp $$i ${DESTDIR}/usr/lib/tmac/$$i; \
		chmod 644 ${DESTDIR}/usr/lib/tmac/$$i; \
		chog bin ${DESTDIR}/usr/lib/tmac/$$i; \
	done

insme:
	(cd me; make install)

forceit:

clean:
	rm -f cmp* ucmp*
	rm -f ntar mmn mmt vmca org osdd an ptx

clobber:  clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
