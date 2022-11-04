#	Sccsid  @(#)makefile 3.0 4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	(All Macro Definitions are described on line following definition)

# this directory
HERE=.
# SCCS common object library directory
LIBDIR=../lib/comobj.a

# Directory where executable SCCS commands are stored
INSDIR=${DESTDIR}/usr/bin

# C compiler
CC=cc

# Flags used by C compiler
CFLAGS= -O

# Load flags used by C compiler
LDFLAGS= -Y -n

# Libraries used during loading
LIBES= -lPW

SOURCES=admin.c bdiff.c comb.c delta.c get.c help.c prs.c rmchg.c \
	sccsdiff unget.c val.c vc.c what.c prt.c

OBJECTS=admin.o bdiff.o comb.o delta.o get.o help.o prs.o rmchg.o unget.o \
	val.o vc.o what.o prt.o

BINARYS=admin comb delta get help prs rmchg unget val vc what prt

all:	${BINARYS} bdiff sccsdiff

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

admin:	${LIBDIR} admin.c admin.o 
# link in libv7.a next to shrink base text below 8K
bdiff:	${LIBDIR} bdiff.c bdiff.o
	cc ${LDFLAGS} -o bdiff bdiff.o ${LIBDIR} ${LIBES} -lv7
comb:	${LIBDIR} comb.c comb.o 
delta:	${LIBDIR} delta.c delta.o 
get:	${LIBDIR} get.c get.o 
help:	${LIBDIR} help.c help.o 
prs:	${LIBDIR} prs.c prs.o 
prt:	${LIBDIR} prt.c prt.o 
rmchg:	${LIBDIR} rmchg.c rmchg.o 
unget:	${LIBDIR} unget.c unget.o 
val:	${LIBDIR} val.c val.o 
vc:	${LIBDIR} vc.c vc.o 
what:	${LIBDIR} what.c what.o 

${BINARYS}:
	${CC} ${LDFLAGS} -o $@ $@.o ${LIBDIR} ${LIBES}

install:	all
	cp ${HERE}/admin ${HERE}/bdiff ${HERE}/comb ${HERE}/delta ${INSDIR}
	-strip ${INSDIR}/admin ${INSDIR}/bdiff ${INSDIR}/comb ${INSDIR}/delta
	chmod 755 ${INSDIR}/admin ${INSDIR}/bdiff ${INSDIR}/comb ${INSDIR}/delta
	chog bin ${INSDIR}/admin ${INSDIR}/bdiff ${INSDIR}/comb ${INSDIR}/delta

	mv help sccshelp

	cp ${HERE}/get ${HERE}/sccshelp ${HERE}/prs ${HERE}/prt ${INSDIR}
	-strip ${INSDIR}/get ${INSDIR}/sccshelp ${INSDIR}/prs ${INSDIR}/prt
	chmod 755 ${INSDIR}/get ${INSDIR}/sccshelp ${INSDIR}/prs ${INSDIR}/prt
	chog bin ${INSDIR}/get ${INSDIR}/sccshelp ${INSDIR}/prs ${INSDIR}/prt

	cp ${HERE}/rmchg ${HERE}/sccsdiff ${HERE}/unget ${INSDIR}
	-strip ${INSDIR}/rmchg ${INSDIR}/unget
	chmod 755 ${INSDIR}/rmchg ${INSDIR}/sccsdiff ${INSDIR}/unget
	chog bin ${INSDIR}/rmchg ${INSDIR}/sccsdiff ${INSDIR}/unget

	cp ${HERE}/val ${HERE}/vc ${HERE}/what ${INSDIR}
	-strip ${INSDIR}/val ${INSDIR}/vc ${INSDIR}/what
	chmod 755 ${INSDIR}/val ${INSDIR}/vc ${INSDIR}/what
	chog bin ${INSDIR}/val ${INSDIR}/vc ${INSDIR}/what

	rm -f ${INSDIR}/cdc
	ln ${INSDIR}/rmchg ${INSDIR}/cdc
	rm -f ${INSDIR}/rmdel
	ln ${INSDIR}/rmchg ${INSDIR}/rmdel
	rm -f ${INSDIR}/sact
	ln ${INSDIR}/unget ${INSDIR}/sact

	make clean

${LIBDIR}:
	@echo ${LIBDIR} not found. Please make ${LIBDIR} first
	@false

clean:
	rm -f *.o log a.out
	rm -f ${BINARYS} bdiff sccshelp

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
