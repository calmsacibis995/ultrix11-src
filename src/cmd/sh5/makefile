# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for System V shell (/bin/sh5)
# Based on:	@(#) sh.mk  1.8	(System V)

#CFLAGS = -O -DNICE -DACCT -DNICEVAL=4
CFLAGS = -O -DNICE -DNICEVAL=4

SOURCES= setbrk.c blok.c stak.c cmd.c fault.c main.c \
	word.c string.c name.c args.c xec.c service.c \
	error.c io.c print.c macro.c expand.c \
	test.c defs.c echo.c hash.c hashserv.c pwd.c \
	func.c ctype.c msg.c

OBJ70=	setbrk.o blok.o stak.o cmd.o fault.o main.o \
	word.o string.o name.o args.o xec.o service.o \
	error.o io.o print.o macro.o expand.o \
	test.o defs.o echo.o hash.o hashserv.o pwd.o \
	func.o ctype.o msg.o

OBJ40=	setbrk.ov.o blok.ov.o stak.ov.o cmd.ov.o fault.ov.o main.ov.o \
	word.ov.o string.ov.o name.ov.o args.ov.o xec.ov.o service.ov.o \
	error.ov.o io.ov.o print.ov.o macro.ov.o expand.ov.o \
	test.ov.o defs.ov.o echo.ov.o hash.ov.o hashserv.ov.o \
	pwd.ov.o func.ov.o

all:	sh540 sh570

sh570:	${OBJ70} defs.h
	$(CC) -i $(OBJ70) -o sh570
	@echo

sh540:	${OBJ40} defs.h ctype.ov.o msg.ov.o
	ld -n -X -o sh540 /lib/crt0.o \

	-Z func.ov.o hash.ov.o hashserv.ov.o word.ov.o pwd.ov.o test.ov.o \

	-Z service.ov.o xec.ov.o blok.ov.o echo.ov.o \
	setbrk.ov.o stak.ov.o string.ov.o \

	-Z name.ov.o expand.ov.o cmd.ov.o args.ov.o macro.ov.o \

	-Z io.ov.o fault.ov.o error.ov.o \

	-L main.ov.o print.ov.o defs.ov.o msg.ov.o ctype.ov.o -ljobs -lc

	size sh540


#	Base Segment text sizes:
#	------------------------
#	main.ov.o: 1424
#	print.ov.o: 944
#	defs.ov.o: 0
#	msg.ov.o: 1426
#	ctype.ov.o: 256

#	Overlay #1 text sizes:
#	----------------------
#	func.ov.o: 1446
#	hash.ov.o: 508
#	hashserv.ov.o: 1682
#	word.ov.o: 1168
#	pwd.ov.o: 926
#	test.ov.o: 1890

#	Overlay #2 text sizes:
#	----------------------
#	service.ov.o: 2110
#	xec.ov.o: 4326
#	blok.ov.o: 334
#	echo.ov.o: 290
#	setbrk.ov.o: 30
#	stak.ov.o: 266
#	string.ov.o: 160

#	Overlay #3 text sizes:
#	----------------------
#	name.ov.o: 2272
#	expand.ov.o: 1000
#	cmd.ov.o: 2414
#	args.ov.o: 834
#	macro.ov.o: 1610

#	Overlay #4 text sizes:
#	----------------------
#	io.ov.o: 1256
#	fault.ov.o: 610
#	error.ov.o: 318


msg.ov.o: forceit msg.c
	CC=$(CC) AS="$(AS)" /bin/sh ./:fix msg
	mv msg.o msg.ov.o

ctype.ov.o: ctype.h
# next line can save 256 bytes data space, but causes
# base text segment to exceed 8192 bytes, so leave it out.
#	CC=$(CC) AS="$(AS)" /bin/sh ./:fix ctype
	cc -c ${CFLAGS} `basename $@ .ov.o`.c
	mv ctype.o ctype.ov.o

${OBJ40}:
	cc -c ${CFLAGS} `basename $@ .ov.o`.c
	mv `basename $@ .ov.o`.o $@

xec.o:	xec.c
	$(CC) $(CFLAGS) -c xec.c

service.o: service.c
	$(CC) $(CFLAGS) -c service.c

blok.o fault.o main.o stak.o:	brkincr.h
blok.ov.o fault.ov.o main.ov.o stak.ov.o:	brkincr.h

io.o main.o:	dup.h
io.ov.o main.ov.o:	dup.h

xec.o hash.o hashserv.o:	hash.h
xec.ov.o hash.ov.o hashserv.ov.o:	hash.h

cmd.o macro.o main.o msg.o word.o xec.o:	sym.h
cmd.ov.o macro.ov.o main.ov.o msg.ov.o word.ov.o xec.ov.o:	sym.h

main.o:	timeout.h
main.ov.o:	timeout.h

install_manual: all
	@(echo "/bin/sh5 must be installed MANUALLY")
	sync

install: all
	rm -f ${DESTDIR}/bin/sid/sh5 ${DESTDIR}/bin/nsid/sh5
	cp sh570 ${DESTDIR}/bin/sid/sh5
	cp sh540 ${DESTDIR}/bin/nsid/sh5
	-strip ${DESTDIR}/bin/sid/sh5 ${DESTDIR}/bin/nsid/sh5
	chmod 755 ${DESTDIR}/bin/sid/sh5 ${DESTDIR}/bin/nsid/sh5
	chog bin ${DESTDIR}/bin/sid/sh5 ${DESTDIR}/bin/nsid/sh5
	-(if [ -f ${DESTDIR}/bin/sh5 ] ;\
	then \
		mv ${DESTDIR}/bin/sh5 ${DESTDIR}/bin/sh5.junk ;\
	else \
		true ;\
	fi)
	ln ${DESTDIR}/bin/sid/sh5 ${DESTDIR}/bin/sh5
	rm -f ${DESTDIR}/bin/rsh5
	ln ${DESTDIR}/bin/sh5 ${DESTDIR}/bin/rsh5

	make clean

clean:
	  rm -f *.o a.out core log sh540 sh570

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info

forceit:

setbrk.o setbrk.ov.o: setbrk.c
blok.o blok.ov.o: blok.c
stak.o stak.ov.o: stak.c
cmd.o cmd.ov.o: cmd.c
fault.o fault.ov.o: fault.c
main.o main.ov.o: main.c
word.o word.ov.o: word.c
string.o string.ov.o: string.c
name.o name.ov.o: name.c
args.o args.ov.o: args.c
xec.o xec.ov.o: xec.c
service.o service.ov.o: service.c
error.o error.ov.o: error.c
io.o io.ov.o: io.c
print.o print.ov.o: print.c
macro.o macro.ov.o: macro.c
expand.o expand.ov.o: expand.c
test.o test.ov.o: test.c
defs.o defs.ov.o: defs.c
echo.o echo.ov.o: echo.c
hash.o hash.ov.o: hash.c
hashserv.o hashserv.ov.o: hashserv.c
pwd.o pwd.ov.o: pwd.c
func.o func.ov.o: func.c
ctype.o ctype.ov.o: ctype.c
msg.o msg.ov.o: msg.c

