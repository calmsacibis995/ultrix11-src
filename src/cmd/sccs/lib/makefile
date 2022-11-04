# Sccsid: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CC = cc
CFLAGS = -O

LNAME=comobj.a
LD = -ld
LDFLAGS =
#LDFLAGS = -x

SOURCES= chkid.c chksid.c del_ba.c  date_ba.c dodelt.c del_ab.c dofile.c \
	dohist.c doie.c dolist.c eqsid.c flushto.c newstats.c permiss.c \
	logname.c pf_ab.c date_ab.c rdmod.c setup.c sid_ab.c sid_ba.c \
	sidtoser.c sinit.c stats_ab.c fmterr.c getline.c putline.c auxf.c

OBJECTS=chkid.o chksid.o del_ba.o  date_ba.o dodelt.o del_ab.o dofile.o \
	dohist.o doie.o dolist.o eqsid.o flushto.o newstats.o permiss.o \
	logname.o pf_ab.o date_ab.o rdmod.o setup.o sid_ab.o sid_ba.o \
	sidtoser.o sinit.o stats_ab.o fmterr.o getline.o putline.o auxf.o

all: ${LNAME}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

${LNAME}: ${OBJECTS}
	-rm -f ${LNAME}
	ar r ${LNAME} ${OBJECTS}
	chog bin ${LNAME}
	chmod 644 ${LNAME}
	ranlib ${LNAME}
	@echo ${LNAME} DONE

clean:
	rm -f *.o a.out ${LNAME}

clobber: clean
	sccs clean

chkid.o:	chkid.c
chksid.o:	chksid.c
del_ba.o:	del_ba.c
date_ba.o:	date_ba.c
dodelt.o:	dodelt.c
del_ab.o:	del_ab.c
dofile.o:	dofile.c
dohist.o:	dohist.c
doie.o:		doie.c
dolist.o:	dolist.c
eqsid.o:	eqsid.c
flushto.o:	flushto.c
newstats.o:	newstats.c
permiss.o:	permiss.c
logname.o:	logname.c
pf_ab.o:	pf_ab.c
date_ab.o:	date_ab.c
rdmod.o:	rdmod.c
setup.o:	setup.c
sid_ab.o:	sid_ab.c
sid_ba.o:	sid_ba.c
sidtoser.o:	sidtoser.c
sinit.o:	sinit.c
stats_ab.o:	stats_ab.c
fmterr.o:	fmterr.c
getline.o:	getline.c
putline.o:	putline.c
auxf.o:		auxf.c

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
