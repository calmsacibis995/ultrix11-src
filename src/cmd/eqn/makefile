#	SCCSID: @(#)makefile	3.0	5/7/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	eqn makefile

CFLAGS = -O
YFLAGS = -d

SOURCES = e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
	 io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
	 pile.c shift.c size.c sqrt.c text.c eqnchar

OOBJS =  diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
	 io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
	 pile.o shift.o size.o sqrt.o text.o

OBJS = ${OOBJS} e.o

all: eqn eqnchar

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all eqnchar
	cp eqn ${DESTDIR}/usr/bin/eqn
	-strip ${DESTDIR}/usr/bin/eqn
	chmod 755 ${DESTDIR}/usr/bin/eqn
	chog bin ${DESTDIR}/usr/bin/eqn

	cp eqnchar ${DESTDIR}/usr/pub/eqnchar
	chmod 644 ${DESTDIR}/usr/pub/eqnchar
	chog bin ${DESTDIR}/usr/pub/eqnchar
	make clean

eqn:	${OBJS}
	cc -n ${CFLAGS} -o eqn ${OBJS} -ly

${OBJS}: e.h e.def ${SOURCES}

e.def:	  y.tab.h
	  -cmp -s y.tab.h e.def || cp y.tab.h e.def

y.tab.h:  e.o

clean:
	  rm -f *.o y.tab.h e.def eqn

clobber:  clean
	  sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
