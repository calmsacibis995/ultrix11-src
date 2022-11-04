# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

LESSONS = files editor vi morefiles macros eqn C

SOURCES= lrndef lrnref Init.vi Init.eqn \
	copy.c dounit.c learn.c list.c \
	makpipe.c maktee.c mem.c mysys.c selsub.c selunit.c \
	start.c whatnow.c wrapup.c lcount.c tee.c

OBJECTS = copy.o dounit.o learn.o list.o mem.o \
	makpipe.o maktee.o mysys.o selsub.o selunit.o \
	start.o whatnow.o wrapup.o

CFLAGS = -O
LLIB	= ${DESTDIR}/usr/lib/learn

all:	learn tee lcount

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install:	all
	cp learn ${DESTDIR}/usr/bin/learn
	-strip ${DESTDIR}/usr/bin/learn
	chmod 755 ${DESTDIR}/usr/bin/learn
	chog bin ${DESTDIR}/usr/bin/learn
	cp tee ${LLIB}/tee
	-strip ${LLIB}/tee
	chmod 755 ${LLIB}/tee
	chog bin ${LLIB}/tee
	cp lcount ${LLIB}/lcount
	-strip ${LLIB}/lcount
	chmod 755 ${LLIB}/lcount
	chog bin ${LLIB}/lcount
	cp Init.vi ${LLIB}/vi/Init
	chmod 755 ${LLIB}/vi/Init
	chog bin ${LLIB}/vi/Init
	cp Init.eqn ${LLIB}/eqn/Init
	chmod 755 ${LLIB}/eqn/Init
	chog bin ${LLIB}/eqn/Init
	@echo "Now do 'make lessons' if you need to extract the lesson archives"
	@echo "Then do 'make play; make log' to make playpen and log directories"
	make clean

cmp:	all
	cmp learn ${DESTDIR}/bin/learn
	cmp tee $(LLIB)/tee
	cmp lcount $(LLIB)/lcount

learn: ${OBJECTS} Init.vi Init.eqn
	cc -n -o learn ${CFLAGS} ${OBJECTS}

learn.o: lrndef

lcount tee:
	cc ${CFLAGS} -n $@.c -o $@

lessons:	${LESSONS}

${LESSONS}:
	-rm -r ${LLIB}/$@
	mkdir ${LLIB}/$@
	chmod 775 ${LLIB}/$@
	chog bin ${LLIB}/$@
	(cd ${LLIB}/$@; ar x ../$@.a)

play log:
	-rm -r ${LLIB}/$@
	mkdir ${LLIB}/$@
	chmod 777 ${LLIB}/$@
	chog bin ${LLIB}/$@

check:
	-@test -r ${LLIB}/tee || echo 'tee not present; make tee'
	-@test -r ${LLIB}/lcount || echo 'lcount not present; make lcount'
	-@test -r ${LLIB}/play || echo 'play directory not present; make play'
	-@test -r ${LLIB}/log || echo 'log directory not present; make log'
	-@for i in ${LESSONS}; do test -r ${LLIB}/$$i/L0 || echo $$i not unarchived, make $$i; done

# DEPENDENCY list
lrndef:	lrndef
lrnref: lrnref
copy.o: copy.c
dounit.o: dounit.c
learn.o: learn.c
list.o: list.c
makpipe.o: makpipe.c
maktee.o: maktee.c
mem.o: mem.c
mysys.o: mysys.c
selsub.o: selsub.c
selunit.o: selunit.c
start.o: start.c
whatnow.o: whatnow.c
wrapup.o: wrapup.c
lcount.o: lcount.c
tee.o: tee.c

clean:
	rm -f *.o learn tee lcount

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
