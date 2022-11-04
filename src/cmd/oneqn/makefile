# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS=-O -DNEQN
YFLAGS=-d

all:	neqn

SOURCES = e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
 io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
 pile.c shift.c size.c sqrt.c text.c

OBJS = e.o diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
 io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
 pile.o shift.o size.o sqrt.o text.o

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install:	neqn
	cp neqn ${DESTDIR}/usr/orphan/usr/bin/neqn
	-strip ${DESTDIR}/usr/orphan/usr/bin/neqn
	chmod 755 ${DESTDIR}/usr/orphan/usr/bin/neqn 
	chog bin ${DESTDIR}/usr/orphan/usr/bin/neqn 
	make clean

cmp:	neqn
	cmp neqn ${DESTDIR}/usr/orphan/usr/bin/neqn

neqn:	$(OBJS)
	cc -n $(CFLAGS) $(OBJS) -o neqn

e.c:	e.def

e.def:	e.y
	yacc -d e.y
	mv y.tab.c e.c
	mv y.tab.h e.def

$(OBJS):	$(SOURCES) e.def

list:
	pr TODO $(SOURCES) makefile

gcos:	y.tab.c
	fsend e.h e.y *.c

clean:
	rm -f *.o e.c e.def
	rm -f neqn

clobber: clean
	sccs clean

lint:	y.tab.c
	lint -spb *.c

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
