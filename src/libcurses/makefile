# SCCSID =@(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for libcurses.a
#
SOURCES = unctrl.h cr_ex.h curses.ext \
	box.c clear.c initscr.c endwin.c mvprintw.c mvscanw.c mvwin.c \
	newwin.c overlay.c overwrite.c printw.c scanw.c refresh.c \
	touchwin.c erase.c clrtobot.c clrtoeol.c cr_put.c cr_tty.c \
	longname.c delwin.c insertln.c deleteln.c scroll.c getstr.c \
	getch.c addstr.c addch.c move.c curses.c unctrl.c standout.c \
	insch.c delch.c tstp.c curtest.c show.c

OBJECTS = box.o clear.o initscr.o endwin.o mvprintw.o mvscanw.o mvwin.o \
	newwin.o overlay.o overwrite.o printw.o scanw.o refresh.o \
	touchwin.o erase.o clrtobot.o clrtoeol.o cr_put.o cr_tty.o \
	longname.o delwin.o insertln.o deleteln.o scroll.o getstr.o \
	getch.o addstr.o addch.o move.o curses.o unctrl.o standout.o \
	insch.o delch.o tstp.o

ARCHIVE = libcurses.a

CFLAGS=	-O

all: ${ARCHIVE}

${ARCHIVE}: ${OBJECTS}
	rm -f ${ARCHIVE}
	ar cr ${ARCHIVE} ${OBJECTS}
	ranlib ${ARCHIVE}

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

# TEST PROGRAMS:
#  curtest is for testing non-destructive overlay windows
curtest:	libcurses.a curtest.c
	cc -O curtest.c -o curtest libcurses.a -ltermlib
	./curtest

#  show is a command like /usr/ucb/more
show:	libcurses.a show.c
	cc -O show.c -o show libcurses.a -ltermlib


install: ${ARCHIVE}
	cp ${ARCHIVE} ${DESTDIR}/usr/lib/${ARCHIVE}
	chown bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chgrp bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/usr/lib/${ARCHIVE}
	ranlib ${DESTDIR}/usr/lib/${ARCHIVE}
	make clean

clean:
	rm -f ${OBJECTS} ${ARCHIVE} curtest show a.out core log

clobber: clean
	sccs clean

addch.o addstr.o clear.o clrtobot.o \
clrtoeol.o delch.o deleteln.o delwin.o \
endwin.o erase.o getch.o getstr.o \
initscr.o insch.o insertln.o move.o \
mvprintw.o mvscanw.o mvwin.o newwin.o \
printw.o refresh.o scanw.o scroll.o \
standout.o touchwin.o :		curses.ext unctrl.h /usr/include/curses.h
cr_put.o cr_tty.o : curses.ext cr_ex.h
box.o curses.o overlay.o overwrite.o : /usr/include/curses.h

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
