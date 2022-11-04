# SCCSID: @(#)Makefile	3.0	(ULTRIX-11)	4/22/86

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

DOBJECT = talkd.o announce.o process.o table.o print.o
TOBJECT = talk.o get_names.o display.o io.o ctl.o init_disp.o\
	  msgs.o get_addrs.o ctl_transact.o invite.o look_up.o
DSOURCE = talkd.c announce.c process.c table.c print.c
TSOURCE = talk.c get_names.c display.c io.c ctl.c init_disp.c\
	  msgs.c get_addrs.c ctl_transact.c invite.c look_up.c
INCLUDE = talk.h ctl.h talk_ctl.h
SOURCES = ${DSOURCE} ${TSOURCE} ${INCLUDE}
CFLAGS= -O

all:	talk talkd

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

talkd:	${DOBJECT}
	cc ${CFLAGS} -o talkd ${DOBJECT}

talk:	${TOBJECT}
	cc -o talk ${TOBJECT} -lcurses -ltermlib

${DOBJECT}: ctl.h
${TOBJECT}: talk.h ctl.h talk_ctl.h

install: Install clean

Install: talk talkd
	cp talk ${DESTDIR}/usr/ucb/talk
	strip ${DESTDIR}/usr/ucb/talk
	chmod 755 ${DESTDIR}/usr/ucb/talk
	chog bin ${DESTDIR}/usr/ucb/talk
	cp talkd ${DESTDIR}/usr/etc/talkd
	strip ${DESTDIR}/usr/etc/talkd
	chmod 755 ${DESTDIR}/usr/etc/talkd
	chog bin ${DESTDIR}/usr/etc/talkd

lint:
	lint ${DSOURCE}

clean:
	rm -f *.o talk talkd

clobber: clean
	sccsclean

sccsinfo:
	sccs info

tags:	${SOURCES}
	ctags ${SOURCES}
