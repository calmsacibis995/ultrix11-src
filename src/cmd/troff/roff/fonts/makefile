# SCCSID: @(#)makefile	2.1 4/22/86
# troff font tables makefile
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

FFILES = chars.c ftB.c ftBC.c ftC.c ftCE.c ftCI.c ftCK.c ftCS.c ftCW.c \
	 ftG.c ftGI.c ftGM.c ftGR.c ftI.c ftL.c ftLI.c ftPA.c ftPB.c \
	 ftPI.c ftR.c ftS.c ftSB.c ftSI.c ftSM.c ftUD.c
PFILES = makefonts.c
MKFILES = mkfont.c mkfont1.c
FILES = ftB ftBC ftC ftCE ftCI ftCK ftCS ftCW ftG ftGI ftGM ftGR ftI \
	ftL ftLI ftPA ftPB ftPI ftR ftS ftSB ftSI ftSM ftUD
SOURCES = ${FFILES} ${PFILES} ${MKFILES}

all:	makefonts

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

makefonts: makefonts.c
	cc -o makefonts makefonts.c
	./makefonts

install:  all
	(for i in ${FILES}; \
	do \
	 	cp $$i ${DESTDIR}/usr/lib/font/$$i; \
		chmod 444 ${DESTDIR}/usr/lib/font/$$i; \
		chog bin ${DESTDIR}/usr/lib/font/$$i; \
	done)
	rm -f ${DESTDIR}/usr/lib/font/ftH
	ln ${DESTDIR}/usr/lib/font/ftG ${DESTDIR}/usr/lib/font/ftH
	chmod 444 ${DESTDIR}/usr/lib/font/ftH
	chog bin ${DESTDIR}/usr/lib/font/ftH
	rm -f ${DESTDIR}/usr/lib/font/ftHI
	ln ${DESTDIR}/usr/lib/font/ftGI ${DESTDIR}/usr/lib/font/ftHI
	chmod 444 ${DESTDIR}/usr/lib/font/ftHI
	chog bin ${DESTDIR}/usr/lib/font/ftHI
	rm -f ${DESTDIR}/usr/lib/font/ftHM
	ln ${DESTDIR}/usr/lib/font/ftGM ${DESTDIR}/usr/lib/font/ftHM
	chmod 444 ${DESTDIR}/usr/lib/font/ftHM
	chog bin ${DESTDIR}/usr/lib/font/ftHM
	rm -f ${DESTDIR}/usr/lib/font/ftFD
	ln ${DESTDIR}/usr/lib/font/ftUD ${DESTDIR}/usr/lib/font/ftFD
	chmod 444 ${DESTDIR}/usr/lib/font/ftFD
	chog bin ${DESTDIR}/usr/lib/font/ftFD
	make clean

mkfont:  mkfont.c
	cc -n mkfont.c -o mkfont

clean:
	rm -f makefonts a.out ${FILES}

clobber:  clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
