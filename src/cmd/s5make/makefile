# SCCSID: @(#)makefile	3.1	12/1/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# Based on:	make.mk  1.7	(System V)
#
# The rules.c file can be modified locally for people who still like
#	things like fortran.

LIBS = -lld
SOURCES = defs main.c doname.c misc.c files.c rules.c dosys.c gram.y dyndep.c \
	prtmem.c
OBJECTS =  \
	main.o \
	doname.o \
	misc.o \
	files.o \
	rules.o \
	dosys.o \
	gram.o \
	dyndep.o \
	prtmem.o

CFLAGS = -O

all:  s5make40 s5make70

sources: ${SOURCES}

s5make40:  $(OBJECTS)
	$(CC) -n -o s5make40 $(OBJECTS)

s5make70:  $(OBJECTS)
	$(CC) -i -o s5make70 $(OBJECTS)

gram.c:	gram.y

gram.o: gram.c

$(OBJECTS):  defs

${SOURCES}:
	sccs get $@

install: all
	cp s5make40 s5make70 ${DESTDIR}/usr/bin
	-strip ${DESTDIR}/usr/bin/s5make40 ${DESTDIR}/usr/bin/s5make70
	chog bin ${DESTDIR}/usr/bin/s5make40 ${DESTDIR}/usr/bin/s5make70
	chmod 755 ${DESTDIR}/usr/bin/s5make40 ${DESTDIR}/usr/bin/s5make70
# for Bedrock:
	cp ${DESTDIR}/usr/bin/s5make70 ${DESTDIR}/usr/bin/s5make
	chog bin ${DESTDIR}/usr/bin/s5make
	chmod 755 ${DESTDIR}/usr/bin/s5make
	make clean

clean:
	rm -f $(OBJECTS) gram.c s5make40 s5make70 core a.out log

clobber: clean
	sccs clean

sccsinfo:
	sccs info

tags:	${SOURCES}
	ctags ${SOURCES}
