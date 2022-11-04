# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for tar/mdtar
#
# 06-Jan-86
#
#	Commands to make tar -
#
# To make "mdtar" available, create a link to "tar" in the execution
# directory of tar.
#
#
SOURCES= tar.c tar.h readtape.c writetape.c command.c

CFLAGS40=-O -DU11 -DTAR40
CFLAGS70=-O -DU11
LIBCo = fprintf.o fread.o fseek.o malloc.o printf.o cvt.o \
      atof.o ctime.o ecvt.o doscan.o flsbuf.o fwrite.o
OBJ70= tar70.o command70.o readtape70.o writetape70.o
OBJ40= tar40.o command40.o readtape40.o writetape40.o

all:	tar40 tar70

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

tar40:	${OBJ40}
	ar x /lib/libc.a ${LIBCo}
	ld -n -X -o tar40 /lib/crt0.o tar40.o \
		-Z writetape40.o -Z readtape40.o \
		-Z command40.o -Z ${LIBCo} -L -lc
	rm -f ${LIBCo}
	size tar40

tar70:	${OBJ70}
	cc -i -o tar70 tar70.o command70.o readtape70.o writetape70.o

${OBJ40}: ${SOURCES}
	cc -c ${CFLAGS40} `basename $@ 40.o`.c
	mv `basename $@ 40.o`.o $@

${OBJ70}: ${SOURCES}
	cc -c ${CFLAGS70} `basename $@ 70.o`.c
	mv `basename $@ 70.o`.o $@
	
install: all
	cp tar70 ${DESTDIR}/bin/sid/tar
	-strip ${DESTDIR}/bin/sid/tar
	chog bin ${DESTDIR}/bin/sid/tar
	chmod 755 ${DESTDIR}/bin/sid/tar

	cp tar40 ${DESTDIR}/bin/nsid/tar
	-strip ${DESTDIR}/bin/nsid/tar
	chog bin ${DESTDIR}/bin/nsid/tar
	chmod 755 ${DESTDIR}/bin/nsid/tar

	rm -f ${DESTDIR}/bin/tar ${DESTDIR}/bin/mdtar
	ln ${DESTDIR}/bin/sid/tar ${DESTDIR}/bin/tar
	ln ${DESTDIR}/bin/tar ${DESTDIR}/bin/mdtar
	make clean

clean:	forceit
	rm -f *.o tar40 tar70 a.out core log

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo: forceit
	sccs info

forceit:
