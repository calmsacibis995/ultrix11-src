# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#  Chung-Wu - add OCSV so we don't save ovno otherwise the relative position
#             in stock will off 2

CFLAGS=	-O -DHZ=60.0
OCSV = /lib/v7csv.o
FPTERP = ../utilities/fpnofetchi/fp.o
STARTUP= ../utilities/assubs/pcrt0.o

ASRCS =	00int.s 02rel.s 02relset.s 03bool.s 04as.s 05lv.s  \
	06add.s 07sub.s 10mul.s 12div.s 13mod.s 14neg.s  \
	16dvd.s 17ind.s 17rv.s 20con.s 21rang.s 24case.s 24pxp.s 25set.s  \
	26for.s 27conv.s 30atof.s 30getname.s 30io.s 30iosubs.s 30read.s  \
	30write.s 34fun.s opcode.s wait.s
CSRCS =	exit.c int.c palloc.c pcttot.c pdattim.c perror.c pwrite.c
OSCRS =	getc.s putc.s ../utilities/assubs/pcrt0.s
SOURCES = ${ASRCS} ${CSRCS} ${OSRCS}
OBJS =	exit.o int.o palloc.o pcttot.o pdattim.o perror.o pwrite.o \
	getc.o putc.o

all:	px70 px40

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

px70:	as.o ${OBJS} ${STARTUP}
	ld -X -i -o px70 ${STARTUP} ${OCSV} as.o ${OBJS} -lm -lc

px40:	as.o ${OBJS} ${STARTUP}
	ld -X -o px40 ${STARTUP} ${OCSV} as.o ${OBJS} -lm -lc

getc.o: getc.s
	as -o getc.o /usr/include/sys.s getc.s

putc.o: putc.s
	as -o putc.o /usr/include/sys.s putc.s

as.o:	E.s ${ASRCS}
	rm -f 00head.s
	echo 'FP = 1' > 00head.s
	rm -f as.o
	as -o as.o /usr/include/sys.s 00head.s E.s ${ASRCS}

install: all
	cp px70 ${DESTDIR}/usr/bin/px70
	-strip ${DESTDIR}/usr/bin/px70
	chmod 755 ${DESTDIR}/usr/bin/px70
	chog bin ${DESTDIR}/usr/bin/px70
	cp px40 ${DESTDIR}/usr/bin/px40
	-strip ${DESTDIR}/usr/bin/px40
	chmod 755 ${DESTDIR}/usr/bin/px40
	chog bin ${DESTDIR}/usr/bin/px40
	cp ${DESTDIR}/usr/bin/px70 ${DESTDIR}/usr/bin/px
	chmod 755 ${DESTDIR}/usr/bin/px
	chog bin ${DESTDIR}/usr/bin/px
	make clean

int.o palloc.o pcttot.o perror.o: E.h

E.s E.h: Edata
	ed - Edata < Emake

../utilities/assubs/pcrt0.o: ../utilities/assubs/pcrt0.s
	(cd ../utilities/assubs; make pcrt0.o)

clean:
	rm -f a.out core E.h E.s 00head.s *.o px70 px40 px

clobber: clean
	sccs clean

tags:	${CSRC}
	ctags ${CSRCS}

sccsinfo:
	sccs info
