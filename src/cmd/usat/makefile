# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for USAT

FILES= etime.sh etimes help1 help2
SOURCES = ${FILES} usat
SUBS= awk bc cc dc doc f77 learn lex lint misc pascal pcc sed sys yacc

all:	${SOURCES}

${SOURCES}:
	sccs get $@

install: install1
	-(for i in ${SUBS} ;\
	do \
	    (echo; echo "cd $$i"; cd $$i; \
	    echo "make $@"; make $@) ;\
	done)
	make clean

# install all the local stuff, make the usat directory...
install1: all forceit
	cp usat ${DESTDIR}/usr/bin/usat
	chmod 755 ${DESTDIR}/usr/bin/usat
	chog bin ${DESTDIR}/usr/bin/usat

# set-up /usr/lib/usat directory
	-(if [ ! -d  ${DESTDIR}/usr/lib/usat ]; \
	then \
		mkdir ${DESTDIR}/usr/lib/usat; \
	else \
		true; \
	fi)

	chmod 755 ${DESTDIR}/usr/lib/usat
	chog bin ${DESTDIR}/usr/lib/usat

# load help files and time scripts
	cp ${FILES} ${DESTDIR}/usr/lib/usat
	-(for i in ${FILES} ; \
	do \
	    chmod 644 ${DESTDIR}/usr/lib/usat/$$i;\
	    chog bin ${DESTDIR}/usr/lib/usat/$$i;\
	done)
# special case the shell script
	chmod 755 ${DESTDIR}/usr/lib/usat/etime.sh

sources: ${SOURCES} forceit
	-(for i in ${SUBS} ;\
	do \
	    (echo; echo "cd $$i"; cd $$i; \
	    echo "make $@"; make $@) ;\
	done)

clean tags: forceit
	-(for i in ${SUBS} ;\
	do \
	    (echo; echo "cd $$i"; cd $$i; \
	    echo "make $@"; make $@) ;\
	done)

clobber: forceit
	-(for i in ${SUBS} ;\
	do \
	    (echo; echo "cd $$i"; cd $$i; \
	    echo "make $@"; make $@) ;\
	done)
	sccs clean

sccsinfo: forceit
	sccs info
	-(for i in ${SUBS} ;\
	do \
	    (echo; echo "cd $$i"; cd $$i; \
	    echo "make $@"; make $@) ;\
	done)

forceit:
	@true
