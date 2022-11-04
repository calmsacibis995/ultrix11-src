# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
LDFLAGS= -n
#
OWNER=uucp
GROUP=daemon
#
# This version uses 4.2bsd directory reading routines.
# Set the Make variable below to reference the "new directory" routines.
#
#
########################################################################
#
# Common object files assembled into a library (uulib.a)
#
# Targets defined are:
#	all	(default) Make all UUCP commands
#	install	Install all executables
#	save	save old executables and install new version
#	restore	reinstall old executables
#	new	Install executables and make all necessary directories
#	cp	Make and install executables
#	cpdirs
#	cpfiles
#	cpXXXX	Install XXXX program, where cpXXXX can be:
#			cpuucp cpuux cpuuxqt cpuucico cpuulog
#			cpuuclean cpuuname cpuustat cpuusub cpuupoll
#			cpuumonitor cpuumkspool cpuucomp cpuurespool
#	clean	Remove all .o files
#	clobber	Remove .o files and executables, and do 'sccs clean'.
#	cmp:	all
#	XXXX	Make XXXX program, where XXXX can be:
#			uucp uux uuxqt uucico uulog uuclean uuname uustat
#			uusub uupoll uucomp uurespool uumonitor uumkspool
#	mkdirs	Create necessary directories
#	tar	Make exportable 'tar' archive of all
#			necessary sources
########################################################################
CC=cc
#-DNEWLDISC
# CONDEVS contains devices used by uucico 
CONDEVS=-DDF0 -DHAYES -DHAYSTONE -DVENTEL -DHAYESQ -DVADIC -DDF1
CFLAGS=-O -DVMUNIX -DV7M11  -DUUSTAT $(CONDEVS)
# The minimum subdirectory set. The order of the D. files is crucial!
SUBDIRS=C. C./OTHERS STST.  D.`uuname -l`X D.`uuname -l` D. X. TM.

#
# Files to export with tar
HEADRS=	*.h pk.p
METOO=	[mM]akefile *INSTALL* *CHANGE* *README* *TODO*
TFILES= $(METOO) $(HEADRS) $(SOURCES)

FRC=
PKON=pkon.o
CLEAN=clean
LINTOP=-hbau
COMMANDS=uucp uux uuxqt uucico70 uucico40 uulog uuclean uuname uustat \
	uusub uupoll uumonitor uumkspool uurespool uucompact
# things that are made in makefile.misc
MISCMAKE = save restore cmp new cpdirs cpuucp cpuux cpuuxqt \
	cpuucico cpuulog cpuuclean cpuuname cpuustat cpuusub cpuumon \
	cpuumkspool cpuucomp cpuurespool cpuupoll mkdirs cpfiles

COMMLIB=uulib.a
# Common object files
COMMON =anlwrk.o anyread.o assert.o cfgets.o chkpth.o  \
	cpmv.o expfile.o gename.o getargs.o getopt.o getprm.o \
	getpw.o getpwinfo.o gio.o gnamef.o gnxseq.o gwd.o imsg.o \
	index.o lastpart.o logent.o mailst.o pk0.o pk1.o prefix.o \
	sdmail.o setline.o shio.o subdir.o sysacct.o systat.o ub_sst.o \
	ulockf.o us_crs.o us_open.o us_rrs.o us_sst.o uucpname.o \
	versys.o xqt.o uucpdefs.o
# Source files
CFILES =anlwrk.c anyread.c assert.c cfgets.c chkpth.c \
	cpmv.c expfile.c gename.c getargs.c getopt.c getprm.c \
	getpw.c getpwinfo.c gio.c gnamef.c gnxseq.c gwd.c imsg.c \
	index.c lastpart.c logent.c mailst.c pk0.c pk1.c pk.p prefix.c \
	sdmail.c setline.c shio.c subdir.c sysacct.c systat.c ub_sst.c \
	ulockf.c us_crs.c us_open.c us_rrs.c us_sst.c uucpname.c \
	versys.c xqt.c uucpdefs.c uusub.c \
	cico.c cntrl.c conn.c dialout.c condevs.c gnsys.c uucp.c \
	uux.c uuxqt.c uulog.c uuclean.c uuname.c uustat.c uupoll.c \
	uumon.c pkon.c mkspool.c uurespool.c uucompact.c condefs.c
