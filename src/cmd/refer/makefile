# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS = -O

SOURCES= deliv2.c\
	glue1.c glue2.c glue3.c glue4.c glue5.c\
	hunt1.c hunt2.c hunt3.c hunt5.c hunt6.c hunt7.c hunt8.c hunt9.c\
	inv1.c inv2.c inv3.c inv5.c inv6.c\
	mkey1.c mkey2.c mkey3.c\
	refer..c refer0.c refer1.c refer2.c refer3.c\
	refer4.c refer5.c refer6.c refer7.c refer8.c\
	shell.c\
	what..c what1.c what2.c what3.c what4.c

all:	mkey inv hunt40 hunt70 refer40 refer70

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install:	all
	cp refer40 refer70 ${DESTDIR}/usr/bin
	-strip ${DESTDIR}/usr/bin/refer40
	chmod 755 ${DESTDIR}/usr/bin/refer40
	chog bin ${DESTDIR}/usr/bin/refer40
	-strip ${DESTDIR}/usr/bin/refer70
	chmod 755 ${DESTDIR}/usr/bin/refer70
	chog bin ${DESTDIR}/usr/bin/refer70
	cp inv hunt40 hunt70 mkey ${DESTDIR}/usr/lib/refer
	-strip ${DESTDIR}/usr/lib/refer/inv
	chmod 755 ${DESTDIR}/usr/lib/refer/inv
	chog bin ${DESTDIR}/usr/lib/refer/inv
	-strip ${DESTDIR}/usr/lib/refer/hunt40
	chmod 755 ${DESTDIR}/usr/lib/refer/hunt40
	chog bin ${DESTDIR}/usr/lib/refer/hunt40
	-strip ${DESTDIR}/usr/lib/refer/hunt70
	chmod 755 ${DESTDIR}/usr/lib/refer/hunt70
	chog bin ${DESTDIR}/usr/lib/refer/hunt70
	-strip ${DESTDIR}/usr/lib/refer/mkey
	chmod 755 ${DESTDIR}/usr/lib/refer/mkey
	chog bin ${DESTDIR}/usr/lib/refer/mkey

	cp refer70 ${DESTDIR}/usr/bin/refer 
	-strip ${DESTDIR}/usr/bin/refer
	chmod 755 ${DESTDIR}/usr/bin/refer
	chog bin ${DESTDIR}/usr/bin/refer
	cp hunt70 ${DESTDIR}/usr/lib/refer/hunt
	-strip ${DESTDIR}/usr/lib/refer/hunt
	chmod 755 ${DESTDIR}/usr/lib/refer/hunt
	chog bin ${DESTDIR}/usr/lib/refer/hunt
	make clean

cmp:	all
	cmp refer40 ${DESTDIR}/usr/bin/refer40
	cmp refer70 ${DESTDIR}/usr/bin/refer70
	cmp inv ${DESTDIR}/usr/lib/refer/inv
	cmp hunt40 ${DESTDIR}/usr/lib/refer/hunt40
	cmp hunt70 ${DESTDIR}/usr/lib/refer/hunt70
	cmp mkey ${DESTDIR}/usr/lib/refer/mkey

mkey: mkey1.o mkey2.o mkey3.o deliv2.o
	cc -n mkey?.o deliv2.o -o mkey

inv: inv1.o inv2.o inv3.o inv5.o inv6.o deliv2.o
	cc -n inv?.o deliv2.o -o inv

hunt70: hunt1.o hunt2.o hunt3.o hunt5.o hunt6.o hunt7.o glue5.o
hunt70: refer3.o hunt9.o shell.o deliv2.o hunt8.o glue4.o
	cc -i hunt?.o refer3.o glue5.o glue4.o shell.o deliv2.o -o hunt70

hunt40: hunt1.o hunt2.o hunt3.o hunt5.o hunt6.o hunt7.o glue5.o
hunt40: refer3.o hunt9.o shell.o deliv2.o hunt8.o glue4.o
	cc -n hunt?.o refer3.o glue5.o glue4.o shell.o deliv2.o -o hunt40

deliv: deliv1.o deliv2.o
	cc -s -n deliv?.o -o deliv

refpart: refer0.o refer1.o refer2.o refer3.o refer4.o refer5.o
refpart: refer6.o refer7.o refer8.o deliv2.o glue4.o
	cc -s -n refer?.o deliv2.o glue4.o -o refpart

refer70: glue1.o refer1.o refer2.o refer4.o refer5.o refer6.o mkey3.o
refer70: refer7.o refer8.o hunt2.o hunt3.o deliv2.o hunt5.o hunt6.o hunt8.o
refer70: glue3.o hunt7.o hunt9.o glue2.o glue4.o glue5.o refer0.o shell.o
	cc -i glue?.o refer[01245678].o hunt[2356789].o mkey3.o shell.o \
	deliv2.o -o refer70

refer40: glue1.o refer1.o refer2.o refer4.o refer5.o refer6.o mkey3.o
refer40: refer7.o refer8.o hunt2.o hunt3.o deliv2.o hunt5.o hunt6.o hunt8.o
refer40: glue3.o hunt7.o hunt9.o glue2.o glue4.o glue5.o refer0.o shell.o
	cc -n glue?.o refer[01245678].o hunt[2356789].o mkey3.o shell.o \
	deliv2.o -o refer40

whatabout: what1.o what2.o what3.o what4.o shell.o mkey3.o
	cc -s -n what?.o shell.o mkey3.o -o whatabout

refer0.o: refer0.c refer..c
refer1.o: refer1.c refer..c
refer2.o: refer2.c refer..c
refer3.o: refer3.c refer..c
refer4.o: refer4.c refer..c
refer5.o: refer5.c refer..c
refer6.o: refer6.c refer..c
refer7.o: refer7.c refer..c
refer8.o: refer8.c refer..c

deliv2.o: deliv2.c

glue1.o: glue1.c
glue2.o: glue2.c
glue3.o: glue3.c refer..c
glue4.o: glue4.c
glue5.o: glue5.c

hunt1.o: hunt1.c
hunt2.o: hunt2.c refer..c
hunt3.o: hunt3.c refer..c
hunt5.o: hunt5.c
hunt6.o: hunt6.c
hunt7.o: hunt7.c
hunt8.o: hunt8.c
hunt9.o: hunt9.c

inv1.o: inv1.c
inv2.o: inv2.c
inv3.o: inv3.c
inv5.o: inv5.c
inv6.o: inv6.c

mkey1.o: mkey1.c
mkey2.o: mkey2.c
mkey3.o: mkey3.c

shell.o: shell.c

what1.o: what1.c what..c
what2.o: what2.c what..c
what3.o: what3.c what..c
what4.o: what4.c what..c

clean:
	rm -f *.o log mkey inv hunt40 hunt70 refer40 refer70

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
