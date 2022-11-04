# SCCSID: @(#)makefile	3.0	(ULTRIX-11)	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

SOURCES = arp.c ifconfig.c inetd.c miscd.c rawfs.c rdate.c rexecd.c \
	rlogind.c route.c rshd.c telnetd.c tftpd.c tzname.c uucpsetup

# NORMCMD are commands that are made via "cc -n -O foo.c -o foo"
NORMCMD = arp ifconfig netsetup rawfs route tzname

# JOBSCMD are commands that are made via "cc -n -O foo.c -o foo -ljobs"
JOBSCMD = inetd miscd rexecd rlogind rshd rwhod tftpd rdate telnetd

SUBDIRS = ftpd routed

CMDS = ${NORMCMD} ${JOBSCMD} ${OTHERS} ${SUBDIRS} uucpsetup

all: ${CMDS} dgated

sources: ${SOURCES} subsrc

${SOURCES}:
	sccs get $@

subsrc:
	@(for i in ${SUBDIRS}; \
	do \
		(cd $$i; \
		echo; \
		echo "Making $$i sources"; \
		make sources); \
	done)

dgated: dgated.c
	-@(if [ `whoami` = "jsd" ] ; \
	then \
	echo "rcp dgated.c abyss.jsd:/tmp" ;\
	rcp dgated.c abyss.jsd:/tmp ;\
	sleep 1 ;\
	echo "rsh abyss -l jsd cc -s -O -o /tmp/dgated /tmp/dgated.c" ;\
	rsh abyss -l jsd cc -s -O -o /tmp/dgated /tmp/dgated.c ;\
	sleep 1 ;\
	echo "rcp abyss.jsd:/tmp/dgated . " ;\
	rcp abyss.jsd:/tmp/dgated . ;\
	sleep 1 ;\
	echo "rsh abyss -l jsd rm /tmp/dgated /tmp/dgated.c" ;\
	rsh abyss -l jsd rm /tmp/dgated /tmp/dgated.c ;\
	else \
		echo \
		echo "ERROR: You must be jsd to make VAX executable dgated";\
		echo "(continuing...)";\
	fi)

install:
	(DESTDIR=${DESTDIR}; export DESTDIR; install ${CMDS})

${SUBDIRS}: forceit
	cd $@; make $@

${NORMCMD}:
	cc -n -O $@.c -o $@

${JOBSCMD}:
	cc -n -O $@.c -o $@ -ljobs

clean:
	@(echo)
	@(echo "Cleaning etc")
	rm -f *.o core log ${NORMCMD} ${JOBSCMD}
	@(for i in ${SUBDIRS}; \
	do \
		(cd $$i; \
		echo; \
		echo "Cleaning $$i"; \
		make clean); \
	done)

clobber:
	@(echo)
	@(echo "Clobbering etc")
	rm -f *.o core log ${NORMCMD} ${JOBSCMD}
	sccs clean
	@(for i in ${SUBDIRS};  \
	do \
		(cd $$i; \
		echo; \
		echo "Clobbering $$i"; \
		make clobber); \
	done)

sccsinfo:	forceit
	sccs info
	@(for i in ${SUBDIRS}; \
	do \
		(cd $$i; \
		echo; \
		echo "Making $$i sccsinfo"; \
		make sccsinfo); \
	done)

tags:	forceit
	@(for i in ${SUBDIRS}; \
	do \
		(cd $$i; \
		echo; \
		echo "Making $$i tags"; \
		make tags); \
	done)
	
forceit:
	@true

arp: arp.c
ifconfig: ifconfig.c
inetd: inetd.c
miscd: miscd.c
netsetup: netsetup.c
rdate: rdate.c
rexecd: rexecd.c
rlogind: rlogind.c
rshd: rshd.c
rwhod: rwhod.c
telnetd: telnetd.c
tftpd: tftpd.c
tzname: tzname.c
uucpsetup:	uucpsetup
