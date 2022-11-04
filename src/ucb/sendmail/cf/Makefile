# SCCSID: @(#)Makefile	3.0	4/23/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
######################################################################
#
#	Makefile for system generation handling of
#	sendmail configuration files. Liberally adapted
#	from more standard sendmail Makefiles
#
#	Based on:	@(#)Makefile	1.12	(ULTRIX-32)	3/22/85
#
######################################################################

EXFILES= KEY Makefile.example amber.mc arpa.mc arpaproto.mc \
	berkhosts.m4 berkm.m4 brahms.mc \
	cad.mc calder.mc ccbase.m4 cchosts.m4 coral.mc cory.mc \
	csbase.m4 csether.mc dali.mc dbvax.mc ernie.mc ether.m4 etherm.m4 \
	fncpm.m4 fudge.m4 ibmm.m4 ic.mc icw.mc ingres.mc ivory.mc jade.mc \
	kim.mc lapis.mc matisse.mc medea.mc miro.mc mmdfm.m4 monet.mc \
	ncphosts.m4 ncpm.m4 oz.mc ruby.mc statvax.mc suucpm.m4 tcphosts.m4 \
	topaz.mc turner.mc ucbvax.mc uucpproto.mc

EXMAKE= Makefile.dist

GENSRCS= exampleether.mc exampleuucp.mc \
	base.m4 compat.m4 localm.m4 tcpm.m4 uucpm.m4 version.m4 xm.m4 \
	zerobase.m4

SRCS= ${EXFILES} ${EXMAKE} ${GENSRCS}

EXAMPLE= exampleuucp.cf exampleether.cf

COPY= ${EXAMPLE} ${EXFILES} ${GENSRCS}

GET=	sccs get

.SUFFIXES: .mc .cf

.mc.cf:
	m4 $*.mc > $*.cf

all: $(EXAMPLE)

############################
#   special dependencies   #
############################

# pessimize -- basic dependencies
$(EXAMPLE): base.m4 localm.m4 tcpm.m4 version.m4 zerobase.m4

# hosts using UUCP
exampleuucp.cf: uucpm.m4 compat.m4

#####################
#   administrivia   #
#####################

$(SRCS):
#	$(GET) $(REL) SCCS/s.$@

install: ${SRCS} ${EXAMPLE}
	-if [ ! -d ${DESTDIR}/usr/src ]; then\
		mkdir ${DESTDIR}/usr/src; \
		/bin/chown root ${DESTDIR}/usr/src; \
		chgrp sys ${DESTDIR}/usr/src; \
	else true; \
	fi
	-if [ ! -d ${DESTDIR}/usr/spool/mqueue ]; then\
		mkdir ${DESTDIR}/usr/spool/mqueue ; \
	else true; \
	fi
	-/bin/chown daemon ${DESTDIR}/usr/spool/mqueue
	-chgrp sys ${DESTDIR}/usr/spool/mqueue
	-chmod 755 ${DESTDIR}/usr/spool/mqueue 
	-if [ ! -d ${DESTDIR}/usr/spool/mail ]; then\
		mkdir ${DESTDIR}/usr/spool/mail ; \
	else true; \
	fi
	-/bin/chown root ${DESTDIR}/usr/spool/mail
	-chgrp sys ${DESTDIR}/usr/spool/mail
	-chmod 755 ${DESTDIR}/usr/spool/mail 

clean:
	rm -f $(EXAMPLE) a.out core make.out log

clobber: clean
	sccs clean

sccsinfo:
	sccs info

tags:
	true
