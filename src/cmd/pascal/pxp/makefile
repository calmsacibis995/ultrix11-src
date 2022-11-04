# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#  On a separate I/D machine without floating point,
#  you will have to use ../utililities/fpterp/fp.o instead of
#  ../utilities/fpnofetchi/fp.o; see the comments in ../utilities/fpterp.

EYACC=	../eyacc/eyacc -v
CFLAGS=	-O -DPXP

SOURCES1 = call.c case.c const.c cset.c \
	error.c fdec.c func.c hash.c lab.c lval.c \
	main.c nl.c pas.y pmon.c pp.c printf.s proc.c rec.c rval.c \
	stat.c string.c subr.c tree.c treen.s type.c var.c \
	yycomm.c yycopy.s yycosts.c yyerror.c yyget.c yyid.c yylex.c yymain.c \
	yypanic.c yyparse.c yyprint.c yyput.c yyrecover.c \
	yyseman.c yytree.c TRdata.c 

SOURCES2 = ../px/getc.s ../px/putc.s \
	../utilities/fpnofetchi/fp1.s ../utilities/fpnofetchi/fp2.s \
	../utilities/fpnofetchi/fp3.s ../utilities/fpnofetchi/fpx.s

SOURCES = ${SOURCES1} ${SOURCES2}

OBJS1 =	call.o case.o const.o cset.o \
	error.o fdec.o func.o hash.o lab.o lval.o \
	main.o nl.o pmon.o pp.o proc.o rec.o rval.o \
	stat.o string.o subr.o tree.o type.o var.o y.tab.o \
	yycomm.o yycosts.o yyerror.o yyget.o yyid.o yylex.o yymain.o \
	yypanic.o yyparse.o yyprint.o yyput.o yyrecover.o \
	yyseman.o yytree.o

OBJS2 =	printf.o treen.o yycopy.o ../utilities/fpnofetchi/fp.o \
	TRdata.o ../px/getc.o ../px/putc.o

OBJS = ${OBJS1} ${OBJS2}

all:	pxp ../eyacc/eyacc

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

pxp:	${OBJS}
	cc -c version.c
	cc ${CFLAGS} -o pxp ${OBJS} version.o

y.tab.c: pas.y
	${EYACC} pas.y
	ed - y.tab.c < gram

../eyacc/eyacc:
	(cd ../eyacc; make)

../utilities/fpterp/fp.o:
	(cd ../utilities/fpterp; make fp.o)

../utilities/fpnofetchi/fp.o:
	(cd ../utilities/fpnofetchi; make fp.o)

../px/getc.o: ../px/getc.s
	(cd ../px; make getc.o)

../px/putc.o: ../px/putc.s
	(cd ../px; make putc.o)

install: all
	cp pxp ${DESTDIR}/usr/bin/pxp
	-strip ${DESTDIR}/usr/bin/pxp
	chmod 755 ${DESTDIR}/usr/bin/pxp
	chog bin ${DESTDIR}/usr/bin/pxp
	cp how_pxp ${DESTDIR}/usr/lib/how_pxp
	chmod 644 ${DESTDIR}/usr/lib/how_pxp
	chog bin ${DESTDIR}/usr/lib/how_pxp
	make clean

clean:
	rm -f pi1.2strings a.out core y.tab.c y.output *.o x* pxp

clobber: clean
	sccs clean

tags:	${SOURCES1}
	ctags ${SOURCES1}

sccsinfo:
	sccs info
