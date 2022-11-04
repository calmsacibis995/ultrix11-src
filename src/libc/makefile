# SCCSID: @(#)makefile	3.0	(ULTRIX-11)	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# This makefile creates the C library, libc.a
#
AS = as -V
LIBNAME = libc.a
# The order of directories should not be changed!
SUBS= net dir stdio gen sys crt csu sysV

all: ${LIBNAME} csu
	:

csu: forceit
	(cd csu; make)

install: all
	cp ${LIBNAME} ${DESTDIR}/lib
	ranlib ${DESTDIR}/lib/${LIBNAME}
	chog bin ${DESTDIR}/lib/${LIBNAME}
	chmod 644 ${DESTDIR}/lib/${LIBNAME}
	(cd csu; make "DESTDIR=${DESTDIR}" install)
	make clean

${LIBNAME}: forceit
	rm -f ${LIBNAME}
	(cd net; make "CC=${CC}" "AS=${AS}" LIB=${LIBNAME} archive)
	(cd dir; make "CC=${CC}" "AS=${AS}" LIB=${LIBNAME} archive)
	(cd stdio; make "CC=${CC}" "AS=${AS}" LIB=${LIBNAME} archive)
	(cd gen; make "CC=${CC}" "AS=${AS}" LIB=${LIBNAME} archive)
	(cd sys; make "CC=${CC}" "AS=${AS}" LIB=${LIBNAME} archive)
	(cd crt; make "CC=${CC}" "AS=${AS}" LIB=${LIBNAME} archive)
	(cd sysV; make "CC=${CC}" "AS=${AS}" LIB=${LIBNAME} archive)
	ranlib ${LIBNAME}

clean clobber sources tags sccsinfo: forceit
	@(for i in ${SUBS} ;\
	do \
	    (echo; echo cd $$i; cd $$i; \
	    echo make $@; make $@) ;\
	done)
	@(case $@ in \
	  clean) echo rm -f *.o a.out core log $LIBNAME} ; \
		 rm -f *.o a.out core log $LIBNAME} ;;\
	clobber) echo rm -f *.o a.out core log ; \
		 rm -f *.o a.out core log ;\
		 echo sccs clean ; sccs clean ;;\
	esac)

forceit:
