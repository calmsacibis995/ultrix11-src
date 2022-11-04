# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# make file for tip device drivers
#
# Current drivers:
#	BIZCOMP
#	DEC DF02-AC, DF03-AC
#	DEC DN-11/Able Quadracall
#	VENTEL 212+ (w/o echo)
#	VADIC 831 RS232 adaptor
#	VADIC 3451
#	DEC DF112, DF224

CFLAGS=	-O -I../
OBJS=	biz22.o biz31.o df.o dff.o dn11.o ventel.o v831.o v3451.o
SOURCES = biz22.c biz31.c df.c dff.c dn11.c ventel.c v831.c v3451.c

all:	aculib.a

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all

aculib.a: ${OBJS}
	ar cu aculib.a *.o
	ranlib aculib.a

${OBJS}: ../tip.h

../tip.h:
	cd ..; sccs get tip.h

clean:
	rm -f *.o core errs a.out aculib.a

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
