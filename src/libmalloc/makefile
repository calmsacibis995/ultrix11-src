# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#
# makefile for libmalloc
#
# This is the fast malloc, found in libmalloc.a;
# We don't use the lint libraries associated with
# this malloc because it is not the default malloc,
# the one in /lib/libc.a is.
AR=ar
ARFLAGS=r
CC=cc
CFLAGS= -Y -O
INC= /usr/include
LIB= libmalloc.a
OBJECTS=malloc.o

all: ${LIB}

${LIB}: ${OBJECTS}
	${AR} ${ARFLAGS} ${LIB} ${OBJECTS}

${OBJECTS}: ${INC}/malloc.h mallint.h

.c.o:
	@echo $*.c:
	$(CC) $(CFLAGS) -c $*.c

llib-lmalloc.l:  llib-lmall.c
	cp llib-lmall.c llib-lmalloc.c
	lint -c llib-lmalloc.c
	rm llib-lmalloc.c

install: all
	cp libmalloc.a ${DESTDIR}/usr/lib/libmalloc.a
	ranlib ${DESTDIR}/usr/lib/libmalloc.a
	chog bin ${DESTDIR}/usr/lib/libmalloc.a
	chmod 644 ${DESTDIR}/usr/lib/libmalloc.a
#	cp llib-lmalloc.l ${DESTDIR}/usr/lib/llib-lmalloc.l
	make clean

clean: 
	rm -f ${OBJECTS} ${LIB} log core a.out

clobber: clean
	sccs clean

sccsinfo:
	sccs info

tags:
	true
