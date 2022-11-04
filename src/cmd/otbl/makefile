# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS=-O

OBJECTS= t0.o t1.o t2.o t3.o t4.o t5.o t6.o t7.o t8.o t9.o tb.o tc.o te.o tf.o \
	tg.o ti.o tm.o ts.o tt.o tu.o tv.o

SOURCES= t0.c t1.c t2.c t3.c t4.c t5.c t6.c t7.c t8.c t9.c tb.c tc.c te.c tf.c \
	tg.c ti.c tm.c ts.c tt.c tu.c tv.c t..c

BIN = ${DESTDIR}/usr/orphan/usr/bin

all:	tbl70 tbl40

tbl:	tbl70
	cp tbl70 tbl

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	cp tbl40 tbl70 ${BIN}
	-strip ${BIN}/tbl40 ${BIN}/tbl70
	cp ${BIN}/tbl70 ${BIN}/tbl
	chmod 755 ${BIN}/tbl70 ${BIN}/tbl40 ${BIN}/tbl
	chog bin ${BIN}/tbl70 ${BIN}/tbl40 ${BIN}/tbl
	rm -f *.o

cmp:	tbl
	cmp tbl ${BIN}/tbl

tbl40:	$(OBJECTS)
	cc -O ${OBJECTS} -o tbl40

tbl70:	$(OBJECTS)
	cc -i -O ${OBJECTS} -o tbl70

${OBJECTS}: t..c

clean:
	rm -f *.o
	rm -f tbl tbl40 tbl70

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
