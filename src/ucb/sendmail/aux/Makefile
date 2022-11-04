# SCCSID: @(#)Makefile	3.1	(ULTRIX-11)	10/20/87

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

# Based on:  @(#)Makefile	1.3	(ULTRIX-32)	9/23/84
#
#  Makefile for assorted programs related (distantly) to Sendmail.
#
#	Version:
#		@(#)Makefile.m4	4.1		7/25/83
#

ALL=	logger   mconnect   syslog   vacation
SRCS=	logger.c mconnect.c syslog.c vacation.c

LIBS=	-ldbm
DBMLIB=	-ldbm
CONVTIME=../src/convtime.o
DESTROOT=

CHOWN=	-echo chown
CHMOD=	chmod
O=	-O
COPTS=
#CCONFIG=-I../include -DDBM -DDEBUG -DLOG -DVMUNIX
CCONFIG=-I../include -DDBM -DDEBUG -DLOG
CFLAGS=	$O $(COPTS) $(CCONFIG)
ASMSED=	../include/asm.sed
AR=	-ar
ARFLAGS=rvu
LINT=	lint
XREF=	ctags -x
CP=	cp
MV=	mv
INSTALL=install -c -s
M4=	m4
TOUCH=	touch
ABORT=	false

GET=	sccs get
DELTA=	sccs delta
WHAT=	sccs what
PRT=	sccs prt
REL=

ROOT=	root
OBJMODE=755

.c.o:
	cc -S $(CFLAGS) $*.c
	sed -f $(ASMSED) $*.s > tmp.s
	as - -V -o $*.o tmp.s
	rm -f $*.s tmp.s

all: $(ALL)

install: all

logger: logger.o
	cc $(COPTS) -o $@ $*.o $(LIBS)

mconnect: mconnect.o
	cc $(COPTS) -o $@ $*.o

praliases: praliases.o
	cc $(COPTS) -o $@ $*.o

syslog: syslog.o
	cc $(COPTS) -o $@ $*.o

vacation: vacation.o
	cc $(COPTS) $(DBMLIB) -o $@ $*.o $(CONVTIME)

sources: $(SRCS)

$(SRCS):
	$(GET) $(REL) SCCS/s.$@

install: all
	cp syslog ${DESTDIR}/usr/etc/syslog
	-strip ${DESTDIR}/usr/etc/syslog
	chog bin ${DESTDIR}/usr/etc/syslog
	chmod 755 ${DESTDIR}/usr/etc/syslog

clean:
	rm -f *.o $(ALL) core a.out make.out lint.out

clobber: clean
	sccs clean

tags:
	true
sccsinfo:
	sccs info
sccsget:
	sccs get SCCS
