# SCCSID: @(#)makefile	3.0	(ULTRIX-11)	3.0"
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES=gethbyname.c gethbyaddr.c gethostent.c \
	getnetbyaddr.c getnetent.c getnetbyname.c \
	getproto.c getprotoent.c getprotoname.c \
	getservent.c getsbyport.c getsbyname.c \
	rcmd.c rexec.c ruserpass.c recv.c send.c \
	setruid.c seteuid.c setrgid.c setegid.c \
	inet_addr.c inet_lnaof.c inet_mkaddr.c inet_netof.c \
	inet_network.c inet_ntoa.c \
	bcmp.c bcopy.c bzero.s hton.s ffs.s \
	syslog.c mkdir.c rmdir.c rename.c

OBJECTS=gethbyname.o gethbyaddr.o gethostent.o \
	getnetbyaddr.o getnetent.o getnetbyname.o \
	getproto.o getprotoent.o getprotoname.o \
	getservent.o getsbyport.o getsbyname.o \
	rcmd.o rexec.o ruserpass.o recv.o send.o \
	setruid.o seteuid.o setrgid.o setegid.o \
	inet_addr.o inet_lnaof.o inet_mkaddr.o inet_netof.o \
	inet_network.o inet_ntoa.o \
	bcmp.o bcopy.o bzero.o hton.o ffs.o \
	syslog.o mkdir.o rmdir.o rename.o

CFLAGS=	-O

all: ${OBJECTS}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

archive: ${OBJECTS}
	ar r ../${LIB} ${OBJECTS}

clean:
	rm -f *.o

clobber:
	rm -f *.o; sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
