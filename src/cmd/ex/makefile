# SCCSID: @(#)makefile	3.1	5/16/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
VERSION=3.7
#
# Ex skeletal makefile for version ULTRIX-11
#
# NB: This makefile doesn't indicate any dependencies on header files.
#
# Ex is very large - this version will not fit on PDP-11's without overlay
# software.  Things that can be turned off to save
# space include LISPCODE (-l flag, showmatch and lisp options), UCVISUAL
# (visual \ nonsense on upper case only terminals), CHDIR (the undocumented
# chdir command.)  CRYPT includes code to handle the -x option to allow
# encrypted files.  Defining CRYPT drags in most of stdio (from the call
# to getpass) and makes vi about 4k text and 4k data bigger).
#
EXSRC=/usr/src/cmd/ex
BINDIR=	/usr/bin
LIBDIR=	/usr/lib
XSTR=	xstr
AS=	as -V
DEBUGFLAGS=	-DTRACE
NONDEBUGFLAGS=	-O
DEB= ${NONDEBUGFLAGS} # or ${DEBUGFLAGS} to debug
#DEB=	${DEBUGFLAGS}	# or ${NONDEBUGFLAGS} for no debug
CFLAGS=	-DTABS=8 ${DEB} -DCTAGS -DCHDIR
TERMLIB=	-ltermlib
MKSTR=	mkstr
PRINTF=	ovprintf.o ovdoprnt.o
PRINTFS=ovprintf.c ovdoprnt.s
INCLUDE=/usr/include
PR=	pr
OB1=	ex.o ex_addr.o ex_cmds.o ex_cmds2.o ex_cmdsub.o \
	ex_data.o ex_get.o ex_io.o ex_put.o ex_re.o \
	ex_set.o ex_subr.o ex_temp.o ex_unix.o \
	ex_vget.o ex_vmain.o ex_voper.o \
	ex_vops.o ex_vops2.o ex_vops3.o ex_vput.o ex_vwind.o \
	${PRINTF} bcopy.o
OB2=	ex_tty.o ex_v.o ex_vadj.o
OVOB2=	ex_tty.z ex_v.z ex_vadj.z
OBJS=	${OB1} ${OB2}
OVOBJS=	${OB1} ${OVOB2}

SRCS=	ex.h ex_argv.h ex_re.h ex_temp.h ex_tty.h ex_tune.h ex_vars.h ex_vis.h\
	ex.c ex_addr.c ex_cmds.c ex_cmds2.c ex_cmdsub.c \
	ex_data.c ex_get.c ex_extern.c ex_io.c ex_put.c ex_re.c \
	ex_set.c ex_subr.c ex_temp.c ex_tty.c ex_unix.c \
	ex_v.c ex_vadj.c ex_vget.c ex_vmain.c ex_voper.c \
	ex_vops.c ex_vops2.c ex_vops3.c ex_vput.c ex_vwind.c \
	${PRINTFS} bcopy.c rofix exrecover.c expreserve.c

INCLDS= ex.h ex_argv.h ex_re.h ex_temp.h ex_tty.h ex_tune.h ex_vars.h ex_vis.h

.SUFFIXES:
.SUFFIXES: .o .z .c .s
.c.o:
	${MKSTR} - ex${VERSION}strings x $*.c
	${CC} -E ${CFLAGS} ${OVFLG} x$*.c | ${XSTR} -c -
	rm -f x$*.c
	${CC} ${CFLAGS} ${OVFLG} -c x.c 
	mv x.o $*.o

.c.z:
	${MKSTR} - ex${VERSION}strings x $*.c
	${CC} -E -DSMALL ${CFLAGS} ${OVFLG} x$*.c | ${XSTR} -c -
	rm -f x$*.c
	${CC} -DSMALL ${CFLAGS} ${OVFLG} -c x.c 
	mv x.o $*.z

all:	ex70 ex40 exrecover expreserve

sources: ${SRCS}

${SRCS}:
	sccs get $@

# 11/44, 70 seperate i/d space machines
ex:
ex70:	ex_extern.o ${OBJS} strings.o
	${CC} ex_extern.o -i -o ex70\
	    -Z ex_voper.o ex_vops.o ex_vops2.o ex_vops3.o \
	    -Z ex_set.o ex_re.o ex_io.o ex_tty.o ex_unix.o ex.o ex_v.o \
	    -L ex_addr.o ex_cmds.o ex_cmds2.o ex_cmdsub.o ex_get.o ex_temp.o\
	       ex_vadj.o ex_vget.o ex_vmain.o ex_vwind.o\
	       ex_vput.o ex_put.o ex_subr.o ${PRINTF} strings.o \
	       ex_data.o ${TERMLIB}
	@echo DONE

