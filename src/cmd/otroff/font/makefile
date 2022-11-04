# SCCSID: @(#)makefile	3.0	4/22/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

OBJECTS= ftB.o ftBC.o ftC.o ftCE.o ftCI.o ftCK.o ftCS.o ftCW.o ftG.o ftGI.o \
	ftGM.o ftGR.o ftI.o ftL.o ftLI.o ftPA.o ftPB.o ftPI.o ftR.o ftS.o \
	ftSB.o ftSI.o ftSM.o ftUD.o ftXM.o

SOURCES= ftB.c ftBC.c ftC.c ftCE.c ftCI.c ftCK.c ftCS.c ftCW.c ftG.c ftGI.c \
	ftGM.c ftGR.c ftI.c ftL.c ftLI.c ftPA.c ftPB.c ftPI.c ftR.c ftS.c \
	ftSB.c ftSI.c ftSM.c ftUD.c ftXM.c

all:	fnts

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install:	all
	for i in ${OBJECTS};\
	do \
		d=`basename $$i .o`; \
		cp $$i ${DESTDIR}/usr/orphan/usr/lib/font/$$d;\
		chmod 444 ${DESTDIR}/usr/orphan/usr/lib/font/$$d; \
		chog bin ${DESTDIR}/usr/orphan/usr/lib/font/$$d; \
	done

cmp:	all
	for i in ${OBJECTS}; \
	do \
		cmp $$i ${DESTDIR}/usr/orphan/usr/lib/font/`basename $$i .o`; \
	done

fnts:	$(OBJECTS)

.c.o:
	cc -c $<; strip $@

mkfont:mkfont.c mkfont1.c -o mkfont
	cc -n -s mkfont.c

clean:
	rm -f *.o

clobber: clean
	rm -f mkfont
	sccs clean

tags:	sources
	ctags ${SOURCES}

sccsinfo:
	sccs info
