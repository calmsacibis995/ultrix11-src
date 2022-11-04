# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
SOURCES=  elc.c eli.c elp.h elp1.c elp2.c elp3.c elp4.c \
	cda_u.c cda_k.c cda_q.c  cda.h cda.c ccd.c

all: sources elp cda ccd elc eli

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	cp elp ${DESTDIR}/bin
	-strip  ${DESTDIR}/bin/elp
	chmod 4755 ${DESTDIR}/bin/elp
	chog root ${DESTDIR}/bin/elp

	cp ccd ${DESTDIR}/bin
	-strip  ${DESTDIR}/bin/ccd
	chmod 700 ${DESTDIR}/bin/ccd
	chog root ${DESTDIR}/bin/ccd

	cp cda ${DESTDIR}/bin
	-strip ${DESTDIR}/bin/cda
	chmod 755 ${DESTDIR}/bin/cda
	chog root ${DESTDIR}/bin/cda

	cp eli ${DESTDIR}/etc
	-strip ${DESTDIR}/etc/eli
	chmod 700 ${DESTDIR}/etc/eli
	chown bin ${DESTDIR}/etc/eli
	chgrp 0 ${DESTDIR}/etc/eli

	cp elc ${DESTDIR}/etc
	-strip ${DESTDIR}/etc/elc
	chmod 700 ${DESTDIR}/etc/elc
	chog root ${DESTDIR}/etc/elc

	make clean
	
cda:	cda_u.o cda_k.o cda_q.o cda.o 
	cc cda.o cda_u.o cda_k.o cda_q.o -o cda

cda_u.o:	cda_u.c cda.h
	cc -c -DKERNEL cda_u.c 

cda_k.o:	cda_k.c cda.h
	cc -c -DKERNEL cda_k.c 

cda_q.o:	cda_q.c cda.h /usr/include/sys/buf.h
	cc -c cda_q.c

cda.o:	cda.c cda.h
	cc -c cda.c 

ccd: ccd.o
	cc -o ccd ccd.o

ccd.o:
	cc -c -O ccd.c

elc: elc.o
	cc -o elc elc.o

elc.o:
	cc -c -O elc.c

eli: eli.o
	cc -o eli eli.o

eli.o:
	cc -c -O eli.c


elp: elp1.o elp2.o elp3.o elp4.o
	cc -n -o elp elp1.o elp2.o elp3.o elp4.o

elp1.o:
	cc -c -O elp1.c

elp2.o:
	cc -c -O elp2.c

elp3.o:
	cc -c elp3.c

elp4.o:
	cc -c elp4.c

clean:
	rm -f *.o log elp cda ccd elc eli

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
