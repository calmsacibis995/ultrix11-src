SCCSID="@(#)makefile	2.4	11/14/85"

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

SOURCES = chfn.c chsh.c ctags.c finger.c from.c last.c mkstr.c renice.c \
	reset.c script.c strings.c users.c which.sh whoami.c xstr.c \
	rcp.c rlogin.c rsh.c ruptime.c rwho.c telnet.c w.c

# NORMCMD are commands that are made via "cc -n -O foo.c -o foo"
NORMCMD = chfn chsh ctags finger from last mkstr rcp renice reset \
	rwho ruptime script strings users w whoami xstr

# JOBSCMD are commands that are made via "cc -n -O foo.c -o foo -ljobs"
JOBSCMD = rlogin rsh telnet

SUBDIRS = error Mail l11 m11 sendmail talk tset ftp netstat tftp

OTHERS = which

CMDS = ${NORMCMD} ${JOBSCMD} ${SUBDIRS} ${OTHERS}
HERE=/usr/src/ucb

all: ${CMDS}

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
	@(echo "Cleaning ucb")
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
	@(echo "Clobbering ucb")
	rm -f *.o core log ${NORMCMD} ${JOBSCMD}
	sccs clean
	@(for i in ${SUBDIRS};  \
	do \
		(cd $$i; \
		echo; \
		echo "Clobbering $$i"; \
		make clobber); \
	done)

sccsinfo tags:	forceit
	@(for i in ${SUBDIRS}; \
	do \
		(cd $$i; \
		echo; \
		echo "Making $$i $@"; \
		make $@); \
	done)

forceit:
	@true

which:	which.sh
	cp which.sh which
	chmod +x which

chsh:	chsh.c
chfn:	chfn.c
ctags:	ctags.c
finger:	finger.c
from:	from.c
last:	last.c
mkstr:	mkstr.c
rcp:	rcp.c
reset:	reset.c
renice:	renice.c
rlogin:	rlogin.c
ruptime: ruptime.c
script:	script.c
strings:	strings.c
users:	users.c
whoami:	whoami.c
xstr:	xstr.c
