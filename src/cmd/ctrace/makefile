# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	makefile for ctrace, ctc, ctcr
# Based on:	ctrace.mk  1.3	(System V)

# installation directory
BIN = $(DESTDIR)/usr/bin

# directory for runtime.c source
LIB = $(DESTDIR)/usr/lib/ctrace

# preprocessor symbols
CC=cc
CFLAGS = -O -DLIB=\"$(LIB)
YFLAGS = -d

SOURCE	 = constants.h global.h main.c parser.y \
	   scanner.l lookup.c trace.c runtime.c ctcr.sh
OTHER	 = makefile
OBJECTS	 = main.o parser.o scanner.o lookup.o trace.o

all:	ctrace

ctrace: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

install: all
	cp ctrace ${BIN}/ctrace
	-strip ${BIN}/ctrace
	chog bin ${BIN}/ctrace
	chmod 755 ${BIN}/ctrace

	rm -f $(BIN)/ctc
	cp ctcr.sh ${BIN}/ctcr
	chog bin ${BIN}/ctcr
	chmod 755 ${BIN}/ctcr

	ln $(BIN)/ctcr $(BIN)/ctc

	-(if [ ! -d ${LIB} ] ;\
	then \
	    mkdir ${LIB} ;\
	else \
	    true ;\
	fi)
	cp runtime.c ${LIB}/runtime.c
	chog bin ${LIB}/runtime.c
	chmod 644 ${LIB}/runtime.c

	make clean

clean:
	rm -f ctrace *.o core log a.out y.tab.h y.output

clobber: clean
	sccs clean

tags:	${SOURCE}
	ctags ${SOURCE}

sccsinfo:
	sccs info
