# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS= -O
SOURCES= restor.c

all:	restor 512restor

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

restor:	restor70 restor40


restor40:	restor.c
	cc -O -DRESTOR40 restor.c -o restor40 -lv7

restor70:	restor.c
	cc -i -O restor.c -o restor70

512restor:	restor.c
	cc -O -DRESTOR40 -DREST_512 restor.c -o 512restor -lv7


install: all
	rm -f ${DESTDIR}/bin/sid/restor ${DESTDIR}/bin/nsid/restor
	cp 512restor ${DESTDIR}/bin/512restor
	cp restor70 ${DESTDIR}/bin/sid/restor
	cp restor40 ${DESTDIR}/bin/nsid/restor
	-strip ${DESTDIR}/bin/sid/restor \
		${DESTDIR}/bin/nsid/restor ${DESTDIR}/bin/512restor 
	chmod 755 ${DESTDIR}/bin/sid/restor \
		${DESTDIR}/bin/nsid/restor ${DESTDIR}/bin/512restor 
	chog bin ${DESTDIR}/bin/sid/restor \
		${DESTDIR}/bin/nsid/restor ${DESTDIR}/bin/512restor 
	-(if [ -f ${DESTDIR}/bin/restor ] ;\
	then \
		rm -f ${DESTDIR}/bin/restor ;\
	else \
		true ;\
	fi)
	ln ${DESTDIR}/bin/sid/restor ${DESTDIR}/bin/restor 
	make clean

clean:
	rm -f *.o
	rm -f restor40 restor70 restor 512restor 

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
