# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES = t..c t[0-9].c t[bcefgimrstuv].c
OBJS = t0.o t1.o t2.o t3.o t4.o t5.o t6.o t7.o t8.o t9.o tb.o tc.o\
	te.o tf.o tg.o ti.o tm.o tr.o ts.o tt.o tu.o tv.o

all: tbl40 tbl70

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	cp tbl40 ${DESTDIR}/usr/bin/tbl40
	cp tbl70 ${DESTDIR}/usr/bin/tbl70
	-strip ${DESTDIR}/usr/bin/tbl40
	-strip ${DESTDIR}/usr/bin/tbl70
	cp ${DESTDIR}/usr/bin/tbl70 ${DESTDIR}/usr/bin/tbl
	chmod 755 ${DESTDIR}/usr/bin/tbl ${DESTDIR}/usr/bin/tbl40 ${DESTDIR}/usr/bin/tbl70
	chog bin ${DESTDIR}/usr/bin/tbl ${DESTDIR}/usr/bin/tbl40 ${DESTDIR}/usr/bin/tbl70
	make clean

tbl40:	${OBJS}
	cc -O ${OBJS} -o tbl40

tbl70:	${OBJS}
	cc -i -O ${OBJS} -o tbl70

clean:
	rm -f *.o
	rm -f tbl40 tbl70

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
