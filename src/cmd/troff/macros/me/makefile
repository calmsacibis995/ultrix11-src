# SCCSID: @(#)makefile	3.0	4/22/86 
#
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# make install will copy the me troff and nroff files in this directory 
#
#

MEMACS= acm.me chars.me deltext.me eqn.me float.me footnote.me index.me \
	local.me null.me refer.me sh.me tbl.me thesis.me
SOURCES = ${MEMACS} tmac.e

all:

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install:
	for i in ${MEMACS}; do \
		cp $$i ${DESTDIR}/usr/lib/me/$$i ;\
		chmod 644 ${DESTDIR}/usr/lib/me/$$i ;\
		chown bin ${DESTDIR}/usr/lib/me/$$i ;\
	done
	cp tmac.e ${DESTDIR}/usr/lib/tmac/tmac.e

clean:

clobber:
	sccs clean
tags:
sccsinfo:
	sccs info
