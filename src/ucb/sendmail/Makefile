# SCCSID: @(#)Makefile	3.0	(ULTRIX-11)	4/23/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	Makefile for sendmail base directory
#
# Based on:
#		@(#)Makefile	4.2		9/12/83
#
SUBS=lib src aux cf

all:	sendmail

sendmail:
	cd lib; make
	cd src; make
	cd aux; make
	cd cf; make

install:  all
	cd lib; make DESTDIR=${DESTDIR} install
	cd src; make DESTDIR=${DESTDIR} install
	cd aux; make DESTDIR=${DESTDIR} install
	cd cf; make
	make clean

clean clobber sources tags sccsinfo:
	@(for i in ${SUBS} ;\
	do \
	    (echo; echo cd $$i; cd $$i; \
	    echo make $@; make $@) ;\
	done)
	@(case $@ in \
	       clean) ;; \
	     clobber) echo sccs clean; sccs clean ;; \
	     sources) ;; \
		tags) ;; \
	    sccsinfo) echo; sccs info ;; \
	esac)
