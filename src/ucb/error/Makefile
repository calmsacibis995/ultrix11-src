# SCCSID: @(#)Makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# Based on @(#)Makefile	1.4 (Berkeley) 5/4/82
#
#	error.h			error header file
#	errorinput.c		does all of the input, and canonicalizes errors
#	errorpi.c		canonicalizes pi errors
#	errormain.c		main
#	errorsubr.c		does grotty subroutine work
#	errorfilter.c		"greps" out unwanted messages
#	errortouch.c		touches all mentioned files
#

HDRS = error.h
PHDRS = $(HDRS)
SRCS = errormain.c errorinput.c errorpi.c errorsubr.c errorfilter.c errortouch.c
PSRCS = $(SRCS)
OBJS = errormain.o errorinput.o errorpi.o errorsubr.o errorfilter.o errortouch.o

DFLAGS =
CFLAGS =  -O $(DFLAGS)

LINTFLAGS = $(DFLAGS)

all: error

error:	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -n -o error

sources: ${SRCS}

${SRCS}:
	sccs get $@

$(OBJS): $(HDRS)

install: Install clean

Install: error
	cp error $(DESTDIR)/usr/ucb/error
	-strip $(DESTDIR)/usr/ucb/error
	chog bin ${DESTDIR}/usr/ucb/error
	chmod 755 ${DESTDIR}/usr/ucb/error

clean:
	rm -f error $(OBJS)

clobber: clean
	sccs clean

lint:	
	lint $(LINTFLAGS) $(SRCS)

psrcs:
	echo $(PHDRS) $(PSRCS)

print:
	print $(PHDRS) $(PSRCS)

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
