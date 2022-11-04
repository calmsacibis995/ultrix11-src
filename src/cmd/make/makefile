# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# Description file for the Make command

SOURCES= ident.c defs main.c doname.c\
	misc.c files.c dosys.c gram.y
OBJECTS = ident.o main.o doname.o misc.o files.o dosys.o gram.o 
LIBES= 
CFLAGS = -O

all:	make40 make70

make:	 all

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

${OBJECTS}:  defs

make40: ${OBJECTS}
	${CC} -n ${CFLAGS} ${OBJECTS} ${LIBES} -o make40

make70:  ${OBJECTS}
	${CC} -i ${CFLAGS} ${OBJECTS} ${LIBES} -o make70

install: all
	rm -f ${DESTDIR}/bin/nsid/make ${DESTDIR}/bin/sid/make
	cp make40 ${DESTDIR}/bin/nsid/make
	cp make70 ${DESTDIR}/bin/sid/make
	-strip ${DESTDIR}/bin/sid/make ${DESTDIR}/bin/nsid/make
	chog bin ${DESTDIR}/bin/sid/make ${DESTDIR}/bin/nsid/make
	chmod 755 ${DESTDIR}/bin/sid/make ${DESTDIR}/bin/nsid/make
	-(if [ -f ${DESTDIR}/bin/make ] ;\
	then \
		mv ${DESTDIR}/bin/make ${DESTDIR}/bin/make.junk ;\
	else \
		true ;\
	fi)
	ln ${DESTDIR}/bin/sid/make ${DESTDIR}/bin/make
	make clean

clean:
	-rm -f *.o gram.c
	-rm -f make40 make70

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
