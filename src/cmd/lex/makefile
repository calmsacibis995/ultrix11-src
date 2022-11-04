# SCCSID: @(#)makefile	3.0	4/24/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS = -O

SOURCES= header.c ldefs.c lmain.c once.c parser.y sub1.c sub2.c ncform
SUBSRCS= allprint.c main.c reject.c yyless.c yywrap.c

all: lex70 lex40 libln.a

sources: ${SOURCES} ${SUBSRCS}

${SOURCES}:
	sccs get $@

${SUBSRCS}:
	cp ./lib/$@ .

install:	all
	cp lex70 ${DESTDIR}/usr/bin/lex70
	cp lex70 ${DESTDIR}/usr/bin/lex
	-strip ${DESTDIR}/usr/bin/lex ${DESTDIR}/usr/bin/lex70
	chmod 755 ${DESTDIR}/usr/bin/lex ${DESTDIR}/usr/bin/lex70
	chog bin ${DESTDIR}/usr/bin/lex ${DESTDIR}/usr/bin/lex70
	cp lex40 ${DESTDIR}/usr/bin/lex40
	-strip ${DESTDIR}/usr/bin/lex40
	chmod 755 ${DESTDIR}/usr/bin/lex40
	chog bin ${DESTDIR}/usr/bin/lex40
	rm -f ${DESTDIR}/lib/libln.a ${DESTDIR}/lib/libl.a
	cp libln.a ${DESTDIR}/lib/libln.a
	chmod 644 ${DESTDIR}/lib/libln.a
	chog bin ${DESTDIR}/lib/libln.a
	ln ${DESTDIR}/lib/libln.a ${DESTDIR}/lib/libl.a
	-(if [ ! -d ${DESTDIR}/usr/lib/lex ] ; \
	then \
		mkdir ${DESTDIR}/usr/lib/lex; \
		chog bin ${DESTDIR}/usr/lib/lex; \
		chmod 777 ${DESTDIR}/usr/lib/lex; \
	else \
		true; \
	fi)
	-(if [ ! -s ${DESTDIR}/usr/lib/lex/ncform ] ; \
	then \
		cp ncform ${DESTDIR}/usr/lib/lex/ncform; \
	else \
		true; \
	fi)
	chog bin ${DESTDIR}/usr/lib/lex/ncform
	chmod 664 ${DESTDIR}/usr/lib/lex/ncform
	make clean

lex70: lmain.o y.tab.o sub1.o sub2.o header.o ncform
	cc -i lmain.o y.tab.o sub1.o sub2.o header.o -o lex70

lex40: lmain.o y.tab.o sub1.o sub2.o header.o ncform
	cc -DSMALL -n -O lmain.c y.tab.c sub1.c sub2.c header.c -o lex40

y.tab.c: parser.y
	yacc parser.y

lmain.o:lmain.c ldefs.c once.c
	cc -c -O lmain.c

sub1.o: sub1.c ldefs.c
	cc -c -O sub1.c

sub2.o: sub2.c ldefs.c
	cc -c -O sub2.c

header.o: header.c ldefs.c
	cc -c -O header.c

libln.a: allprint.c main.c reject.c yyless.c yywrap.c
	cc -c -O allprint.c main.c reject.c yyless.c
	cc -c -O yywrap.c
	rm -f libln.a
	ar rvc libln.a allprint.o main.o reject.o yyless.o yywrap.o

clean:
	rm -f allprint.o main.o reject.o yyless.o yywrap.o
	rm -f allprint.c main.c reject.c yyless.c yywrap.c
	rm -f *.o y.tab.c
	rm -f lex70 lex40 libln.a

clobber: clean
	sccs clean

tags:	${SOURCES} ${SUBSRCS}
	ctags ${SOURCES} ${SUBSRCS}

sccsinfo:
	sccs info
