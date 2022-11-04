# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# 
#  Facility:	 Part of the REGIS plot package 
#
#  Abstract:   make tregis  -- creates standalone plot driver tregis
#		    lib     -- creates library archive /usr/lib/libtregis.a
#		    tar     -- creates regisplt.tar containing all code
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

SOURCES = arc.c box.c circle.c close.c color.c cont.c dot.c erase.c map.c\
	label.c line.c linemod.c move.c open.c point.c space.c subr.c wrtng.c

OBJECTS = arc.o box.o circle.o close.o color.o cont.o dot.o erase.o map.o\
	label.o line.o linemod.o move.o open.o point.o space.o subr.o wrtng.o 

ARCHIVE = libtregis.a
CFLAGS = -O
# DESTDIR = /usr
DESTDIR =

all: ${ARCHIVE}

tregis: ${OBJECTS} driver.o
	cc ${CFLAGS} ${OBJECTS} driver.o -o tregis -lm

${ARCHIVE}: forceit ${OBJECTS}
	rm -f ${ARCHIVE}
	ar cr ${ARCHIVE} ${OBJECTS}
	ranlib ${ARCHIVE}

sources: ${SOURCES}

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
	rm -f *.o ${ARCHIVE} tregis a.out core log

clobber: clean
	sccs clean

tar:
	tar cvf regisplt.tar  makefile driver.c ${SOURCES}

print:	print makefile ${SOURCES} 

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
