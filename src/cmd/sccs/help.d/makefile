# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES= ad bd cb cm cmds co de default ge he prs rc un ut vc

all: ${SOURCES}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	-(if [ ! -d ${DESTDIR}/usr/lib/help ]; \
	then \
		echo "mkdir ${DESTDIR}/usr/lib/help"; \
		mkdir ${DESTDIR}/usr/lib/help; \
	else \
		true; \
	fi)

	@(for i in ${SOURCES}; \
	do \
		echo "cp $$i ${DESTDIR}/usr/lib/help/$$i"; \
		cp $$i ${DESTDIR}/usr/lib/help/$$i; \
		echo "chog bin ${DESTDIR}/usr/lib/help/$$i"; \
		chog bin ${DESTDIR}/usr/lib/help/$$i; \
		echo "chmod 444 ${DESTDIR}/usr/lib/help/$$i"; \
		chmod 444 ${DESTDIR}/usr/lib/help/$$i; \
	done)

clean:
	rm -f *.o log

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
