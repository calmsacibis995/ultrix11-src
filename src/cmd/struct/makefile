# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS=-O
YFLAGS=-d
0FILES.c = 0.alloc.c 0.args.c 0.def.c 0.extr.c\
	   0.graph.c 0.list.c 0.parts.c 0.string.c
0FILES.o = 0.alloc.o 0.args.o 0.def.o 0.extr.o\
	   0.graph.o 0.list.o 0.parts.o 0.string.o

1FILES.c = 1.finish.c 1.form.c 1.fort.c 1.hash.c 1.init.c\
	   1.line.c 1.main.c 1.node.c 1.recog.c 1.tables.c
1FILES.o = 1.finish.o 1.form.o 1.fort.o 1.hash.o 1.init.o\
	   1.line.o 1.main.o 1.node.o 1.recog.o 1.tables.o

2FILES.c = 2.dfs.c 2.dom.c 2.head.c 2.inarc.c 2.main.c 2.tree.c
2FILES.o = 2.dfs.o 2.dom.o 2.head.o 2.inarc.o 2.main.o 2.tree.o

3FILES.c = 3.branch.c 3.flow.c 3.loop.c 3.main.c 3.reach.c 3.then.c
3FILES.o = 3.branch.o 3.flow.o 3.loop.o 3.main.o 3.reach.o 3.then.o

4FILES.c = 4.brace.c 4.form.c 4.main.c 4.out.c
4FILES.o = 4.brace.o 4.form.o 4.main.o 4.out.o

BFILES= beauty.c tree.c lextab.l bdef.c
BFILE=  beauty.o tree.o lextab.o bdef.o

SOURCES= struct.sh main.c b.h  beauty.y 1.incl.h\
	def.h 1.defs.h 2.def.h 3.def.h 4.def.h\
	${0FILES.c} ${1FILES.c} ${2FILES.c}\
	${3FILES.c} ${4FILES.c} tree.c lextab.l bdef.c

all:	structure40 structure70 struct.sh beautify

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	cp structure40 structure70 beautify ${DESTDIR}/usr/lib/struct
	-strip ${DESTDIR}/usr/lib/struct/structure70
	chmod 755 ${DESTDIR}/usr/lib/struct/structure70
	chog bin ${DESTDIR}/usr/lib/struct/structure70
	-strip ${DESTDIR}/usr/lib/struct/structure40
	chmod 755 ${DESTDIR}/usr/lib/struct/structure40
	chog bin ${DESTDIR}/usr/lib/struct/structure40
	-strip ${DESTDIR}/usr/lib/struct/beautify
	chmod 755 ${DESTDIR}/usr/lib/struct/beautify
	chog bin ${DESTDIR}/usr/lib/struct/beautify
	cp structure70 ${DESTDIR}/usr/lib/struct/structure
	-strip ${DESTDIR}/usr/lib/struct/structure
	chmod 755 ${DESTDIR}/usr/lib/struct/structure
	chog bin ${DESTDIR}/usr/lib/struct/structure
	cp struct.sh ${DESTDIR}/usr/bin/struct
	chmod 755 ${DESTDIR}/usr/bin/struct
	chog bin ${DESTDIR}/usr/bin/struct
	make clean

cmp:	beautify structure
	cmp beautify ${DESTDIR}/usr/lib/struct/beautify
	cmp structure ${DESTDIR}/usr/lib/struct/structure

structure70: main.o $(0FILES.o) $(1FILES.o) $(2FILES.o) $(3FILES.o) $(4FILES.o)
	cc -o structure70 -i main.o 0.*.o 1.*.o 2.*.o 3.*.o 4.*.o 

structure40: main.o $(0FILES.o) $(1FILES.o) $(2FILES.o) $(3FILES.o) $(4FILES.o)
	cc -o structure40 main.o 0.*.o 1.*.o 2.*.o 3.*.o 4.*.o 

beautify: $(BFILE)
	cc -n -O $(BFILE) -o beautify -lln

main.o $(0FILES.o) $(1FILES.o) $(2FILES.o) $(3FILES.o) $(4FILES.o): def.h
main.o: main.c 1.defs.h
lextab.o tree.o: y.tab.h
$(BFILE): b.h $(BFILES)
$(0FILES.o): $(0FILES.c) def.h 1.defs.h
$(1FILES.o): $(1FILES.c) 1.defs.h 1.incl.h
$(2FILES.o): $(2FILES.c) 2.def.h
$(3FILES.o): $(3FILES.c) 3.def.h
$(4FILES.o): $(4FILES.c) 4.def.h

y.tab.h:	beauty.o

beauty.o: beauty.c

beauty.c: beauty.y

clean:
	rm -f *.o y.tab.h beauty.c structure40 structure70 beautify

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
