# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#
# Makefile for ULTRIX-11  C Shell with process control
# 	-Dave Borman 5/8/84
#
CFLAGS=	-O -Dpdp -DULIMIT
SOURCES= doprnt.s rofix sh.init.c \
	sh.c sh.dol.c sh.err.c sh.exec.c sh.exp.c sh.func.c sh.glob.c \
	sh.hist.c sh.lex.c sh.misc.c sh.parse.c sh.print.c sh.sem.c \
	sh.set.c sh.proc.c sh.dir.c sh.time.c \
	alloc.c printf.c getpwent.c getpwnam.c getpwuid.c onintr.c

HEADERS= sh.h sh.local.h sh.dir.h sh.proc.h

OBJ70=	sh.o sh.dol.o sh.err.o sh.exec.o sh.exp.o sh.func.o sh.glob.o \
	sh.hist.o sh.lex.o sh.misc.o sh.parse.o sh.print.o sh.sem.o sh.set.o \
	sh.proc.o sh.dir.o sh.time.o alloc.o sh.init.o printf.o \
	doprnt.o onintr.o

# strings.o must be last in OBJ40 since it
# can change when previous files compile
OBJ40=	sh.ov.o sh.dol.ov.o sh.err.ov.o sh.exec.ov.o sh.exp.ov.o \
	sh.func.ov.o sh.glob.ov.o sh.hist.ov.o sh.lex.ov.o \
	sh.misc.ov.o sh.parse.ov.o sh.print.ov.o sh.sem.ov.o \
	sh.set.ov.o sh.proc.ov.o sh.dir.ov.o sh.time.ov.o \
	alloc.ov.o printf.ov.o \
	getpwent.ov.o getpwnam.ov.o getpwuid.ov.o onintr.ov.o

all:	csh70 csh40

sources: ${SOURCES} ${HEADERS}

${SOURCES} ${HEADERS}:
	sccs get $@

csh:	all

csh70: ${OBJ70} sh.local.h
	rm -f csh70
	cc ${OBJ70} -i -o csh70 -ljobs

print:
	@pr READ_ME
	@pr makefile makefile.*
	@(size -l a.out; size *.o) | pr -h SIZES
	@ls -l | pr 
	@pr sh*.h sh*.c alloc.c

install: csh70 csh40
	rm -f ${DESTDIR}/bin/sid/csh ${DESTDIR}/bin/nsid/csh
	cp csh70 ${DESTDIR}/bin/sid/csh
	cp csh40 ${DESTDIR}/bin/nsid/csh
	-strip ${DESTDIR}/bin/sid/csh ${DESTDIR}/bin/nsid/csh
	chmod 755 ${DESTDIR}/bin/sid/csh ${DESTDIR}/bin/nsid/csh
	chog bin ${DESTDIR}/bin/sid/csh ${DESTDIR}/bin/nsid/csh
	-(if [ -f ${DESTDIR}/bin/csh ] ;\
	then \
		mv ${DESTDIR}/bin/csh ${DESTDIR}/bin/csh.junk ;\
	else \
		true ;\
	fi)
	ln ${DESTDIR}/bin/sid/csh ${DESTDIR}/bin/csh
	make clean

clean:
	-rm -f a.out strings log core x.c xs.c csh40 csh70
	-rm -f *.o

clobber: clean
	sccs clean

csh40: ${OBJ40} doprnt.o sh.init.ov.o strings.o sh.local.h
	ld -n -X -o csh40 /lib/crt0.o \
	-Z sh.dol.ov.o sh.exec.ov.o sh.exp.ov.o sh.func.ov.o \
	   sh.glob.ov.o sh.print.ov.o sh.time.ov.o \
	-Z alloc.ov.o sh.err.ov.o sh.hist.ov.o sh.lex.ov.o sh.misc.ov.o \
	   sh.parse.ov.o sh.sem.ov.o sh.set.ov.o \
	-Z sh.proc.ov.o sh.dir.ov.o getpwent.ov.o \
	-L sh.ov.o sh.init.ov.o printf.ov.o doprnt.o \
	   getpwnam.ov.o getpwuid.ov.o onintr.ov.o strings.o -ljobs -lc
	size csh40

# Special massaging of C files for sharing of strings
${OBJ40}:
	cc -E ${CFLAGS} `basename $@ .ov.o`.c | xstr -c -
	cc -c ${CFLAGS} x.c 
	mv x.o $@

# strings.o and sh.init.ov.o are specially processed to be shared
strings.o: strings rofix
	xstr
	cc -S xs.c
	ed - <rofix xs.s
	as -V - -o strings.o xs.s
	rm xs.s

sh.init.ov.o: sh.init.c sh.local.h rofix
	cc -E ${CFLAGS} sh.init.c | xstr -c -
	cc ${CFLAGS} -S x.c
	ed - <rofix x.s
	as -V - -o sh.init.ov.o x.s
	rm x.s

# doprnt.s is the old ovdoprnt.s, here it just gets overlaid.

doprnt.o:	doprnt.s
sh.o sh.ov.o:	sh.h sh.local.h sh.c
sh.dol.o sh.dol.ov.o:	sh.h sh.local.h sh.dol.c
sh.err.o sh.err.ov.o:	sh.h sh.local.h sh.err.c
sh.exec.o sh.exec.ov.o:	sh.h sh.local.h sh.exec.c
sh.exp.o sh.exp.ov.o:	sh.h sh.local.h sh.exp.c
sh.func.o sh.func.ov.o:	sh.h sh.local.h sh.func.c
sh.glob.o sh.glob.ov.o:	sh.h sh.local.h sh.glob.c
sh.hist.o sh.hist.ov.o:	sh.h sh.local.h sh.hist.c
sh.lex.o sh.lex.ov.o:	sh.h sh.local.h sh.lex.c
sh.misc.o sh.misc.ov.o:	sh.h sh.local.h sh.misc.c
sh.parse.o sh.parse.ov.o:	sh.h sh.local.h sh.parse.c
sh.print.o sh.print.ov.o:	sh.h sh.local.h sh.print.c
sh.sem.o sh.sem.ov.o:	sh.h sh.local.h sh.proc.h sh.sem.c
sh.set.o sh.set.ov.o:	sh.h sh.local.h sh.set.c
sh.proc.o sh.proc.ov.o:	sh.h sh.local.h sh.dir.h sh.proc.h sh.proc.c
sh.dir.o sh.dir.ov.o:	sh.h sh.local.h sh.dir.h sh.dir.c
sh.time.o sh.time.ov.o:	sh.h sh.local.h sh.time.c
alloc.o alloc.ov.o:	sh.local.h alloc.c
sh.init.o: sh.local.h
printf.o printf.ov.o:	printf.c
getpwent.ov.o:	getpwent.c
getpwnam.ov.o:	getpwnam.c
getpwuid.ov.o:	getpwuid.c

tags:	${SOURCES} ${HEADERS}
	ctags ${SOURCES} ${HEADERS}

sccsinfo:
	sccs info
