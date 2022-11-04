# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# 
#  Facility:	 Part of the LA50 plot package 
#
#  Abstract:   make tla50  -- creates standalone plot driver tregis
#		    lib     -- creates library archive /usr/lib/libtla50.a
#		    tar     -- creates la50plt.tar containing all code
#		    print   -- Prints all neccessary files
#                   install -- Install everybody
#                   clean   -- clean out the object and garbage files
#                   clobber -- SCCS clean			
#                   sources -- SCCS get			
#			
#  Author:     Kevin J. Dunlap
# 
#  Creation:     March 1985
# 

SOURCES = arc.c box.c circle.c close.c cont.c dot.c erase.c\
	label.c line.c linemod.c move.c open.c point.c space.c subr.c 

OBJECTS = arc.o box.o circle.o close.o cont.o dot.o erase.o\
	label.o line.o linemod.o move.o open.o point.o space.o subr.o  

HEADER = la50.h
ARCHIVE = libtla50.a
CFLAGS = -O
# DESTDIR = /usr
DESTDIR =

all: ${ARCHIVE}

tla50: ${OBJECTS} driver.o
	cc ${CFLAGS} ${OBJECTS} driver.o -o tla50 -lm

subr.o: la50.h

open.o: subr.o la50.h

space.o: la50.h

${HEADER}:
	sccs get $@

${ARCHIVE}: forceit ${OBJECTS}
	rm -f ${ARCHIVE}
	ar cr ${ARCHIVE} ${OBJECTS}
	ranlib ${ARCHIVE}

sources: ${HEADER} ${SOURCES}

${SOURCES}:
	sccs get $@

install: ${ARCHIVE}
	cp ${ARCHIVE} ${DESTDIR}/usr/lib/${ARCHIVE}
	chown bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chgrp bin ${DESTDIR}/usr/lib/${ARCHIVE}
	chmod 664 ${DESTDIR}/usr/lib/${ARCHIVE}
	ranlib ${DESTDIR}/usr/lib/${ARCHIVE}
	make clean

forceit:

clean:
	rm -f *.o ${ARCHIVE} tla50 a.out core log

clobber: clean
	sccs clean

tar:
	tar cvf la50plt.tar  makefile driver.c ${SOURCES}

print:	print makefile ${SOURCES} 

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
