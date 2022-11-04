SCCSID	=	"@(#)Makefile	3.0	4/22/86"

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

CFILES	=	link.c pass2.c in.c sup.c 
SOURCES	=	${CFILES} obint.c link.h
OBJS	=	link.o pass2.o in.o sup.o 
CFLAGS	=	-O 
LFLAGS	=	-n 

all: 		l11

sources:	${SOURCES}

${SOURCES}:
		sccs get $@

install:	l11
		cp l11 ${DESTDIR}/usr/ucb/l11
		-strip ${DESTDIR}/usr/ucb/l11
		chog bin ${DESTDIR}/usr/ucb/l11
		chmod 755 ${DESTDIR}/usr/ucb/l11
		make clean

l11:		${OBJS}
		cc -o l11 ${LFLAGS} link.o pass2.o in.o sup.o 

lint:	
		lint link.c pass2.c in.c sup.c > FLUFF

obint:		obint.o in.o sup.o
		cc -o obint obint.o in.o sup.o

clean:
		rm -f *.o obint l11

clobber:	clean
		sccs clean

list:
		pr link.h link.c pass2.c in.c sup.c obint.c | lpr

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
