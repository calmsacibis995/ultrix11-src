#	Scsid: @(#)makefile 3.0 4/22/86

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

CFLAGS=-O
LNAME = libPW.a

SOURCES=alloca.s any.c anystr.c bal.c cat.c clean.c curdir.c dname.c fatal.c \
	fdfopen.c giveup.c imatch.c index.c lockit.c logname.c move.c patoi.c \
	patol.c regcmp.c regex.c rename.c repeat.c repl.c satoi.c setsig.c \
	sname.c strend.c substr.c trnslat.c userdir.c userexit.c username.c \
	verify.c xalloc.c xcreat.c xlink.c xmsg.c xopen.c xpipe.c xunlink.c \
	xwrite.c zero.c zeropad.c

OBJECTS = anystr.o bal.o curdir.o fdfopen.o giveup.o \
	imatch.o index.o lockit.o logname.o move.o patoi.o \
	patol.o regcmp.o regex.o rename.o repeat.o repl.o satoi.o \
	setsig.o sname.o strend.o substr.o trnslat.o userdir.o \
	username.o verify.o any.o xalloc.o xcreat.o xlink.o \
	xopen.o xpipe.o xunlink.o xwrite.o xmsg.o alloca.o \
	cat.o dname.o fatal.o clean.o userexit.o zero.o zeropad.o

all:	${LNAME}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

${LNAME}: ${OBJECTS}
	rm -f ${LNAME}
	ar r ${LNAME} ${OBJECTS}
	ranlib ${LNAME}
	@echo ${LNAME} DONE

install: ${LNAME}
	cp ${LNAME} ${DESTDIR}/usr/lib/${LNAME}
	ranlib ${DESTDIR}/usr/lib/${LNAME}
	chmod 644 ${DESTDIR}/usr/lib/${LNAME}
	chog bin ${DESTDIR}/usr/lib/${LNAME}
	make clean

${OBJECTS}: ${SOURCES}

clean:
	rm -f ${LNAME} *.o a.out log core

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
