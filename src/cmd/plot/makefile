# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS = -n -O

SOURCES= driver.c chrtab.c vplot.c plot.sh

all:	tek t300 t300s t450 tregis tla50 tla100 vplot plot.sh

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install:	all
	cp tek t300 t300s t450 tregis tla50 tla100 vplot ${DESTDIR}/usr/bin
	-strip ${DESTDIR}/usr/bin/tek
	chmod 755 ${DESTDIR}/usr/bin/tek
	chog bin ${DESTDIR}/usr/bin/tek
	-strip ${DESTDIR}/usr/bin/t300
	chmod 755 ${DESTDIR}/usr/bin/t300
	chog bin ${DESTDIR}/usr/bin/t300
	-strip ${DESTDIR}/usr/bin/t300s
	chmod 755 ${DESTDIR}/usr/bin/t300s
	chog bin ${DESTDIR}/usr/bin/t300s
	-strip ${DESTDIR}/usr/bin/t450
	chmod 755 ${DESTDIR}/usr/bin/t450
	chog bin ${DESTDIR}/usr/bin/t450
	-strip ${DESTDIR}/usr/bin/tregis
	chmod 755 ${DESTDIR}/usr/bin/tregis
	chog bin ${DESTDIR}/usr/bin/tregis
	-strip ${DESTDIR}/usr/bin/tla50
	chmod 755 ${DESTDIR}/usr/bin/tla50
	chog bin ${DESTDIR}/usr/bin/tla50
	-strip ${DESTDIR}/usr/bin/tla100
	chmod 755 ${DESTDIR}/usr/bin/tla100
	chog bin ${DESTDIR}/usr/bin/tla100
	-strip ${DESTDIR}/usr/bin/vplot
	chmod 755 ${DESTDIR}/usr/bin/vplot
	chog bin ${DESTDIR}/usr/bin/vplot
	cp plot.sh ${DESTDIR}/usr/bin/plot
	chmod 755 ${DESTDIR}/usr/bin/plot
	chog bin ${DESTDIR}/usr/bin/plot
	make clean

cmp:	all
	cmp tek ${DESTDIR}/usr/bin/tek
	cmp t300 ${DESTDIR}/usr/bin/t300
	cmp t300s ${DESTDIR}/usr/bin/t300s
	cmp t450 ${DESTDIR}/usr/bin/t450
	cmp tregis ${DESTDIR}/usr/bin/tregis
	cmp tla50 ${DESTDIR}/usr/bin/tla50
	cmp tla100 ${DESTDIR}/usr/bin/tla100
	cmp vplot ${DESTDIR}/usr/bin/vplot
	cmp plot.sh ${DESTDIR}/usr/bin/plot

tek:	driver.c driver.o
	cc -n -o tek  driver.o -lt4014 -lm

t300:	driver.c driver.o 
	cc -n -o t300 driver.o -lt300 -lm

t300s:	driver.c driver.o 
	cc -n -o t300s driver.o -lt300s -lm

t450:	driver.c driver.o 
	cc -n -o t450 driver.o -lt450 -lm

tregis:	driver.c driver.o 
	cc -n -o tregis driver.o -ltregis -lm

tla50:	driver.c driver.o 
	cc -o tla50 driver.o -ltla50 -lm

tla100:	driver.c driver.o 
	cc -o tla100 driver.o -ltla100 -lm

vplot:	chrtab.c vplot.c chrtab.o vplot.o
	cc -o vplot vplot.o chrtab.o

clean:
	rm -f *.o tek t300 t300s t450 tregis tla50 tla100 vplot

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
