# SCCSID: @(#)Makefile	3.0	ULTRIX-11	4/22/86

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

#
#  Makefile for Sendmail library
#
# Based on:
#	@(#)Makefile.m4	3.6		2/9/83
#

ALL=	newsyslog.sh
SRCS=	newsyslog.sh

#CCONFIG=-DVMUNIX
CCONFIG=
CFLAGS=	-O -I../include $(CCONFIG) -DEBUG
ASMSED=	../include/asm.sed

.c.o:
	cc -S ${CFLAGS} $*.c
	sed -f $(ASMSED) $*.s > tmp.s
	as - -V -o $*.o tmp.s
	rm -f $*.s tmp.s

all: $(ALL)

sources: $(SRCS)

$(SRCS):
	sccs get $@

install: all
	cp newsyslog.sh ${DESTDIR}/usr/adm/newsyslog
	chmod 744 ${DESTDIR}/usr/adm/newsyslog
	chown daemon ${DESTDIR}/usr/adm/newsyslog
	chgrp other ${DESTDIR}/usr/adm/newsyslog

clean:
	rm -f *.o core a.out log

clobber: clean
	sccs clean

sccsinfo:
	sccs info

tags:
	true

