# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	neqn makefile

CFLAGS = -O -DNEQN
YFLAGS = -d
SOURCES = e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
	 io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
	 pile.c shift.c size.c sqrt.c text.c
OBJS =  diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
	 io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
	 pile.o shift.o size.o sqrt.o text.o e.o

all:	neqn

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: neqn
	cp neqn ${DESTDIR}/usr/bin/neqn
	-strip ${DESTDIR}/usr/bin/neqn
	chmod 755 ${DESTDIR}/usr/bin/neqn
	chog bin ${DESTDIR}/usr/bin/neqn
	make clean

neqn:	$(OBJS)
	cc -n ${CFLAG} -o neqn $(OBJS) -ly

$(OBJS): e.h e.def $(SOURCES)

e.def:    y.tab.h
	  -cmp -s y.tab.h e.def || cp y.tab.h e.def

y.tab.h:  e.o

listing:
	pr ${SOURCES} mkaeifle

clean:
	rm -f *.o y.tab.[ch] e.def
	rm -f neqn

clobber:  clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
