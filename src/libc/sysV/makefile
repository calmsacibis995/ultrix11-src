# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# This makefile contains most of the new System V
# library routines that have been added to ULTRIX-11
# for System V compatibility.  All of these are new to
# ULTRIX-11.  For a look at the routines that clash,
# see /usr/src/libcV.

SOURCES = a64l.c l64a.c bsearch.c clock.c \
	  dial.c drand48.c ftok.c ftw.c \
	  getcwd.c getopt.c getpw.c getut.c hsearch.c \
	  lfind.c lockf.c lsearch.c \
	  memccpy.c memchr.c memcmp.c memcpy.c memset.c \
	  putenv.c ssignal.c strtod.c strtol.c \
	  strchr.c strrchr.c strpbrk.c strspn.c strcspn.c strtok.c \
	  tolower.c toupper.c _tolower.s _toupper.s tsearch.c \
	  cuserid.c ctermid.c tempnam.c tmpnam.c tmpfile.c

OBJECTS = a64l.o l64a.o bsearch.o clock.o \
	  dial.o drand48.o ftok.o ftw.o \
	  getcwd.o getopt.o getpw.o getut.o hsearch.o \
	  lfind.o lockf.o lsearch.o \
	  memccpy.o memchr.o memcmp.o memcpy.o memset.o \
	  putenv.o ssignal.o strtod.o strtol.o \
	  strchr.o strrchr.o strpbrk.o strspn.o strcspn.o strtok.o \
	  tolower.o toupper.o _tolower.o _toupper.o tsearch.o \
	  cuserid.o ctermid.o tempnam.o tmpnam.o tmpfile.o

CFLAGS = -O

all: ${OBJECTS}

archive: ${OBJECTS}
	ar r ../${LIB} ${OBJECTS}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

clean:
	rm -f *.o

clobber:
	rm -f *.o; sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