# 11/23, 34, 40 and other non split I/D machines
# each of the 5 overlays must stay less than 16K.
ex40:	ex_extern.z ${OVOBJS} strings.z
	${CC} ex_extern.z ex_vget.o -n -o ex40\
	    -Z ex_cmds.o ex_cmds2.o ex_cmdsub.o ex_re.o \
	    -Z ex_vadj.z ex_vmain.o ex_voper.o ex_vwind.o ex_vops3.o\
	    -Z ex_v.z ex_vops.o ex_vops2.o ex_vput.o\
	    -Z ex_get.o ex_temp.o ex_tty.z ex_unix.o ex_addr.o \
	       ex.o ex_set.o \
	    -Z ex_put.o ex_subr.o \
	    -L ex_io.o ex_data.o strings.z \
		${PRINTF} ${TERMLIB}
	@echo DONE

# ---------------------------------------------------
# Start of CRYPT stuff
#
# both crypt versions:
ex_crypt: cpsources docrypt

cpsources: ${SRCS} makefile forceit
	@echo "copying sources to crypt directory..."
	cp ${SRCS} makefile ${EXSRC}/crypt

docrypt: forceit
	@echo
	@echo "making ex/vi crypt version"
	@echo "cleaning up old objects first..."
	@make crypt_clean
	-(cd ${EXSRC}/crypt; make CFLAGS="${CFLAGS} -DCRYPT" ex70_crypt ex40_crypt)
	@echo "crypt versions DONE"
	
# CRYPT_70 version:
# JSD: moved ex_temp.o into overlay 1 from base segment
#
ex70_crypt:	ex_extern.o ${OBJS} strings.o
	${CC} ex_extern.o -i -o ex70_crypt\
	    -Z ex_temp.o ex_voper.o ex_vops.o ex_vops2.o ex_vops3.o \
	    -Z ex_set.o ex_re.o ex_io.o ex_tty.o ex_unix.o ex.o ex_v.o \
	    -L ex_addr.o ex_cmds.o ex_cmds2.o ex_cmdsub.o ex_get.o\
	       ex_vadj.o ex_vget.o ex_vmain.o ex_vwind.o\
	       ex_vput.o ex_put.o ex_subr.o ${PRINTF} strings.o \
	       ex_data.o ${TERMLIB}
	size ex70_crypt
	@echo "ex70_crypt DONE"
	@echo

# CRYPT_40 version:
# JSD: moved ex_io.o into overlay 4 from base segment
ex40_crypt:	ex_extern.z ${OVOBJS} strings.z
	${CC} ex_extern.z ex_vget.o -n -o ex40_crypt \
	    -Z ex_cmds.o ex_cmds2.o ex_cmdsub.o ex_re.o \
	    -Z ex_vadj.z ex_vmain.o ex_voper.o ex_vwind.o ex_vops3.o\
	    -Z ex_v.z ex_vops.o ex_vops2.o ex_vput.o\
	    -Z ex_get.o ex_temp.o ex_tty.z ex_unix.o ex_addr.o \
	       ex.o ex_set.o ex_io.o \
	    -Z ex_put.o ex_subr.o \
	    -L ex_data.o strings.z \
		${PRINTF} ${TERMLIB}
	size ex40_crypt
	@echo "ex40_crypt DONE"
	@echo

# End of CRYPT build stuff
# ---------------------------------------------------

# ex_vars.h:
# 	csh makeoptions ${CFLAGS}

strings.o: 
	${XSTR}
	${CC} -c -S xs.c
#	ed - <:rofix xs.s
	${AS} -o strings.o xs.s
	rm xs.s

strings.z: rofix
	${XSTR}
	${CC} -c -S xs.c
	ed - <rofix xs.s
	${AS} -o strings.z xs.s
	rm xs.s
	
exrecover: ex_extern.o exrecover.o
	${CC} ${CFLAGS} -o exrecover ex_extern.o exrecover.o

expreserve: ex_extern.o expreserve.o
	${CC} ${CFLAGS} -o expreserve ex_extern.o expreserve.o

clean:
#	If we dont have ex we cant make it so dont rm ex_vars.h
	rm -f *.o *.z x*.[cs] strings core errs trace log
	rm -f ex70 ex40 exrecover expreserve
	rm -f ex${VERSION}strings

crypt_clean:
	-(cd ${EXSRC}/crypt; rm -f *.o *.z x*.[cs] \
		strings core errs trace log ex${VERSION}strings)

clobber: clean
	sccs clean