HFILES = pk.h uucp.h uucpdefs.h uust.h uusub.h
SOURCES = ${HFILES} ${CFILES}
#
# Should steal "make depend" from kernel makefile
#
# Object files which depend on "pk.h"
#
PKDEP=	gio.o pk0.o pk1.o
#
# Object files which depend on "uucp.h"
#
UUCPDEP = anlwrk.o anyread.o assert.o chkpth.o cico.o cntrl.o \
	condevs.o conn.o cpmv.o dialout.o expfile.o gename.o \
	getpwinfo.o gio.o gnamef.o gnsys.o gnxseq.o gwd.o \
	imsg.o logent.o mailst.o mkspool.o sdmail.o setline.o \
	shio.o subdir.o systat.o ub_sst.o ulockf.o us_crs.o \
	us_open.o us_rrs.o us_sst.o uuclean.o uucompact.o \
	uucp.o uucpdefs.o uucpname.o uulog.o uumon.o uuname.o \
	uupoll.o uurespool.o uustat.o uux.o uuxqt.o versys.o xqt.o
#
# Object files which depend on "uust.h"
#
UUSTDEP=anlwrk.o cico.o cntrl.o us_crs.o us_open.o us_rrs.o us_sst.o \
	uucp.o uustat.o
#
# Object files which depend on "uusub.h"
UUSUBDEP=cico.o ub_sst.o uusub.o

#
LFILES=assert.c cpmv.c expfile.c gename.c getpwinfo.c index.c lastpart.c \
	prefix.c shio.c ulockf.c xqt.c
OUUCP=uucp.o $(COMMLIB)
LUUCP=uucpdefs.c uucp.c gwd.c chkpth.c getargs.c logent.c uucpname.c\
	versys.c us_crs.c us_open.c
OUUX=uux.o $(COMMLIB)
LUUX=uucpdefs.c uux.c gwd.c anyread.c chkpth.c getargs.c getprm.c\
	logent.c uucpname.c versys.c
OUUXQT=uuxqt.o gnsys.o $(COMMLIB)
LUUXQT=uucpdefs.c uuxqt.c mailst.c getprm.c gnamef.c logent.c uucpname.c \
	chkpth.c getargs.c anyread.c gnsys.c
OUUCICO=$(COMMON) cico.o cntrl.o conn.o condevs.o condefs.o dialout.o gnsys.o
O70UUCICO=cico.o cntrl.o conn.o condevs.o condefs.o dialout.o gnsys.o $(COMMLIB)
LUUCICO=uucpdefs.c cico.c cntrl.c conn.c dialout.c pk0.c pk1.c gio.c anyread.c \
	condevs.c condefs.o \
	anlwrk.c chkpth.c getargs.c gnamef.c gnsys.c gnxseq.c \
	imsg.c logent.c sysacct.c systat.c \
	mailst.c uucpname.c us_rrs.c us_sst.c us_open.c ub_sst.c setline.c
OUULOG=uulog.o $(COMMLIB)
LUULOG=uucpdefs.c uulog.c prefix.c xqt.c ulockf.c gnamef.c assert.c
OUUCLEAN=uuclean.o $(COMMLIB)
OUUCOMPACT=uucompact.o $(COMMLIB)
LUUCLEAN=uucpdefs.c uuclean.c gnamef.c prefix.c mailst.c getpwinfo.c\
	 getargs.c
OUUNAME=uuname.o $(COMMLIB)
LUUNAME=uuname.c uucpname.c uucpdefs.c
OUUSTAT=uustat.o $(COMMLIB)
LUUSTAT=uucpdefs.c uustat.c gnamef.c getpwinfo.c us_open.c getopt.c \
	lastpart.c cpmv.c
OUUSUB=uusub.o $(COMMLIB)
LUUSUB=uucpdefs.c uusub.c getpwinfo.c us_open.c xqt.c getopt.c
OUUPOLL=uupoll.o $(COMMLIB)
LUUPOLL=uucpdefs.c uupoll.c systat.c xqt.c
OUUMON=uumon.o
LUUMON=uumon.c
OUUCOMP=uucompact.o $(COMMLIB)
LUUCOMP=uucompact.c uucpname.c
OUUMKSPOOL=mkspool.o $(COMMLIB)
LUUMKSPOOL=mkspool.c 
OUURESPOOL=uurespool.o $(COMMLIB)
LUUMKSPOOL=uurespool.c 

