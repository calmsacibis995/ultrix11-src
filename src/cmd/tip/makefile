# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# make file for intermachine communications package
#
# Files are:
#	/etc/remote		remote host description file
#	/etc/phones		phone number file, owned by ${OWNER} and
#				  mode 6??
#	${ADM}/aculog		ACU accounting file, owned by ${OWNER} and
#				  mode 6?? {if ACULOG defined}
# Presently supports:
#	BIZCOMP
#	DEC DF02-AC, DF03-AC, DF112 DF224
#	DEC DN-11/Able Quadracall
#	VENTEL 212+
#	VADIC 831 RS232 adaptor
#	VADIC 3451
# (drivers are located in aculib.a)
#
# Configuration defines:
#	DF02, DF03, DF112, DF224, DN11	ACU's supported
#	  BIZ1031, BIZ1022, VENTEL, V831, V3451
#	ACULOG			turn on tip logging of ACU use
#	PRISTINE		no phone #'s put in ACU log file
#	CONNECT			worthless command
#	DEFBR			default baud rate to make connection at
#	DEFFS			default frame size for FTP buffering of
#				writes on local side
#	BUFSIZ			buffer sizing from stdio, must be fed
#				explicitly to remcap.c if not 1024
#	V7M-11			mods to run on V7M-11

ADM=	${DESTDIR}/usr/adm
OWNER=	uucp
CONFIG=	-DBIZ1031 -DBIZ1022 -DDF02 -DDF03 -DDF112 -DDF224 -DDN11 -DVENTEL -DV3451 -DV831
CFLAGS=	-O -DDEFBR=300 -DDEFFS=BUFSIZ -DACULOG -DV7M-11
LDFLAGS= -n
OBJS=	acu.o cmds.o cmdtab.o cu.o hunt.o \
	log.o partab.o remote.o tip.o tipout.o value.o vars.o
SPECIAL=acutab.c remcap.c uucplock.c
SOURCES=acu.c acutab.c cmds.c cmdtab.c cu.c hunt.c \
	log.c partab.c remote.c tip.c tipout.c value.c vars.c \
	${SPECIAL}

all: tip uuunlock

tip:	${OBJS} acutab.o remcap.o uucplock.o aculib/aculib.a
	${CC} -o tip ${LDFLAGS} ${OBJS} acutab.o remcap.o uucplock.o \
		aculib/aculib.a 

uuunlock: uuunlock.c
	${CC} -o uuunlock -n -O uuunlock.c

{SOURCES}:
	sccs get $<

sources: ${SOURCES}

${OBJS}: tip.h
acu.o: acu.c
cmds.o: cmds.c
cmdtab.o: cmdtab.c
cu.o: cu.c
hunt.o: hunt.c
log.o: log.c
partab.o: partab.c
remote.o: remote.c
tip.o: tip.c
tipout.o: tipout.c
value.o: value.c
vars.o: vars.c
uucplock.o: uucplock.c

remcap.o: remcap.c
	${CC} -c -DBUFSIZ=1024 ${CFLAGS} remcap.c

# acutab is configuration dependent, and so depends on the makefile
acutab.o: makefile
acutab.o: acutab.c
	${CC} -c ${CFLAGS} ${CONFIG} acutab.c

# remote.o depends on the makefile because of DEFBR and DEFFS
remote.o: makefile

# log.o depends on the makefile because of ACULOG
log.o:	makefile

# forceit is something that doesn't exist, used to force makeing of aculib.a
aculib/aculib.a: forceit
	cd aculib; make ${MFLAGS}

forceit:

install: tip uuunlock
	cp tip ${DESTDIR}/usr/bin/tip
	-strip ${DESTDIR}/usr/bin/tip
	chog ${OWNER} ${DESTDIR}/usr/bin/tip
	chmod 4711 ${DESTDIR}/usr/bin/tip
	-rm -f ${DESTDIR}/usr/bin/cu
	ln ${DESTDIR}/usr/bin/tip ${DESTDIR}/usr/bin/cu
	cp uuunlock ${DESTDIR}/usr/lib/uucp/uuunlock
	-strip ${DESTDIR}/usr/lib/uucp/uuunlock
	chog ${OWNER} ${DESTDIR}/usr/lib/uucp/uuunlock
	chmod 4711 ${DESTDIR}/usr/lib/uucp/uuunlock
	cat /dev/null >> ${ADM}/aculog
	chown ${OWNER} ${ADM}/aculog
	chmod 600 ${ADM}/aculog
	@echo "create /etc/remote and /etc/phones"
	make clean

clean:
	cd aculib; make ${MFLAGS} clean
	rm -f *.o tip cu

clobber:
	cd aculib; make ${MFLAGS} clobber
	rm -f *.o tip cu
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
