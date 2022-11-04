# SCCSID: @(#)makefile	3.0	4/27/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	This is the makefile to create both awk40 and awk70.
#	the .z files are objects needed for awk40 that are
#	different from their equivilant .o files for awk70.
#	awk.dat2.o is created from awk.g.y, it's data that
#	can be shared, but needs to be in the base segement.
#	Since awk.g.z goes into an overlay, we have to extract
#	the stuff first.  We have our own copy of crt0. This
#	is so that we can enter after the first word, which is
#	0.  We need to keep the first word NULL for the strange
#	people who index off of NULL pointers. (that's a no-no,
#	but why fix something if it works, right???)
#	Thanks to Jerry Brenner who got awk40 running in the first
#	place.
#	8/28/84 -Dave Borman
#
SOURCES =	awk.def awk.g.y awk.lx.l b.c lib.c main.c parse.c \
		proc.c freeze.c run.c tran.c crt0.s
EDSCRIPTS =	_awk.lx.dif _awk.g.fix _dat-to-txt tokenscript
FILES =		awk.lx.o b.o main.o token.o tran.o \
		lib.o run.o parse.o proctab.o freeze.o
40FILES =	awk.dat2.o awk.lx.z b.z main.o token.o tran.z \
		lib.z run.z parse.z proctab.o freeze.o crt0.o
CFLAGS = -O
YFLAGS=-d
CC_N = cc -N

.SUFFIXES:
.SUFFIXES: .o .z .c .y .l .s

.c.z:
	mkstr - awk_strings x $*.c
	${CC_N} ${CFLAGS} -c x$*.c
	rm -f x$*.c
	mv x$*.o $*.z

all: awk70 awk40

awk70:	awk.def ${FILES} awk.g.o
	cc -i -o awk70 ${CFLAGS} awk.g.o  ${FILES}  -lm

awk40:	awk.def ${40FILES} awk.g.z
	ld -X -e __start -n -o awk40 crt0.o proctab.o awk.dat2.o \
	-Z run.z \
	-Z awk.g.z \
	-Z main.o freeze.o token.o lib.z tran.z parse.z \
	-Z b.z awk.lx.z \
	-L -lm -lc
	size awk40

sources: ${SOURCES} token.c.awksav ${EDSCRIPTS}

${SOURCES} ${EDSCRIPTS}:
	sccs get $@

${FILES}: awk.h awk.def

token.c: awk.h tokenscript token.c.awksav
	cp token.c.awksav token.c
	@chmod 644 token.c
	ed - <tokenscript
	@chmod 444 token.c

install: awk70 awk40
	cp awk70 ${DESTDIR}/usr/bin/awk
	cp awk70 ${DESTDIR}/usr/bin/awk70
	-strip ${DESTDIR}/usr/bin/awk70 ${DESTDIR}/usr/bin/awk
	chog bin ${DESTDIR}/usr/bin/awk70 ${DESTDIR}/usr/bin/awk
	chmod 755 ${DESTDIR}/usr/bin/awk70 ${DESTDIR}/usr/bin/awk
	cp awk40 ${DESTDIR}/usr/bin/awk40
	-strip ${DESTDIR}/usr/bin/awk40
	chog bin ${DESTDIR}/usr/bin/awk40
	chmod 755 ${DESTDIR}/usr/bin/awk40
	cp awk_strings ${DESTDIR}/usr/lib/awk_strings
	chog bin ${DESTDIR}/usr/lib/awk_strings
	chmod 644 ${DESTDIR}/usr/lib/awk_strings
	make clean

proc:	awk.h proc.o token.o
	cc -o proc proc.o token.o

proctab.c:	proc
	proc >proctab.c

awk.g.o awk.g.z awk.h: awk.g.y awk.def _awk.g.fix
	yacc -d awk.g.y
	-cmp -s y.tab.h awk.h || cp y.tab.h awk.h
	${CC} ${CFLAGS} -c y.tab.c
	mv y.tab.o awk.g.o
	${CC_N} ${CFLAGS} -S y.tab.c
	ed - y.tab.s < _awk.g.fix
	as - -V -o awk.g.z y.tab.s
	rm y.tab.s y.tab.c y.tab.h

awk.lx.o awk.lx.z awk.dat2.o: awk.h awk.lx.l _awk.lx.dif _dat-to-txt
	lex awk.lx.l
	${CC} ${CFLAGS} -c lex.yy.c
	mv lex.yy.o awk.lx.o
	ed - lex.yy.c < _awk.lx.dif
	${CC_N} ${CFLAGS} -c lex.yy.c
	mv lex.yy.o awk.lx.z
	rm lex.yy.c
	${CC} -S awk.dat2.c
	ed - awk.dat2.s < _dat-to-txt
	as - -o awk.dat2.o awk.dat2.s
	rm -f awk.dat2.s awk.dat2.c lex.yy.c

lib.z:
	mkstr - awk_strings x lib.c
	${CC_N} -DERRFILE -DSMALL -c xlib.c
	rm -f xlib.c
	mv xlib.o lib.z

clean:
	-rm -f a.out *.o t.* *temp* *.out *junk* y.tab.* awk.h proc proctab.c
	rm -f x*.c *.z lex.yy.c \
	Compare dif.* \
	y.tab.* awk.lxz.c temp awk.dat1.c awk.dat2.c
	rm -f awk70 awk40 awk_strings log

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
forceit:
