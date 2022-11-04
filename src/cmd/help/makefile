# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#
# ULTRIX-11 Help Facility makefile

CFLAGS= -O
SOURCES= help.sh help.c helpinit.c helptext1 helptext2 helptext3 helptext4

all:	help helpinit U11_help help_list help.sh

help:	helpinit help.c
	cc ${CFLAGS} -n help.c -o help

helpinit: U11_help helpinit.c
	cc ${CFLAGS} helpinit.c -o helpinit

U11_help: helptext1 helptext2 helptext3
	cat helptext1 > U11_help
	cat helptext2 >> U11_help
	cat helptext3 >> U11_help
	cat helptext4 >> U11_help

help_list: helpinit
	helpinit U11_help > help_list

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	cp help.sh ${DESTDIR}/bin/help
	chmod 755 ${DESTDIR}/bin/help
	chog bin ${DESTDIR}/bin/help
	cp help ${DESTDIR}/bin/help2
	-strip ${DESTDIR}/bin/help2
	chmod 755 ${DESTDIR}/bin/help2
	chog bin ${DESTDIR}/bin/help2
	-(if [ ! -d ${DESTDIR}/usr/lib/help ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/help; \
	else \
		true; \
	fi)

	cp helpinit U11_help help_list ${DESTDIR}/usr/lib/help
	-strip ${DESTDIR}/usr/lib/help/helpinit
	chmod 744 ${DESTDIR}/usr/lib/help/helpinit
	chmod 444 ${DESTDIR}/usr/lib/help/U11_help ${DESTDIR}/usr/lib/help/help_list
	chog bin ${DESTDIR}/usr/lib/help/U11_help \
	  ${DESTDIR}/usr/lib/help/help_list ${DESTDIR}/usr/lib/help/helpinit
	make clean

clean:
	rm -f *.o help_list help helpinit U11_help

clobber: clean
	sccs clean

sccsinfo:
	sccs info

tags:
	true
