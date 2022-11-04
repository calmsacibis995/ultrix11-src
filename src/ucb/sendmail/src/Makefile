# SCCSID: @(#)Makefile	3.0	(ULTRIX-11)	5/2/86

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

#
#  SENDMAIL Makefile.
#
#	Based on:
#		@(#)Makefile.m4	4.1		7/25/83
#

LIBS=	-ldbm -ljobs
OVLIBS= -ljobs
LIBC=	/lib/libc.a

OBJS1=	conf.o main.o collect.o parseaddr.o alias.o deliver.o \
	savemail.o err.o readcf.o stab.o headers.o recipient.o \
	stats.o daemon.o usersmtp.o srvrsmtp.o queue.o \
	macro.o util.o clock.o trace.o envelope.o
OBJS2=	sysexits.o bmove.11.o arpadate.o convtime.o
OVOBJS1=	conf.z main.z collect.z parseaddr.z alias.z deliver.z \
	savemail.z err.z readcf.z stab.z headers.z recipient.z \
	stats.z daemon.z usersmtp.z srvrsmtp.z queue.z \
	macro.z util.z clock.z trace.z envelope.z perror.z syslog.z \
	sysexits.z arpadate.z convtime.z
OVOBJS2=	bmove.11.o
XTRCT=	opendir.o closedir.o gethbyname.o gethbyaddr.o gethostent.o \
	getsbyname.o getpwnam.o getpwuid.o getpwent.o inet_addr.o \
	readdir.o abort.o fgetc.o fputc.o fgets.o fputs.o rew.o \
	fopen.o fdopen.o scanf.o doscan.o fprintf.o printf.o sprintf.o \
	getenv.o getlogin.o ttyslot.o ttyname.o filbuf.o strout.o \
	flsbuf.o findiop.o doprnt.o malloc.o ungetc.o timezone.o \
	getservent.o ctime.o
OBJS=	$(OBJS1) $(OBJS2)
SRCS1=	conf.h sendmail.h \
	conf.c deliver.c main.c parseaddr.c err.c alias.c savemail.c \
	sysexits.c util.c bmove.11.s arpadate.c version.c collect.c \
	macro.c headers.c readcf.c stab.c recipient.c stats.c daemon.c \
	usersmtp.c srvrsmtp.c queue.c clock.c trace.c envelope.c
SRCS2=	TODO convtime.c perror.c syslog.c sendmail.hf
SRCS=	Version.c $(SRCS1) $(SRCS2)

CHMOD=	chmod
O=	-O
COPTS=
CCONFIG=-I../include
CFLAGS=	$O $(COPTS) $(CCONFIG)
LINT=	lint
XREF=	ctags -x

GET=	sccs get
DELTA=	sccs delta
REL=

OBJMODE=755

.SUFFIXES:
.SUFFIXES: .o .z .c .s
.c.z:
	cc -E -DSMALL ${CFLAGS} $*.c | xstr -c -
	cc -S -N -DSMALL ${CFLAGS} x.c
	as -V - -o $*.z x.s
	rm -f $*.s x.s x.c

all: sendmail40 sendmail70

sources: $(SRCS)

$(SRCS):
	sccs get $@

sendmail70: $(OBJS1) $(OBJS2) Version.o
	$(CC) -i $(COPTS) -o sendmail70 Version.o $(OBJS1) $(OBJS2) $(LIBS)
	$(CHMOD) $(OBJMODE) sendmail70
	@echo

sendmail40: $(OVOBJS1) $(OVOBJS2) Version.o xs.o
	cp /usr/lib/libdbm.a dbm.o
	ar x ${LIBC} $(XTRCT)
	/usr/local/calcov -S 1 -a -n -o sendmail40 -B xs.o Version.o $(OVOBJS2) \
		-Z $(OVOBJS1) dbm.o $(XTRCT) -L $(OVLIBS)
	rm -f $(XTRCT) dbm.o
	$(CHMOD) $(OBJMODE) sendmail40
	@echo

xs.c: strings
	xstr

xs.o:
	cc -c -O xs.c

Install: all sendmail.hf
	@echo "You should install SENDMAIL by hand when it isn't running."
	@echo

install: all sendmail.hf
# first, kill the current sendmail daemon
	rm -f ps1.out ps2.out pid.out
	-@(ps ax | grep "/usr/lib/sendmail -bd -q30m" > ps1.out;\
		grep -v "grep" ps1.out > ps2.out ;\
	  cut -f2 -d\   ps2.out > pid.out ;\
	  if [ -f pid.out ] ;\
	  then \
		echo "Killing sendmail PID #`cat pid.out`";\
		kill -9 `cat pid.out` ;\
	  else \
		echo "Cannot find sendmail PID - continuing on..." ;\
	fi)
	rm -f ps1.out ps2.out pid.out
# then, remove the frozen config file
	cat /dev/null > ${DESTDIR}/usr/lib/sendmail.st \
		${DESTDIR}/usr/lib/sendmail.fc
# install the new version
	cp sendmail40 sendmail70 $(DESTDIR)/usr/lib
	-strip $(DESTDIR)/usr/lib/sendmail40 $(DESTDIR)/usr/lib/sendmail70
	mv ${DESTDIR}/usr/lib/sendmail ${DESTDIR}/usr/lib/sendmail.junk
	cp $(DESTDIR)/usr/lib/sendmail70 $(DESTDIR)/usr/lib/sendmail
	chmod 4755 $(DESTDIR)/usr/lib/sendmail
	chog root $(DESTDIR)/usr/lib/sendmail
	rm -f $(DESTDIR)/usr/ucb/mailq $(DESTDIR)/usr/ucb/newaliases
	ln $(DESTDIR)/usr/lib/sendmail $(DESTDIR)/usr/ucb/mailq
	ln $(DESTDIR)/usr/lib/sendmail $(DESTDIR)/usr/ucb/newaliases
	cp sendmail.hf $(DESTDIR)/usr/lib/sendmail.hf
	chog bin $(DESTDIR)/usr/lib/sendmail.hf
	chmod 644 $(DESTDIR)/usr/lib/sendmail.hf
	@echo
	@echo "Re-doing the frozen config file..."
	/usr/lib/sendmail -bz
	@echo
	@echo "Restarting sendmail..."
	/usr/bin/zaptty /usr/lib/sendmail -bd -q30m
#	make clean

version: newversion $(OBJS) Version.c

newversion:
	@rm -f SCCS/p.version.c version.c
	@$(GET) $(REL) -e SCCS/s.version.c
	@$(DELTA) -s SCCS/s.version.c
	@$(GET) -t -s SCCS/s.version.c

fullversion: $(OBJS) dumpVersion Version.o

dumpVersion:
	rm -f Version.c


$(OBJS1): sendmail.h
$(OBJS): conf.h

sendmail.h util.o: ../include/useful.h

#
#  Auxiliary support entries
#

clean:
	rm -f core sendmail sendmail40 sendmail70
	rm -f rmail usersmtp uucp a.out XREF sendmail.cf
	rm -f strings xs.c log *.[oz]

clobber: clean
	sccs clean

tags:	${SRCS}
	ctags ${SRCS}

sccsinfo:
	sccs info

sccsget:
	sccs get SCCS

print: $(SRCS)
	@ls -l | pr -h "sendmail directory"
	@$(XREF) *.c | pr -h "cross reference listing"
	@size *.o | pr -h "object code sizes"
	@pr Makefile *.m4 *.h *.[cs]

lint:
	$(LINT) $(CCONFIG) $(SRCS1)
