# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for line printer spooling system
#
# Ownerships
# ROOT		a user that can access any and all files
# DAEMON	someone special + the group id of the spooling programs
#
# Options:
#	add -DSLEEPER to CFLAGS if you want the printer daemon to sleep
#	awaiting work;  otherwise a new daemon is started with each job.
#

SOURCES= lpr.c lpd.c lpq.c lprm.c lpdrestart.c ulf.c \
	ln01of.c ln03of.c lqf.c \
	lpdchar.c printcap.c lp.local.h globals.h

CFLAGS=		-O
LDFLAGS=	-n
LIBDIR=		${DESTDIR}/usr/lib
BINDIR=		${DESTDIR}/usr/bin
ETCDIR=		${DESTDIR}/etc
SPOOLDIR=	${DESTDIR}/usr/spool/lpd
SPOOLDIR2=
ERRF=		${DESTDIR}/usr/adm/lp.err
ACCF=		${DESTDIR}/usr/adm/lpacct
PCAP=		${ETCDIR}/printcap

ROOT=		root
DAEMON=		daemon
SYSGRP=		sys
BIN=		bin

all:		lpd lpr lpq lprm lpdrestart ulf ln01of ln03of lqf

sources:	${SOURCES}

${SOURCES}:
		sccs get $@

lpd:		lpd.o lpdchar.o printcap.o
		${CC} -o lpd ${LDFLAGS} lpd.o lpdchar.o printcap.o

lpr:		lpr.o printcap.o
		${CC} -o lpr ${LDFLAGS} lpr.o printcap.o

lpq:		lpq.o printcap.o
		${CC} -o lpq ${LDFLAGS} lpq.o printcap.o

lprm:		lprm.o printcap.o
		${CC} -o lprm ${LDFLAGS} lprm.o printcap.o

lpdrestart:	lpdrestart.o
		${CC} -o lpdrestart ${LDFLAGS} lpdrestart.o

ulf:		ulf.o printcap.o
		${CC} -o ulf ${CFLAGS} ${LDFLAGS} ulf.o printcap.o

lqf:		lqf.o printcap.o
		${CC} -o lqf ${CFLAGS} ${LDFLAGS} lqf.o printcap.o

ln01of:		ln01of.o printcap.o
		${CC} -o ln01of ${CFLAGS} ${LDFLAGS} ln01of.o printcap.o

ln03of:		ln03of.o printcap.o
		${CC} -o ln03of -DLN03of ${CFLAGS} ${LDFLAGS} ln03of.o printcap.o

ln03of.o:	lp.local.h globals.h
ln01of.o:	lp.local.h globals.h
lqf.o:		lp.local.h globals.h
ulf.o:		lp.local.h globals.h
lpr.o:		lp.local.h
lpd.o:		lp.local.h
lpq.o:		lp.local.h
lprm.o:		lp.local.h
lpdrestart.o:	lp.local.h


install:	all
	cp lpd ${LIBDIR}/lpd
	-strip ${LIBDIR}/lpd
	chown ${ROOT} ${LIBDIR}/lpd
	chgrp ${DAEMON} ${LIBDIR}/lpd
	chmod 6711 ${LIBDIR}/lpd

	cp lpr ${BINDIR}/lpr
	-strip ${BINDIR}/lpr
	chown ${ROOT} ${BINDIR}/lpr
	chgrp ${DAEMON} ${BINDIR}/lpr
	chmod 6711 ${BINDIR}/lpr

	cp ulf ${LIBDIR}/ulf
	-strip ${LIBDIR}/ulf
	chog ${BIN} ${LIBDIR}/ulf
	chmod 755 ${LIBDIR}/ulf

	cp lqf ${LIBDIR}/lqf
	-strip ${LIBDIR}/lqf
	chog ${BIN} ${LIBDIR}/lqf
	chmod 755 ${LIBDIR}/lqf

	cp ln01of ${LIBDIR}/ln01of
	-strip ${LIBDIR}/ln01of
	chog ${BIN} ${LIBDIR}/ln01of
	chmod 755 ${LIBDIR}/ln01of

	cp ln03of ${LIBDIR}/ln03of
	-strip ${LIBDIR}/ln03of
	chog ${BIN} ${LIBDIR}/ln03of
	chmod 755 ${LIBDIR}/ln03of

	cp lpq ${BINDIR}/lpq
	-strip ${BINDIR}/lpq
	chown ${ROOT} ${BINDIR}/lpq
	chgrp ${DAEMON} ${BINDIR}/lpq
	chmod 2711 ${BINDIR}/lpq

	cp lprm ${BINDIR}/lprm
	-strip ${BINDIR}/lprm
	chown ${ROOT} ${BINDIR}/lprm
	chgrp ${DAEMON} ${BINDIR}/lprm
	chmod 6711 ${BINDIR}/lprm

	cp lpdrestart ${ETCDIR}/lpdrestart
	-strip ${ETCDIR}/lpdrestart
	chown ${ROOT} ${ETCDIR}/lpdrestart
	chgrp ${SYSGRP} ${ETCDIR}/lpdrestart
	chmod 4710 ${ETCDIR}/lpdrestart

# rm -f ${LIBDIR}/lp
# ln ${LIBDIR}/ulf ${LIBDIR}/lp

	echo lpr -p $$\* > ${BINDIR}/print
	chog ${BIN} ${BINDIR}/print
	chmod 755 ${BINDIR}/print

	chown ${DAEMON} ${SPOOLDIR} ${SPOOLDIR2}
	chgrp ${DAEMON} ${SPOOLDIR} ${SPOOLDIR2}
	chmod 755 ${SPOOLDIR} ${SPOOLDIR2}

	cat /dev/null >> ${ACCF}
	cat /dev/null >> ${ERRF}
	chown ${DAEMON} ${ACCF} ${ERRF}
	chgrp ${DAEMON} ${ACCF} ${ERRF}
	chmod 644 ${ACCF} ${ERRF}

	-(if [ ! -s ${PCAP} ] ; \
	then \
		touch ${PCAP}; \
	else \
		true ;\
	fi)
	chmod 644 ${PCAP}
	chog ${ROOT} ${PCAP}
	make clean

clean:
	rm -f *.o log
	rm -f lprm lpq lpd lpdrestart lpr ulf ln01of ln03of lqf

clobber:	clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
