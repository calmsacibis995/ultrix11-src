
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

#
# SCCSID:  @(#)makefile	3.0	4/22/86
#
# OBJS=	host.o inet.o if.o main.o mbuf.o route.o
SOURCES= host.c if.c inet.c main.c mbp.c mbuf.c mpb2.c route.c
OBJS=	inet.o if.o main.o mbuf.o route.o
CFLAGS=-O

all: netstat

netstat: ${OBJS}
	${CC} ${OBJS} -o netstat

install: netstat
	cp netstat ${DESTDIR}/usr/ucb/netstat
	-strip ${DESTDIR}/usr/ucb/netstat
	chmod 755 ${DESTDIR}/usr/ucb/netstat
	chog bin ${DESTDIR}/usr/ucb/netstat
	make clean

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

clean:
	rm -f *.o core a.out errs netstat

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
