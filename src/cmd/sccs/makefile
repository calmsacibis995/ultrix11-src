# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
ALL=lib cmds sccs help.d
HDR= hdr/defines.h hdr/had.h
CMDS= admin bdiff comb delta get help prs prt rmchg unget val vc what
SOURCES= ${HDR}

CCMDS= cmds/admin cmds/bdiff cmds/comb cmds/delta cmds/get cmds/help \
	cmds/prs cmds/prt cmds/rmchg cmds/unget cmds/val cmds/vc cmds/what

all: ${ALL}

sources: ${SOURCES} forceit
	(cd cmds; make sources)
	(cd lib; make sources)
	(cd help.d; make sources)
	(cd sccs; make sources)

${SOURCES}:
	(cd hdr; sccs get $@)

install: all
	(cd cmds; make install DESTDIR=${DESTDIR})
	(cd help.d; make install DESTDIR=${DESTDIR})
	(cd sccs; make install DESTDIR=${DESTDIR})
	make clean

cmds: forceit ${HDR}
	(cd cmds; make)

help.d: forceit
	(cd help.d; make)

lib: forceit ${HDR}
	(cd lib; make)

sccs: forceit
	(cd sccs; make)

${CMDS}: cmds/$@
	(cd cmds; make $@)

forceit:

clean:
	(cd cmds; make clean)
	(cd lib; make clean)
	(cd sccs; make clean)
	rm -f log

clobber:
	(cd cmds; make clobber)
	(cd help.d; make clobber)
	(cd lib; make clobber)
	(cd sccs; make clobber)
	sccs clean

tags:	forceit
	(cd cmds; make tags)
	(cd help.d; make tags)
	(cd lib; make tags)
	(cd sccs; make tags)

sccsinfo: forceit
	sccs info
	(cd cmds; make sccsinfo)
	(cd help.d; make sccsinfo)
	(cd lib; make sccsinfo)
	(cd sccs; make sccsinfo)
