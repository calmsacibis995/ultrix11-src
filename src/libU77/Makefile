#  SCCSID: @(#)Makefile	3.0	(ULTRIX-11)	4/22/86

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#  Makefile for libU77
#

SOURCES	= access_.c alarm_.c chdir_.c chmod_.c ctime_.c dtime_.c \
	etime_.c f77lid_.c fdate_.c fgetc_.c fork_.c fputc_.c \
	fseek_.c fstat_.c ftell_.c gerror_.c getarg_.c getc_.c \
	getcwd_.c getenv_.c getgid_.c getlog_.c getpid_.c getuid_.c \
	gmtime_.c hostnm_.c iargc_.c idate_.c ierrno_.c isatty_.c \
	itime_.c kill_.c link_.c loc_.c lstat_.c ltime_.c \
	perror_.c putc_.c qsort_.c signal_.c sleep_.c stat_.c \
	symlnk_.c system_.c tapeio.c time_.c ttynam_.c unlink_.c \
	wait_.c

OBJECTS	= access_.o alarm_.o chdir_.o chmod_.o ctime_.o dtime_.o \
	etime_.o f77lid_.o fdate_.o fgetc_.o fork_.o fputc_.o \
	fseek_.o fstat_.o ftell_.o gerror_.o getarg_.o getc_.o \
	getcwd_.o getenv_.o getgid_.o getlog_.o getpid_.o getuid_.o \
	gmtime_.o hostnm_.o iargc_.o idate_.o ierrno_.o isatty_.o \
	itime_.o kill_.o link_.o loc_.o lstat_.o ltime_.o \
	perror_.o putc_.o qsort_.o signal_.o sleep_.o stat_.o \
	symlnk_.o system_.o tapeio.o time_.o ttynam_.o unlink_.o \
	wait_.o

CFLAGS = -O
ARCHIVE = libU77.a

all:	${ARCHIVE}

sources: ${SOURCES}

${SOURCES}:	
	sccs get $@

${ARCHIVE}: ${OBJECTS}
	ar r ${ARCHIVE} ${OBJECTS}

install: forceit ${ARCHIVE}
	cp ${ARCHIVE} ${DESTDIR}/usr/lib
	ranlib ${DESTDIR}/usr/lib/${ARCHIVE}
	chog bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/usr/lib/${ARCHIVE}
	make clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info

forceit:

clean:	
	rm -f ${OBJECTS} ${ARCHIVE}

clobber: clean
	sccs clean
