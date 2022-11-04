# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT/misc

# source files
SOURCES= misctest misc2test misc3test date+5.c ipc_test.c

# NOTE: ac.in is a data file and cannot be placed under SCCS control.
# list of files to copy to /usr/lib/usat/misc directory
MISCFILES= ac.in ac.sav adb.sav bdiff.sav cal.sav cat.sav \
	cb.in cb.sav chfn.sav comb.sav comm.sav ctags.sav \
	ctagspgm.c cut.list cut.sav diff3.sav \
	echo.sav file.sav file1 file2 file3 file4 finger.sav \
	grep.sav help.sav join.sav make.mk make.sav \
	man.sav more.sav nm.sav \
	od.sav paste.sav pr.sav prep.sav s5make.sav sccshelp.sav \
	sdiff.sav sort.sav strings.sav stty.sav sum.sav tail.sav \
	tr.sav tsort.sav uniq.sav units.sav wc.sav 

OTHERMISC= misctest misc2test misc3test date+5 ipc_test

COPYFILES= ${MISCFILES} ${OTHERMISC}

all:	${SOURCES}

sources: ${SOURCES}
	
${SOURCES}:
	sccs get $@

#------------------------
# copy all the misc files
#------------------------
install: ${COPYFILES}
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat/misc ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat/misc; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat/misc
	chog bin ${DESTDIR}/usr/lib/usat/misc

	cp ${COPYFILES} ${DESTDIR}/usr/lib/usat/misc

	chmod 644 ${DESTDIR}/usr/lib/usat/misc/*
	chmod 755 ${DESTDIR}/usr/lib/usat/misc/misctest \
		  ${DESTDIR}/usr/lib/usat/misc/misc2test \
		  ${DESTDIR}/usr/lib/usat/misc/misc3test \
		  ${DESTDIR}/usr/lib/usat/misc/date+5 \
		  ${DESTDIR}/usr/lib/usat/misc/ipc_test

	-strip ${DESTDIR}/usr/lib/usat/misc/ipc_test

	make clean

date+5:
	true
# never recompile date+5.c unless absolutely necessary
date+5.exe: date+5.c
	cc -O date+5.c -o date+5.exe
	mv date+5.exe date+5

ipc_test: ipc_test.c
	cc -O ipc_test.c -o ipc_test

clean:
# do not remove date+5 executable -- this should NOT be recompiled
	rm -f log ipc_test

clobber: clean
	sccs clean
tags:
	true
sccsinfo:
	sccs info
