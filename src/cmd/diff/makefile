# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for diff
DIFF=	/bin/diff
DIFFH=	/usr/lib/diffh
PR=	/bin/pr
CFLAGS=	-O -DDIFF='"${DIFF}"' -DDIFFH='"${DIFFH}"' -DPR='"${PR}"'

SRCS=	diff.c diffh.c diffdir.c diffreg.c
HDRS=	diff.h
OBJS=	diff.o diffdir.o diffreg.o

all: diff40 diff70 diffh

sources: ${SRCS} ${HDRS}

${SRCS} ${HDRS}:
	sccs get $@

${OBJS}: ${SRCS} ${HDRS}

diff70: ${OBJS}
	cc -i ${CFLAGS} -o diff70 ${OBJS}

diff40: ${OBJS}
	cc ${CFLAGS} -o diff40 ${OBJS}

diffh: diffh.o
	cc -n -o diffh diffh.o

install: all
	rm -f ${DESTDIR}/bin/sid/diff ${DESTDIR}/bin/nsid/diff
	cp diff40 ${DESTDIR}/bin/nsid/diff
	cp diff70 ${DESTDIR}/bin/sid/diff
	cp diffh ${DESTDIR}/usr/lib/diffh
	-strip ${DESTDIR}/usr/lib/diffh \
		${DESTDIR}/bin/sid/diff ${DESTDIR}/bin/nsid/diff
	chog bin ${DESTDIR}/usr/lib/diffh \
		${DESTDIR}/bin/sid/diff ${DESTDIR}/bin/nsid/diff
	chmod 755 ${DESTDIR}/usr/lib/diffh \
		${DESTDIR}/bin/sid/diff ${DESTDIR}/bin/nsid/diff
	-(if [ -f ${DESTDIR}/bin/diff ] ;\
	then \
		mv ${DESTDIR}/bin/diff ${DESTDIR}/bin/diff.junk ;\
	else \
		true ;\
	fi)
	ln ${DESTDIR}/bin/sid/diff ${DESTDIR}/bin/diff
	make clean

cmp: diff diffh
	cmp diff ${DESTDIR}/bin/diff
	cmp diffh ${DESTDIR}/usr/lib/diffh

clean:
	rm -f *.o diff40 diff70 diffh

clobber: clean
	sccs clean

tags:	${SRCS} ${HDRS}
	ctags ${SRCS} ${HDRS}

sccsinfo:
	sccs info
forceit:
