# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
# makefile for ltf
#
CFLAGS=-DU11 -O
SOURCES = filenames.c filetype.c initvol.c ltf.c \
	ltfvars.c mstrcmp.c odm.c putdir.c putfile.c \
	scantape.c xtractf.c
HEADERS= ltferrs.h ltfdefs.h filetypes.h
  
# Objects NOT to run xstr on
OBJ40 = ltfvars40.o

# Objects it is OK to run xstr on:
OBJ40X= filetype40.o filenames40.o initvol40.o ltf40.o \
	mstrcmp40.o odm40.o putdir40.o \
	putfile40.o scantape40.o xtractf40.o

OBJ70 = ltfvars.o filetype.o filenames.o initvol.o ltf.o \
	mstrcmp.o odm.o putdir.o \
	putfile.o scantape.o xtractf.o

# C library routines we put into overlays
LIBOBJ = fread.o fseek.o malloc.o printf.o cvt.o \
      atof.o ctime.o ecvt.o doscan.o flsbuf.o

# moved expnum(), showhelp() and usage() into filenames.c (from ltf.c)
# nothing in base except required libc.a things and strings.o
base =
ovr1 = odm40.o scantape40.o
ovr2 = putfile40.o
ovr3 = filenames40.o filetype40.o initvol40.o mstrcmp40.o ltfvars40.o
ovr4 = putdir40.o xtractf40.o ltf40.o
ovr5 = ${LIBOBJ}

all:	ltf40 ltf70

sources: ${SOURCES} ${HEADERS}

${SOURCES} ${HEADERS}:
	sccs get $@

# Split I&D version
ltf70:	${OBJ70}
	cc -i ${OBJ70} -o ltf70

${OBJ70}:
	cc -c ${CFLAGS} $<

# Overlayed version
# 	base segment of 16K and
# 	five overlays of 8K
#	strings.o must be last in this list because
#	it sometimes changes, and goes into the base segment.
ltf40:	${OBJ40} ${OBJ40X} strings.o
	ar x /lib/libc.a ${LIBOBJ}
	ld -n -X -o ltf40 /lib/crt0.o $(base) \
	-Z $(ovr1) \
	-Z $(ovr2) \
	-Z $(ovr3) \
	-Z $(ovr4) \
	-Z $(ovr5) \
	-L strings.o -lc
	 size ltf40
	 rm -f ${LIBOBJ}
	 @echo

${OBJ40}:
	cc -c ${CFLAGS} `basename $@ 40.o`.c
	mv `basename $@ 40.o`.o $@

${OBJ40X}:
	cc -E ${CFLAGS} `basename $@ 40.o`.c | xstr -c -
	cc -c ${CFLAGS} x.c
	mv x.o $@
	-rm x.c

strings.o: strings dofix
	xstr
	cc -S xs.c
	ex - <dofix xs.s
	as -V - -o strings.o xs.s
	rm xs.s xs.c

install: all
	cp ltf40 ${DESTDIR}/bin/nsid/ltf
	-strip ${DESTDIR}/bin/nsid/ltf
	chog bin ${DESTDIR}/bin/nsid/ltf
	chmod 755 ${DESTDIR}/bin/nsid/ltf

	cp ltf70 ${DESTDIR}/bin/sid/ltf
	-strip ${DESTDIR}/bin/sid/ltf
	chog bin ${DESTDIR}/bin/sid/ltf
	chmod 755 ${DESTDIR}/bin/sid/ltf

	rm -f ${DESTDIR}/bin/ltf
	ln ${DESTDIR}/bin/sid/ltf ${DESTDIR}/bin/ltf

	make clean

# make multi-module xref listing
#.mml:
#	xref *.h *.c > ltf.lis
#	pr50 -HREADME.1 -l README.1 > readme.1
#	lpr readme.1 ltf.lis

clean:
	rm -f core a.out *.o log ltf40 ltf70
	rm -f x.c xs.c
	cat /dev/null > strings

clobber: clean
	sccs clean

tags:	${SOURCES} ${HEADERS}
	ctags ${SOURCES} ${HEADERS}

sccsinfo:
	sccs info

# these just specify source/destination dependencies
filenames.o filetype.o initvol.o ltf.o:		ltfdefs.h ltferrs.h filetypes.h
ltfvars.o mstrcmp.o odm.o putdir.o putfile.o:	ltfdefs.h ltferrs.h filetypes.h
scantape.o xtractf.o:				ltfdefs.h ltferrs.h filetypes.h
filenames40.o filetype40.o initvol40.o ltf40.o:		ltfdefs.h ltferrs.h filetypes.h
ltfvars40.o mstrcmp40.o odm40.o putdir40.o putfile40.o:	ltfdefs.h ltferrs.h filetypes.h
scantape40.o xtractf40.o:				ltfdefs.h ltferrs.h filetypes.h
filenames40.o: filenames.c
filetype40.o: filetype.c
initvol40.o: initvol.c
ltf40.o: ltf.c
ltfvars40.o: ltfvars.c
mstrcmp40.o: mstrcmp.c
odm40.o: odm.c
putdir40.o: putdir.c
putfile40.o: putfile.c
scantape40.o: scantape.c
xtractf40.o: xtractf.c
