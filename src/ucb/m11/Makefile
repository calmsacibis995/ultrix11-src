SCCSID	=	"@(#)Makefile	3.0	4/22/86"

######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################

.SUFFIXES:	.m11 .obj

LFLAGS	=	-n
LINK	=	/usr/ucb/l11
M11	=	./m11.x
MACXRF	=	/usr/ucb/macxrf		# this pathname is wired into exec.m11
MFLAGS	=

.m11.obj:
		${M11}  ${MFLAGS} $<

OBJS=		exec.obj macro.obj code.obj expr.obj fltg.obj getl.obj \
		lout.obj mac.obj srch.obj xlat.obj misc.obj pst.obj \
		errs.obj debug.obj often.obj xpcor.obj syscalls.obj
PROGS=		exec.m11 macro.m11 code.m11 expr.m11 fltg.m11 getl.m11 \
		lout.m11  mac.m11 srch.m11 xlat.m11 misc.m11 pst.m11 \
		errs.m11 debug.m11 often.m11 xpcor.m11 syscalls.m11 at.sml

all:		m11 macxrf

sources:

install:	all
		cp m11	${DESTDIR}/usr/ucb/m11
		-strip ${DESTDIR}/usr/ucb/m11
		cp macxrf ${DESTDIR}${MACXRF}
		-strip ${DESTDIR}${MACXRF}
		chmod 755 ${DESTDIR}${MACXRF} ${DESTDIR}/usr/ucb/m11
		chog bin ${DESTDIR}${MACXRF} ${DESTDIR}/usr/ucb/m11
		make clean

cmp:		all
		cmp m11	${DESTDIR}/usr/ucb/m11
		cmp macxrf ${DESTDIR}${MACXRF}

m11:		${OBJS}
		${LINK} -ls ${LFLAGS} ${OBJS}
		mv exec.out m11

macxrf:		macxrf.c
		cc ${LFLAGS} -O -o macxrf macxrf.c

clean:
		rm -f *.obj m11 maxcrf

clobber: clean

sccsinfo:
	sccs info

tags:
	true