# install in standard place (/usr/bin)
install: ex70 ex40 exrecover expreserve
	-(cd ${DESTDIR}${BINDIR}; rm -f e ex vi view edit)
	cp ex70 ex40 ${DESTDIR}${BINDIR}
	-strip ${DESTDIR}${BINDIR}/ex40 ${DESTDIR}${BINDIR}/ex70
	-(cd ${DESTDIR}${BINDIR}; \
		chog bin ex70 ex40; \
		chmod 755 ex70 ex40; \
		cp ex70 ex; \
		chog bin ex; \
		chmod 1755 ex; \
		ln ex e; ln ex edit; ln ex vi; ln ex view)
	-strip exrecover expreserve
	cp ex${VERSION}strings ${DESTDIR}${LIBDIR}/ex${VERSION}strings
	cp exrecover ${DESTDIR}${LIBDIR}/ex${VERSION}recover
	-strip ${DESTDIR}${LIBDIR}/ex${VERSION}recover
	cp expreserve ${DESTDIR}${LIBDIR}/ex${VERSION}preserve
	(cd ${DESTDIR}${LIBDIR}; \
		chog bin ex${VERSION}strings; \
		chmod 644 ex${VERSION}strings; \
		chown root ex${VERSION}recover ex${VERSION}preserve; \
		chgrp bin ex${VERSION}recover ex${VERSION}preserve; \
		chmod 4755 ex${VERSION}recover ex${VERSION}preserve)

	-(if [ ! -d ${DESTDIR}/usr/preserve ] ; \
	then \
		mkdir ${DESTDIR}/usr/preserve; \
	else \
		true; \
	fi)
	make clean

exrecover.o:	${EXH} ex_temp.h ex_tty.h exrecover.c
	cc -O -c exrecover.c

expreserve.o: expreserve.c
	cc -O -c expreserve.c

ex_extern.o: ex_extern.c ${INCLDS}
ex_extern.z: ex_extern.c ${INCLDS}

EXH=	ex.h ex_tune.h ex_vars.h
ex.o:		${EXH} ex_argv.h ex_temp.h ex_tty.h ex.c
ex_addr.o:	${EXH} ex_re.h ex_addr.c
ex_cmds.o:	${EXH} ex_argv.h ex_temp.h ex_tty.h ex_vis.h ex_cmds.c
ex_cmds2.o:	${EXH} ex_argv.h ex_temp.h ex_tty.h ex_vis.h ex_cmds2.c
ex_cmdsub.o:	${EXH} ex_argv.h ex_temp.h ex_tty.h ex_vis.h ex_cmdsub.c
ex_data.o:	${EXH} ex_tty.h ex_data.c
ex_get.o:	${EXH} ex_tty.h ex_get.c
ex_io.o:	${EXH} ex_argv.h ex_temp.h ex_tty.h ex_vis.h ex_io.c
ex_put.o:	${EXH} ex_tty.h ex_vis.h ex_put.c
ex_re.o:	${EXH} ex_re.h ex_re.c
ex_set.o:	${EXH} ex_temp.h ex_tty.h ex_set.c
ex_subr.o:	${EXH} ex_re.h ex_tty.h ex_vis.h ex_subr.c
ex_temp.o:	${EXH} ex_temp.h ex_tty.h ex_vis.h ex_temp.c
ex_tty.o:	${EXH} ex_tty.h ex_tty.c
ex_tty.z:	${EXH} ex_tty.h ex_tty.c
ex_unix.o:	${EXH} ex_temp.h ex_tty.h ex_vis.h ex_unix.c
ex_v.o:		${EXH} ex_re.h ex_tty.h ex_vis.h ex_v.c
ex_v.z:		${EXH} ex_re.h ex_tty.h ex_vis.h ex_v.c
ex_vadj.o:	${EXH} ex_tty.h ex_vis.h ex_vadj.c
ex_vadj.z:	${EXH} ex_tty.h ex_vis.h ex_vadj.c
ex_vget.o:	${EXH} ex_tty.h ex_vis.h ex_vget.c
ex_vmain.o:	${EXH} ex_tty.h ex_vis.h ex_vmain.c
ex_voper.o:	${EXH} ex_tty.h ex_vis.h ex_voper.c
ex_vops.o:	${EXH} ex_tty.h ex_vis.h ex_vops.c
ex_vops2.o:	${EXH} ex_tty.h ex_vis.h ex_vops2.c
ex_vops3.o:	${EXH} ex_tty.h ex_vis.h ex_vops3.c
ex_vput.o:	${EXH} ex_tty.h ex_vis.h ex_vput.c
ex_vwind.o:	${EXH} ex_tty.h ex_vis.h ex_vwind.c
ovprintf.o:	ovprintf.c
ovdoprnt.o:	ovdoprnt.s
bcopy.o:	bcopy.c

tags:	${SRCS} ${INCLDS}
	ctags ${SRCS} ${INCLDS}

sccsinfo:
	sccs info

forceit:
	@true
#
# Based on ULTRIX-32 Makefile
# Make the versions of the source that should not contain
# any crypt code.  Essentially filters out all text between
# #ifdef CRYPT and #endif.
#
nocrypt:
	for file in `grep -l "^#ifdef CRYPT" $(SRCS) $(INCLDS)`;\
	do \
	    sed '/^#ifdef[      ]CRYPT/,/^#endif/d' $$file > $$file.NOCRYPT;\
	done
