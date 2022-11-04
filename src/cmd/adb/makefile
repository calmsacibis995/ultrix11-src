# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

SOURCES= defs.h mac.h machine.h mode.h access.c command.c expr.c findfn.c \
	format.c input.c opset.c main.c message.c output.c pcs.c print.c \
	runpcs.c setup.c sym.c

# access.o message.o and sym.o are explicitly stated below
OBJECTS= command.o expr.o findfn.o \
	format.o input.o opset.o main.o \
	output.o pcs.o print.o \
	runpcs.o setup.o
CFLAGS = -O

all:	adb70 adb40

adb:	all

sources: ${SOURCES}

${SOURCES} dofix message.fix:
	sccs get $@

install: all
# remove current versions if any
	rm -f ${DESTDIR}/bin/sid/adb ${DESTDIR}/bin/nsid/adb
	cp adb70 ${DESTDIR}/bin/sid/adb
	cp adb40 ${DESTDIR}/bin/nsid/adb
	-strip ${DESTDIR}/bin/sid/adb ${DESTDIR}/bin/nsid/adb
	chmod 755 ${DESTDIR}/bin/sid/adb ${DESTDIR}/bin/nsid/adb
	chog bin ${DESTDIR}/bin/sid/adb ${DESTDIR}/bin/nsid/adb
# attempt link only if files already in place
	-(if [ -f ${DESTDIR}/bin/adb ] ;\
	then \
		mv ${DESTDIR}/bin/adb ${DESTDIR}/bin/adb.junk ;\
	else \
		true ;\
	fi)
	ln ${DESTDIR}/bin/sid/adb ${DESTDIR}/bin/adb
	make clean

clean:
	rm -f *.o message.z
	rm -f *.s adb70 adb40

clobber:	clean
	sccs clean

adb70:	message.o access.o ${OBJECTS} sym.o
	cc -i -o adb70 message.o access.o ${OBJECTS} sym.o
	@echo adb70 DONE
	@echo

adb40:	message.z access.o ${OBJECTS} sym.o
	/usr/local/calcov -a -n -o adb40 -S 8192 -B access.o sym.o message.z -Z ${OBJECTS}
	@echo adb40 DONE
	@echo

access.o:	defs.h
command.o:	defs.h
expr.o:		defs.h
findrtn.o:	defs.h
format.o:	defs.h
input.o:	defs.h
main.o:		defs.h
output.o:	defs.h
pcs.o:		defs.h
print.o:	defs.h
runpcs.o:	defs.h
setup.o:	defs.h
sym.o:		defs.h
message.o:	mac.h mode.h machine.h message.c

message.z:	mac.h mode.h machine.h message.c dofix message.fix
	${CC} -S message.c
	@sh dofix message
	as - -V -o message.z message.s
	rm message.s

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
