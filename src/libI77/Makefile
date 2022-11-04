# SCCSID: @(#)Makefile	3.0	(ULTRIX-11)	4/22/86"

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

SOURCES = backspace.c close.c dfe.c due.c endfile.c err.c \
	fmt.c fmtlib.c ftest.c iio.c inquire.c \
	ioinit.f ioprim.c lib.c lio.c lread.c \
	ltostr.c nio.c open.c rdfmt.c rewind.c rsfe.c sfe.c \
	sue.c uio.c util.c wrtfmt.c wsfe.c
HEADERS =  fio.h fmt.h lio.h ioprim.h

# Files which are not in this Makefile, but reside in this directory:
# stest.c pwd.c ctest.c dballoc.c 	

# NOTE: nio.o must come before lread.o for proper archive order
OBJECTS = backspace.o close.o dfe.o due.o endfile.o err.o \
	fmt.o fmtlib.o ftest.o iio.o inquire.o \
	ioinit.o ioprim.o lib.o lio.o nio.o lread.o \
	ltostr.o open.o rdfmt.o rewind.o rsfe.o sfe.o \
	sue.o uio.o util.o wrtfmt.o wsfe.o

CFLAGS = -O
ARCHIVE = libI77.a

all: ${ARCHIVE}

sources: ${SOURCES} ${HEADERS}

${SOURCES} ${HEADERS}:
	sccs get $@

${ARCHIVE}: ${OBJECTS}
	rm -f ${ARCHIVE}
	ar cr ${ARCHIVE} ${OBJECTS} ioinit.f
	ranlib ${ARCHIVE}

lio.o lread.o:	lio.h
ioprim.o:	ioprim.h
dfe.o fmt.o iio.o lread.o rdfmt.o rsfe.o wrtfmt.o wsfe.o: fmt.h
${OBJECTS}: fio.h

install: ${ARCHIVE}
	cp ${ARCHIVE} ${DESTDIR}/usr/lib/${ARCHIVE}
	chown bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chgrp bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/usr/lib/${ARCHIVE}
	ranlib ${DESTDIR}/usr/lib/${ARCHIVE}
	make clean

clean:
	rm -f *.o ${ARCHIVE} a.out core log

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
