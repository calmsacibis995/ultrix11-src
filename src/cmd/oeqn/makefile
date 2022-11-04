# SCCSID="@(#)makefile 3.0 4/22/86"
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS=-O
YFLAGS=-d

SOURCES = e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
 io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
 pile.c shift.c size.c sqrt.c text.c eqnchar

OBJS= e.o diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
 io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
 pile.o shift.o size.o sqrt.o text.o

all:	eqn eqnchar

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all eqnchar
	cp eqn ${DESTDIR}/usr/orphan/usr/bin/eqn
	-strip ${DESTDIR}/usr/orphan/usr/bin/eqn
	chmod 755 ${DESTDIR}/usr/orphan/usr/bin/eqn
	chog bin ${DESTDIR}/usr/orphan/usr/bin/eqn

	cp eqnchar ${DESTDIR}/usr/orphan/usr/pub/eqnchar
	chmod 644 ${DESTDIR}/usr/orphan/usr/pub/eqnchar
	chog bin ${DESTDIR}/usr/orphan/usr/pub/eqnchar
	make clean

cmp:	eqn
	cmp eqn ${DESTDIR}/usr/orphan/usr/bin/eqn

eqn:	${OBJS}
	cc -n ${CFLAGS} ${OBJS} -o eqn

e.c:	e.def

e.def:	e.y
	yacc -d e.y
	mv y.tab.c e.c
	mv y.tab.h e.def

${OBJS}:	e.h e.def ${SOURCES}

clean:
	rm -f *.o e.c e.def eqn

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
