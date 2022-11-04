# SCCSID: @(#)makefile	3.0	(ULTRIX-11)	4/22/86

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for libape.a;
# Arbitrary Precision Integer Arithmetic Routines
#

# shortran.o for compatibility with two byte integer F77 libraries
#FORTOBJ=	fortran.o
FORTOBJ=	shortran.o

# shortran.c for compatibility with two byte integer F77 libraries
#FORTSRC=	fortran.c
FORTSRC=	shortran.c

OBJS=		shift.o pointers.o msqrt.o $(FORTOBJ) gcd.o pow.o mout.o \
		mdiv.o util2.o min.o mult.o madd.o square.o util.o

SOURCES=	shift.c pointers.c msqrt.c $(FORTSRC) gcd.c pow.c mout.c \
		mdiv.c util2.c min.c mult.c madd.c square.c util.c

LIB=		libape.a

INCLUDE=	/usr/include/ape.h

DEFS=		-DI2		# to change this you must remake F77 also

# use old C compiler (-B)
CFLAGS=		-O -B $(DEFS)

LFLAGS=

all: 	${LIB}

${LIB}:	$(OBJS)
	rm -f ${LIB}
	ar r  $(LIB) $(OBJS)
	ranlib $(LIB)

install: all
	cp $(LIB) $(DESTDIR)/usr/lib/$(LIB)
	ranlib $(DESTDIR)/usr/lib/$(LIB)
	chog bin $(DESTDIR)/usr/lib/$(LIB)
	chmod 644 $(DESTDIR)/usr/lib/$(LIB)
	make clean

$(OBJS): $(INCLUDE)

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

clean:
	rm -f $(OBJS) $(LIB)

sccsinfo:
	sccs info

tags:	$(SOURCES)
	ctags $(SOURCES)
