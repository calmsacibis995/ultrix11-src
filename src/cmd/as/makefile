# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

AS1=as11.s as12.s as13.s as14.s as15.s as16.s as17.s as18.s as19.s

AS2=as21.s as22.s as23.s as24.s as25.s as26.s as27.s as28.s as29.s

SOURCES= ${AS1} ${AS2}
all:	as as2

sources: ${AS1} ${AS2}

${SOURCES}:
	sccs get $@

cmp:	as as2
	cmp as ${DESTDIR}/bin/as
	cmp as2 ${DESTDIR}/lib/as2

install: all
	cp as ${DESTDIR}/bin/as
	cp as2 ${DESTDIR}/lib/as2
	-strip ${DESTDIR}/bin/as ${DESTDIR}/lib/as2
	chmod 755 ${DESTDIR}/bin/as
	chmod 1755 ${DESTDIR}/lib/as2
	chog bin ${DESTDIR}/bin/as ${DESTDIR}/lib/as2
	make clean

as:	${AS1}
	/bin/as /usr/include/sys.s ${AS1}
	ld -n a.out -o as

as2:	${AS2}
	/bin/as /usr/include/sys.s ${AS2}
	ld -n a.out -o as2

clean:
	rm -f *.o a.out as as2

clobber:	clean
	sccs clean

tags:	forceit
	@(echo "no tags with assembler files")

sccsinfo:
	sccs info

forceit:
