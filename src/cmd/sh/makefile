# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS = -B -O

SOURCES=ctype.h defs.h dup.h mac.h mode.h name.h stak.h sym.h timeout.h \
	setbrk.c builtin.c blok.c stak.c cmd.c fault.c main.c word.c string.c \
	name.c args.c xec.c service.c error.c io.c print.c macro.c expand.c \
	ctype.c msg.c brkincr.h

OBJECTS=setbrk.o builtin.o blok.o stak.o cmd.o fault.o main.o word.o string.o \
	name.o args.o xec.o service.o error.o io.o print.o macro.o expand.o \
	ctype.o msg.o

all:	sh

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	@/bin/sh -c "if [ ${DESTDIR}x = x ]; \
	then echo sh MUST BE INSTALLED IN /bin/sh MANUALLY; exit 1; \
	else exit 0; fi"
	-cp sh ${DESTDIR}/bin/sh
	-strip ${DESTDIR}/bin/sh
	chog bin ${DESTDIR}/bin/sh
	chmod 755 ${DESTDIR}/bin/sh
	make clean

sh:	${OBJECTS}
	cc -o sh -n ${OBJECTS}

args.o:
	cc -S ${CFLAGS} args.c
	sed 's/.text/.data/' < args.s >args.s2
	as -u -V -o args.o args.s2
	@rm -f args.s args.s2

clean:
	rm -f *.o
	rm -f sh

clobber: clean
	sccs clean

HFILES = defs.h mac.h mode.h name.h stak.h brkincr.h ctype.h
args.o blok.o cmd.o ctype.o:	${HFILES}
error.o expand.o fault.o io.o:	${HFILES}
macro.o main.o msg.o name.o:	${HFILES}
print.o service.o setbrk.o:	${HFILES}
stak.o string.o word.o xec.o:	${HFILES}

cmd.o macro.o main.o msg.o:	sym.h
word.o xec.o:			sym.h

io.o main.o:			dup.h
main.o:				timeout.h

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
