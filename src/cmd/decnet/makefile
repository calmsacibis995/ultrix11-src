# SCCSID: @(#)makefile	3.0	(ULTRIX-11)	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for ULTRIX-11 DECnet commands
#
SOURCES= dcat.c dcp.c dlogin.c dls.c drm.c subr.c
HEADERS= dgate.h
DLSOBJ= dls.o subr.o
DCATOBJ= dcat.o subr.o
DCPOBJ= dcp.o subr.o
DRMOBJ= drm.o subr.o
DLOGINOBJ= dlogin.o subr.o

COMMANDS= dcp dls dcat drm dlogin
D=${DESTDIR}/usr/bin

CFLAGS= -O

all: ${COMMANDS}

sources: ${SOURCES} ${HEADERS}

${SOURCES} ${HEADERS}:
	sccs get $@

dls.o dcp.o dcat.o drm.o dlogin.o subr.o: dgate.h

dcp: ${DCPOBJ}
	cc -o dcp -n ${DCPOBJ}

dls: ${DLSOBJ}
	cc -o dls -n ${DLSOBJ}

dcat: ${DCATOBJ}
	cc -o dcat -n ${DCATOBJ}

dlogin: ${DLOGINOBJ}
	cc -o dlogin -n ${DLOGINOBJ} -ljobs

drm: ${DRMOBJ}
	cc -o drm -n ${DRMOBJ}

install: all
	cp dcp dls dcat drm dlogin ${D}
	-strip ${D}/dcp ${D}/dls ${D}/dlogin ${D}/dcat ${D}/drm
	chog guest ${D}/dcp ${D}/dls ${D}/dlogin ${D}/dcat ${D}/drm
	chmod 4755 ${D}/dcp ${D}/dls ${D}/dlogin ${D}/dcat ${D}/drm
	make clean

clean:
	rm -f *.o dls dcat drm dlogin a.out core
	rm -f dcp

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
