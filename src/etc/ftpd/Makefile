# SCCSID: @(#)Makefile	3.0	(ULTRIX-11)	4/22/86

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# Based on:
# 	Makefile	1.6	(ULTRIX-32)	12/4/84
#	@(#)Makefile	4.2	6/12/83
#
SOURCES = fptd.c ftpcmd.y glob.c vers.c
ALL=	ftpd.o ftpcmd.o glob.o vers.o
CFLAGS=	-O
DIR=${DESTDIR}/usr/etc

all:	ftpd

ftpd:	${ALL}
	${CC} -o ftpd ${ALL} -ljobs

vers.o:	ftpd.c ftpcmd.y
	sh newvers.sh
	${CC} -c vers.c

install: Install clean

Install: all
	cp ftpd ${DIR}/ftpd
	strip ${DIR}/ftpd
	chog bin ${DIR}/ftpd
	chmod 755 ${DIR}/ftpd

clean:
	rm -f ftpd *.o *.s errs core a.out t.? yacc.* y.tab.c

clobber: clean
	rm -f *.[chy]
	sccs clean

sccsinfo:
	sccs info
tags:
	true
