# SCCSID: @(#)makefile	3.0	(ULTRIX-11)	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = cuexit.s execvp.c getenv.c getlogin.c perror.c sleep.c \
	timezone.c ttyslot.c ttyname.c abort.s abs.c atof.c atoi.c \
	atol.c crypt.c ctime.c calloc.c malloc.c ecvt.c errlst.c \
	fakcu.s fakfp.s frexp11.s isatty.c l3.c ldexp11.s ldfps.s \
	mktemp.c modf11.s mon.c nlist.c qsort.c rand.c setjmp.s \
	stty.c swab.c tell.c ctype_.c index.c rindex.c strcat.c \
	strncat.c strcmp.c strncmp.c strcpy.c strncpy.c strlen.c \
	regex.c getwd.c

OBJECTS = cuexit.o execvp.o getenv.o getlogin.o perror.o sleep.o \
	timezone.o ttyslot.o ttyname.o abort.o abs.o atof.o atoi.o \
	atol.o crypt.o ctime.o calloc.o malloc.o ecvt.o errlst.o \
	fakcu.o fakfp.o frexp11.o isatty.o l3.o ldexp11.o ldfps.o \
	mktemp.o modf11.o mon.o nlist.o qsort.o rand.o setjmp.o \
	stty.o swab.o tell.o ctype_.o index.o rindex.o strcat.o \
	strncat.o strcmp.o strncmp.o strcpy.o strncpy.o strlen.o \
	regex.o getwd.o

CFLAGS = -O

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
