# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for directory /usr/src

CMDS =	cmd ucb etc 
CLIBS =	libc libcV libcurses libdbm libjobs libm libmp \
	libplot libtermlib libape libfpsim libPW \
	libmalloc libv7 liby
FLIBS =	libF77 libI77 libU77
# MISC = crypt games
SUBDIRS = ${CMDS} ${CLIBS} ${FLIBS} ${MISC}
HERE=/usr/src

all: clibs flibs cmds ${MISC}

dirstruct:
	-(DESTDIR=${DESTDIR}; export DESTDIR; sh mkskel)

cmds:	forceit
	-(for i in ${CMDS} ;\
	do \
	    (echo "cd $$i"; cd $$i; \
	    echo make "CC=${CC}" "AS=${AS}" ;\
	         make "CC=${CC}" "AS=${AS}") ;\
	done)

flibs:	forceit
	-(for i in ${FLIBS} ;\
	do \
	    (echo "cd $$i"; cd $$i ;\
	    echo make "CC=${CC}" "AS=${AS}" ;\
	         make "CC=${CC}" "AS=${AS}") ;\
	done)

clibs:	forceit
	-(for i in ${CLIBS} ;\
	do \
	    (echo "cd $$i"; cd $$i ;\
	    echo make "CC=${CC}" "AS=${AS}" ;\
	         make "CC=${CC}" "AS=${AS}") ;\
	done)

crypt:
	cd crypt; make "CC=${CC}" "AS=${AS}"
games:
	cd games; make "CC=${CC}" "AS=${AS}"

sources tags sccsinfo: forceit
	-(for i in ${SUBDIRS} ;\
	do \
	    (echo "cd $$i"; cd $$i ;\
	    echo "make $@"; make $@) ;\
	done)

install: dirstruct instclibs instflibs instcmds

instcmds: forceit
	-(for i in ${CMDS} ;\
	do \
	    (echo "cd $$i"; cd $$i ;\
	    echo make install "DESTDIR=${DESTDIR}" "CC=${CC}" "AS=${AS}" ;\
	    make install "DESTDIR=${DESTDIR}" "CC=${CC}" "AS=${AS}") ;\
	done)

instflibs: forceit
	-(for i in ${FLIBS} ;\
	do \
	    (echo "cd $$i"; cd $$i ;\
	    echo make install "DESTDIR=${DESTDIR}" "CC=${CC}" "AS=${AS}" ;\
	    make install "DESTDIR=${DESTDIR}" "CC=${CC}" "AS=${AS}") ;\
	done)

instclibs: forceit
	-(for i in ${CLIBS} ;\
	do \
	    (echo "cd $$i"; cd $$i ;\
	    echo make install "DESTDIR=${DESTDIR}" "CC=${CC}" "AS=${AS}" ;\
	    make install "DESTDIR=${DESTDIR}" "CC=${CC}" "AS=${AS}") ;\
	done)

instmisc: forceit
	-(for i in ${MISC} ;\
	do \
	    (echo "cd $$i"; cd $$i ;\
	    echo make install "DESTDIR=${DESTDIR}" "CC=${CC}" "AS=${AS}" ;\
	    make install "DESTDIR=${DESTDIR}" "CC=${CC}" "AS=${AS}") ;\
	done)

clean clobber:	forceit
	rm -f log a.out core
	-(for i in ${SUBDIRS} ;\
	do \
	    (echo; echo "$@ing $$i"; cd $$i; make $@) ;\
	done)

forceit:
