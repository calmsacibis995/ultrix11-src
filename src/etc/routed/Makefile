# SCCSID: @(#)Makefile	3.0	(ULTRIX-11)	4/24/86

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	Based on @(#)Makefile	5.1 (Berkeley) 6/4/85
#
SOURCES= af.c af.h aux.c defs.h if.c inet.c input.c interface.h \
	main.c output.c protocol.h startup.c table.h tables.c \
	timer.c trace.c trace.h
OBJS=	af.o if.o input.o main.o output.o startup.o tables.o timer.o \
	trace.o inet.o aux.o
CFLAGS= -O
LDFLAGS= -n

all:	routed

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

routed: ${OBJS}
	${CC} -o routed ${LDFLAGS} ${OBJS} -ljobs

install: Install clean

Install: routed
	cp routed ${DESTDIR}/usr/etc
	strip ${DESTDIR}/usr/etc/routed
	chog bin ${DESTDIR}/usr/etc/routed
	chmod 755 ${DESTDIR}/usr/etc/routed

clean:
	rm -f routed *.o core a.out t.c linterrs typescript errs log
	cd tools; make clean

clobber:
	rm -f routed *.o core a.out t.c linterrs typescript errs log
	sccs clean
	cd tools; make clobber

sccsinfo:
	sccs info
	cd tools; make sccsinfo

tags:	${SOURCES}
	ctags ${SOURCES}
	cd tools; make tags


DEFS_H = /usr/include/sys/types.h /usr/include/sys/socket.h \
	/usr/include/net/route.h /usr/include/netinet/in.h \
	/usr/include/stdio.h /usr/include/netdb.h defs.h protocol.h \
	trace.h interface.h table.h af.h

af.c: ${DEFS_H}
if.c: ${DEFS_H}
inet.c: ${DEFS_H}
input.c: ${DEFS_H}
main.c: ${DEFS_H}
main.c: /usr/include/sys/ioctl.h
main.c: /usr/include/net/if.h
main.c: /usr/include/errno.h
main.c: /usr/include/nlist.h
main.c: /usr/include/signal.h
main.c: /usr/include/syslog.h
output.c: ${DEFS_H}
startup.c: ${DEFS_H}
startup.c: /usr/include/sys/ioctl.h
startup.c: /usr/include/net/if.h
startup.c: /usr/include/nlist.h
startup.c: /usr/include/syslog.h
tables.c: ${DEFS_H}
tables.c: /usr/include/sys/ioctl.h
tables.c: /usr/include/errno.h
tables.c: /usr/include/syslog.h
timer.c: ${DEFS_H}
trace.c: ${DEFS_H}
