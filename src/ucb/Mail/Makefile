# SCCSID: @(#)Makefile	3.1	10/15/87
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# Unix version 7.
#
# Based on Sccs Id = "@(#)Makefile.11	1.3 10/27/82";
#
# If you do not have a vfork system call or do not support
# job control, you must change the definition of OPTIONS,
#	SIGRETROSRC, and SIGRETROOBJ.
#

OBJS =	aux.o cmd1.o cmd2.o cmd3.o cmdtab.o collect.o config.o edit.o fio.o \
	getname.o head.o v7.local.o lock.o lex.o list.o main.o cksize.o \
	names.o optim.o popen.o quit.o send.o strings.o temp.o tty.o \
	vars.o version.o ${SIGRETROOBJ} str.o

CFILES = aux.c cmd1.c cmd2.c cmd3.c cmdtab.c collect.c config.c edit.c fio.c \
	fmt.c getname.c head.c v7.local.c lock.c lex.c list.c main.c cksize.c \
	names.c optim.c popen.c quit.c send.c strings.c temp.c tty.c vars.c \
	version.c ${SIGRETROSRC}

HFILES = rcv.h configdefs.h def.h glob.h v7.local.h local.h

SOURCES = ${CFILES} ${HFILES}

# These are the files that are dependent on rcv.h
RCVDEP = aux.o cmd1.o cmd2.o cmd3.o collect.o edit.o fio.o getname.o \
	head.o lex.o list.o lock.o cksize.o main.o names.o optim.o quit.o \
	send.o strings.o temp.o tty.o v7.local.o vars.o

BIN=${DESTDIR}/usr/ucb
LIB=${DESTDIR}/usr/lib

OVLDFLAGS=	-n
OPTIONS=	-DV7
LDFLAGS=	-i
CFLAGS=		-O ${OPTIONS}
LIBES=		-ljobs
# for those with job control
SIGRETROSRC=
SIGRETROOBJ=
# for those without job control
#SIGRETROSRC=	sigretro.c
#SIGRETROOBJ=	sigretro.o

XSTR=		/usr/ucb/xstr
RM=		-rm

#
# Special massaging of C files for sharing of strings
#
.c.o:
	${CC} -E ${CFLAGS} $*.c | ${XSTR} -c -
	${CC} -c ${CFLAGS} x.c 
	mv x.o $*.o
	@${RM} -f x.c

all:	Mail70 Mail40 fmt

# Can't run xstr on cksize.c (contains nlist structure)
cksize.o:	cksize.c
	${CC} -c ${CFLAGS} cksize.c

install: Mail70 Mail40 fmt misc/Mail.help misc/Mail.help.~ misc/Mail.rc
	cp Mail70 Mail40 fmt ${BIN}
	-strip ${BIN}/Mail70 ${BIN}/Mail40 ${BIN}/fmt
	cp ${BIN}/Mail70 ${BIN}/Mail
	chmod 755 ${BIN}/Mail ${BIN}/Mail40 ${BIN}/Mail70 ${BIN}/fmt
	chog bin ${BIN}/Mail ${BIN}/Mail40 ${BIN}/Mail70 ${BIN}/fmt
	rm -f ${BIN}/mail
	ln ${BIN}/Mail ${BIN}/mail
	cp misc/Mail.help misc/Mail.help.~ misc/Mail.rc ${LIB}
	chmod 644 ${LIB}/Mail.help ${LIB}/Mail.help.~ ${LIB}/Mail.rc
	chog bin ${LIB}/Mail.help ${LIB}/Mail.help.~ ${LIB}/Mail.rc
	make clean

misc/Mail.help:
	cd misc; sccs get Mail.help

misc/Mail.help.~:
	cd misc; sccs get Mail.help.~

misc/Mail.rc:
	cd misc; sccs get Mail.rc

cmp:	Mail fmt
	cmp Mail ${BIN}/Mail
	cmp fmt ${BIN}/fmt
	rm -f *.o

Mail: Mail70
	cp Mail70 Mail

Mail70:	${OBJS}
	${CC} ${LDFLAGS} -o Mail70 ${OBJS} ${LIBES}
	@size Mail70

Mail40:	${OBJS}
	${CC} ${OVLDFLAGS} -o Mail40 \
	-Z	getname.o names.o optim.o popen.o send.o strings.o temp.o \
		v7.local.o vars.o \
	-Z	aux.o edit.o head.o lex.o list.o lock.o cksize.o \
		main.o quit.o tty.o \
	-Z	cmd1.o cmd2.o cmd3.o collect.o ${SIGRETROOBJ} \
	-L	cmdtab.o config.o fio.o \
		str.o version.o -lv7 ${LIBES}
	@size Mail40

# Header file dependencies
${RCVDEP}: rcv.h def.h local.h glob.h v7.local.h
config.o optim.o: configdefs.h
cmdtab.o: def.h local.h v7.local.h

${SOURCES}:
	sccs get $@;

sources: ${SOURCES}

clean:
	${RM} -f *.o
	rm -f a.out fmt x.c xs.c core Mail Mail70 Mail40
	cp /dev/null strings

clobber: clean
	(cd misc; sccs clean)
	sccs clean

lint:
	lint ${CFLAGS} ${CFILES}

fmt:	fmt.c head.c
	${CC} fmt.c head.c -O -o fmt
	${RM} -f fmt.o head.o
	size fmt

str.o: strings
	${XSTR}
	${CC} -c ${CFLAGS} xs.c
	mv xs.o str.o
	${RM} -f xs.c

config.o:	config.c
	${CC} -c ${CFLAGS} config.c

cmdtab.o: cmdtab.c
	${CC} -c ${CFLAGS} cmdtab.c

wc:
	@wc rcv.h def.h glob.h local.h v7.local.h ${CFILES}
sc:
	@grep -c \; rcv.h def.h glob.h local.h v7.local.h ${CFILES}

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