all:	$(COMMANDS) 

sources: ${SOURCES}

${SOURCES} makefile.misc:
	sccs get $@

$(COMMANDS):	$(COMMLIB)

$(COMMLIB):	$(COMMON)
	rm -f $@
	ar cr $@ $(COMMON) 
	ranlib $@

install:	all cp ${CLEAN}

cp:	makefile.misc all 
	make -f makefile.misc cpall DESTDIR=${DESTDIR}

save:		all

uucp:	$(OUUCP) 
	$(CC) $(LDFLAGS) $(OUUCP) $(LIBNDIR) -o uucp

uux:	$(OUUX) 
	$(CC) $(LDFLAGS) $(OUUX) $(LIBNDIR) -o uux

uuxqt:	$(OUUXQT) 
	$(CC) $(LDFLAGS) $(OUUXQT) $(LIBNDIR) -o uuxqt

# uucico used to also compile in these modules:
#	getopt.o getprm.o getpw.o gwd.o sdmail.o
#	ub_sst.o us_crs.o us_open.o us_sst.o versys.o
# However, the uucico70 version doesn't pull them in from
# the library, and uucico compiles and runs with out them.
# I guess someone was just sloppy.
#		-Dave Borman, 7/10/85

uucico40:	$(OUUCICO) $(IOCTO) $(PKON) $(LIBNDIR)
	$(CC) -n -o uucico40 uucpdefs.o dialout.o condefs.o \
		-Z pk0.o pk1.o pkon.o gio.o imsg.o condevs.o \
		-Z cntrl.o cico.o anlwrk.o lastpart.o \
		   conn.o systat.o anyread.o logent.o \
		-Z xqt.o sysacct.o chkpth.o expfile.o cpmv.o \
		   gnsys.o gnamef.o gename.o setline.o gnxseq.o shio.o \
		   us_rrs.o uucpname.o  assert.o mailst.o \
		   getpwinfo.o prefix.o getargs.o \
		   index.o ulockf.o cfgets.o subdir.o \
		-L

uucico70: $(O70UUCICO) $(IOCTO) $(PKON) $(LIBNDIR)
	$(CC) -i -o uucico70 $(O70UUCICO) $(IOCTO) $(PKON) $(LIBNDIR)

uulog:	$(OUULOG)
	$(CC) $(LDFLAGS) $(OUULOG) $(LIBNDIR) -o uulog

uuclean:  $(OUUCLEAN)
	$(CC) $(LDFLAGS) $(OUUCLEAN) $(LIBNDIR) -o uuclean

uucompact:  $(OUUCOMPACT)
	$(CC) $(LDFLAGS) $(OUUCOMPACT) $(LIBNDIR) -o uucompact

uuname:	$(OUUNAME)
	$(CC) $(LDFLAGS) $(OUUNAME) $(LIBNDIR) -o uuname
 
uustat:	$(OUUSTAT)
	$(CC) $(LDFLAGS) $(OUUSTAT) $(LIBNDIR) -o uustat

uusub:	$(OUUSUB)
	$(CC) $(LDFLAGS) $(OUUSUB) $(LIBNDIR) -o uusub

uumonitor:	$(OUUMON)
	$(CC)  $(OUUMON)  $(LIBNDIR) -o uumonitor

uucomp:	$(OUUCOMP)
	$(CC)  $(OUUCOMP)  $(LIBNDIR) -o uucompact

uurespool:	$(OUURESPOOL)
	$(CC)  $(OUURESPOOL)  $(LIBNDIR) -o uurespool

uumkspool: $(OUUMKSPOOL)
	$(CC)  $(OUUMKSPOOL)  $(LIBNDIR) -o uumkspool

uupoll:	$(OUUPOLL)
	$(CC) $(LDFLAGS) $(OUUPOLL) $(LIBNDIR) -o uupoll

${MISCMAKE}: makefile.misc
	make -f makefile.misc $@

#
# Header file dependencies
#
$(PKDEP):	pk.h pk.p
$(UUCPDEP):	uucp.h
$(UUSTDEP):	uust.h
$(UUSUBDEP):	uusub.h

FRC:


# Make exportable 'tar' archive package
#
tar:
	tar cbf 1 uucp.tar $(TFILES)

clean:
	-rm -f *.o
	-rm -f $(COMMLIB)
	-rm -f $(COMMANDS) uumonitor

clobber:	clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
