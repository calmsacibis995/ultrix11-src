# SCCSID: @(#)makefile	3.0	(ULTRIX-11)	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# removed endopen.c and freopen.c: are now in fopen.c

SOURCES = getgrgid.c getgrnam.c getgrent.c getpass.c getpwnam.c \
	getpwuid.c getpwent.c fgetc.c fputc.c getchar.c putchar.c \
	popen.c fgets.c fputs.c getpw.c fseek.c ftell.c \
	rew.c fread.c fwrite.c system.c fopen.c fdopen.c scanf.c doscan.c \
	fprintf.c gets.c getw.c printf.c puts.c putw.c sprintf.c \
	ungetc.c filbuf.c setbuf.c setvbuf.c fltpr.s doprnt.s cvt.c ffltpr.s \
	strout.c flsbuf.c findiop.c clrerr.c data.c fstab.c \
	stdiom.h vprintf.c vfprintf.c vsprintf.c

OBJECTS = getgrgid.o getgrnam.o getgrent.o getpass.o getpwnam.o \
	getpwuid.o getpwent.o fgetc.o fputc.o getchar.o putchar.o \
	popen.o fgets.o fputs.o getpw.o fseek.o ftell.o \
	rew.o fread.o fwrite.o system.o fopen.o fdopen.o scanf.o doscan.o \
	fprintf.o gets.o getw.o printf.o puts.o putw.o sprintf.o \
	ungetc.o filbuf.o setbuf.o setvbuf.o fltpr.o doprnt.o cvt.o ffltpr.o \
	strout.o flsbuf.o findiop.o clrerr.o data.o fstab.o \
	vprintf.o vfprintf.o vsprintf.o

CFLAGS = -O

all: ${OBJECTS}

archive: ${OBJECTS}
	ar r ../${LIB} ${OBJECTS}

${SOURCES}:
	sccs get $@

sources: ${SOURCES}

clean:
	rm -f *.o

clobber:
	rm -f *.o; sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
