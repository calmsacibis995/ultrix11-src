# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
V6LIB=/usr/src/libc/v6
CFLAGS=-O
SOURCES = tar.c

#only 1 version of the old tar (v7tar)
all:	tar40

tar:	all

tar70:	tar.c
	cc -i -O tar.c -o tar70

tar40:	tar.c
	cc -O tar.c -o tar40

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install:	all
#	cp tar70 ${DESTDIR}/bin/tar
#	cp tar70 ${DESTDIR}/bin/sid/tar
#	cp tar40 ${DESTDIR}/bin/nsid/tar
#	-strip ${DESTDIR}/bin/tar ${DESTDIR}/bin/sid/tar ${DESTDIR}/bin/nsid/tar
#	chmod 755 ${DESTDIR}/bin/tar ${DESTDIR}/bin/sid/tar ${DESTDIR}/bin/nsid/tar
#	chog bin ${DESTDIR}/bin/tar ${DESTDIR}/bin/sid/tar ${DESTDIR}/bin/nsid/tar
	cp tar40 ${DESTDIR}/bin/v7tar
	-strip ${DESTDIR}/bin/v7tar
	chog bin ${DESTDIR}/bin/v7tar
	chmod 755 ${DESTDIR}/bin/v7tar
	make clean

v6tar:	tar.o access.o chown.o execl.o ftime.o gtty.o lseek.o stat.o syscall.o time.o
	cc -i -s -O *.o -o v6tar

v6tar40:	tar.o access.o chown.o execl.o ftime.o gtty.o lseek.o stat.o syscall.o time.o
	cc -s -O *.o -o v6tar40

access.o:	$(V6LIB)/access.c
	cc -c -O $?

chown.o:	$(V6LIB)/chown.c
	cc -c -O $?

execl.o:	$(V6LIB)/execl.c
	cc -c -O $?

ftime.o:	$(V6LIB)/ftime.c
	cc -c -O $?

gtty.o:	$(V6LIB)/gtty.c
	cc -c -O $?

lseek.o:	$(V6LIB)/lseek.c
	cc -c -O $?

stat.o:	$(V6LIB)/stat.c
	cc -c -O $?

syscall.o:	$(V6LIB)/syscall.s
	cc -c -O $?

time.o:	$(V6LIB)/time.s
	cc -c -O $?

clean:
	rm -f *.o tar40 core a.out log

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
