
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

# SCCSID: @(#)Makefile	3.0	4/22/86
#
# Based on: (ULTRIX-32)  Makefile  1.4  12/4/84
#
#	(4.2BSD)  Makefile  4.6  6/15/83
#
SOURCES = cmds.c cmdtab.c ftp.c getpass.c glob.c main.c ftp_var.h
OBJECTS = cmds.o cmdtab.o ftp.o getpass.o glob.o main.o
CFLAGS=	-O

all:	ftp

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

ftp:	${OBJECTS}
	${CC} ${OBJECTS} -o ftp -ljobs

ftp.o:	/usr/include/arpa/ftp.h ftp_var.h
cmds.o:	/usr/include/arpa/ftp.h ftp_var.h
cmdtab.o: ftp_var.h
ftp.o:	ftp_var.h
main.o: ftp_var.h

install: Install clean

Install: all
	cp ftp ${DESTDIR}/usr/ucb/ftp
	strip ${DESTDIR}/usr/ucb/ftp
	chmod 755 ${DESTDIR}/usr/ucb/ftp
	chog bin ${DESTDIR}/usr/ucb/ftp

clean:
	rm -f ftp *.o *.s errs core a.out t.?

clobber: clean
	sccs clean

sccsinfo:
	sccs info

tags:	${SOURCES}
	ctags ${SOURCES}
