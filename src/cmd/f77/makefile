# SCCSID: @(#)makefile	3.1	10/9/87
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	Makefile for a Fortran 77 compiler
#	running on the PDP11, generating code for the PDP11,
#	using the second pass of the Ritchie C compiler as code generator.

SOURCES = fltused.s main.c init.c lex.c proc.c equiv.c data.c \
	expr.c exec.c intr.c io.c misc.c error.c put.c \
	putdmr.c pdp11.c pdp11x.c gram.head gram.dcl gram.expr \
	gram.exec gram.io driver.c fio.h
DEFS = _expr.fix _gram.fix _intr.fix datatotext _putdmr.fix \
	defines defs dmrdefs drivedefs dofix ftypes pdp11defs tokens

OBJECTS = fltused.o main.o init.o gram.o lex.o proc.o equiv.o data.o \
	expr.o exec.o intr.o io.o misc.o error.o put.o \
	putdmr.o pdp11.o pdp11x.o
OVOBJ =	gram.z misc.z lex.z fltused.o main.z init.z proc.z equiv.z \
	data.z expr.z exec.z intr.z io.z put.z error.z\
	putdmr.z pdp11.z pdp11x.z
ARCHOBJ= fopen.o fprintf.o sprintf.o ungetc.o filbuf.o doprnt.o strout.o \
	flsbuf.o findiop.o atof.o calloc.o malloc.o
ARCH2OBJ= ffltpr.o

CFL = -DHERE=PDP11 -DTARGET=PDP11 -DFAMILY=DMR -DOVERLAY

# used by driver.o (/usr/bin/f77 part)
CFLAGS = -O ${CFL}

# "-DOVOPT" automatically defines "-DDOSTRINGS", which allows
# mkstr to be run using references to /usr/lib/f77_strings for
# error().  For SID version, just define "-DDOSTRINGS", since 
# text space is at a premium there.
OVCFL = -N -O ${CFL} -DOVOPT

# used by other Split I & D '.o' files:
CFLAGS+STR = ${CFLAGS} -DDOSTRINGS

AS = as -V -
LDFLAGS = -i
LIB = ${DESTDIR}/usr/lib
BIN = ${DESTDIR}/usr/bin

.SUFFIXES:
.SUFFIXES: .o .z .c .s

all:	f77 F77pass1 F77pass1ov

${SOURCES} ${DEFS}:
	sccs get $@

sources: ${SOURCES}

compiler:	f77 f77pass1
	touch compiler

f77:	driver.o pdp11x.o
	cc -n driver.o pdp11x.o -o f77
	size f77
	@echo

driver.o:	driver.c
	${CC} ${CFLAGS} -c driver.c

pdp11x.o:	pdp11x.c
	${CC} ${CFLAGS} -c pdp11x.c

f77pass1: warning
	@true

F77pass1: ${OBJECTS} sid_xs.o
	@echo LOAD
	${CC} ${LDFLAGS} ${OBJECTS} sid_xs.o -o f77pass1
	size f77pass1
	@echo

f77pass1id: f77pass1
F77pass1id: F77pass1

f77pass1ov:  warning
	@true

# WARNING: If f77_strings (error) file does not exist, and the overlay version
# is made without first making the Split I & D version, then string pointers
# will be bogus for SID version, since order of extraction is different if
# OVERLAY version is compiled with an initially empty f77_strings. You MUST
# make SID version, then leave f77_strings there for OVERLAY version to access.
# All of this would be unnecessary if the order of the build for objects was
# the same.
warning:
	@(echo "You MUST make both versions of the compiler sequentially,")
	@(echo "because a common f77_strings file is generated.  You can't")
	@(echo "build one piece; type \"make all\" so you build it right.");

F77pass1ov: ${OVOBJ} ${ARCHOBJ} ${ARCH2OBJ} xs.o
	@echo LOAD
	/usr/local/calcov -a -n -o f77pass1ov -B xs.o ${ARCH2OBJ} -Z ${OVOBJ} ${ARCHOBJ}
	size f77pass1ov
	@echo

gram.c:	gram.head gram.dcl gram.expr gram.exec gram.io tokdefs
	( sed <tokdefs "s/#define/%token/" ;\
		cat gram.head gram.dcl gram.expr gram.exec gram.io ) >gram.in
	${YACC} ${YFLAGS} gram.in
	@echo "(expect 4 shift/reduce)"
	mv y.tab.c gram.c
	rm gram.in

tokdefs: tokens
	grep -n . <tokens | sed "s/\([^:]*\):\(.*\)/#define \2 \1/" >tokdefs

machdefs: pdp11defs
	cp pdp11defs machdefs

install: all
	cp f77 ${BIN}/f77
	-strip ${BIN}/f77
	chmod 755 ${BIN}/f77
	chog bin ${BIN}/f77
	cp f77pass1 f77pass1ov f77_strings ${LIB}
	cp f77pass1 ${LIB}/f77pass1id
	-strip ${LIB}/f77pass1 ${LIB}/f77pass1id ${LIB}/f77pass1ov
	chmod 755 ${LIB}/f77pass1 ${LIB}/f77pass1id ${LIB}/f77pass1ov
	chmod 444 ${LIB}/f77_strings
	chog bin ${LIB}/f77pass1 ${LIB}/f77pass1id ${LIB}/f77pass1ov
	chog bin ${LIB}/f77_strings
	make clean

${ARCHOBJ} ${ARCH2OBJ}: /lib/libc.a
	ar x /lib/libc.a $@

xs.c:
	xstr
	@mv strings strings.OV
	@cat /dev/null > strings

xs.o:	datatotext
	cc -S -O xs.c
	ed - xs.s <datatotext
	as - -V -o xs.o xs.s
	@rm xs.s

# note: above xs.c depends on "strings" also...
sid_xs.c:
	xstr
	mv xs.c sid_xs.c
	@mv strings strings.ID
	@cat /dev/null > strings

sid_xs.o:
	cc -S -O sid_xs.c
	as - -V -o sid_xs.o sid_xs.s
	@rm sid_xs.s

# Fun, fun, fun.  Run the pre-processor to get error() subbed in,
# run mkstr to remove the error messages, run xstr to remove
# any other strings, then compile to assembly, then do any
# assembly tweeking that is left, and finally assemble it
# and remove all the tmp files. (whew!)

.c.z:	dofix
	${CC} -E ${OVCFL} $< >$*.i
	mkstr - f77_strings x $*.i
	xstr -c - < x$*.i
	${CC} -S ${OVCFL} x.c
	mv x.s $*.s
	@sh dofix $*
	${AS} -o $*.z $*.s
	rm -f $*.i x$*.i x.c $*.s
	@echo

error.z: error.c
	${CC} -S ${OVCFL} error.c
	${AS} -o error.z error.s
	@rm error.s
	@echo

# SID version with error strings extracted (mkstr part)
.c.o:
	${CC} -E ${CFLAGS+STR} $< >$*.i
	mkstr - f77_strings x $*.i
# don't really need to run xstr, but no way around it here...
	xstr -c - < x$*.i
	${CC} -S -O x.c
	mv x.s $*.s
	${AS} -o $*.o $*.s
	rm -f $*.i x$*.i x.c $*.s
	@echo

error.o: error.c
	${CC} -S ${CFLAGS+STR} error.c
	${AS} -o error.o error.s
	@rm error.s
	@echo

clean:
	rm -f f77 f77pass1 f77pass1ov f77_strings strings strings.OV strings.ID
	rm -f machdefs gram.c tokdefs compiler sid_xs.c xs.c log core a.out
	rm -f *.o ${OVOBJ}
	cat /dev/null > strings

clobber: clean
	sccs clean

data.o data.z:		defs ftypes defines machdefs
driver.o:		defines machdefs drivedefs ftypes
equiv.o equiv.z:	defs ftypes defines machdefs
error.o error.z:	defs ftypes defines machdefs
exec.o exec.z:		defs ftypes defines machdefs
expr.o expr.z:		defs ftypes defines machdefs
init.o init.z:		defs ftypes defines machdefs
intr.o intr.z:		defs ftypes defines machdefs
io.o io.z:		defs ftypes defines machdefs
lex.o lex.z:		defs ftypes defines machdefs tokdefs 
main.o main.z:		defs ftypes defines machdefs
misc.o misc.z:		defs ftypes defines machdefs
pdp11.o pdp11.z:	defs ftypes defines machdefs dmrdefs
pdp11x.o pdp11x.z:	defines machdefs
proc.o proc.z:		defs ftypes defines machdefs machdefs 
put.o put.z:		defs ftypes defines machdefs dmrdefs 
putdmr.o putdmr.z:	defs ftypes defines machdefs dmrdefs
${OVOBJ}: dofix
expr.z: _expr.fix
gram.z: _gram.fix
intr.z: _intr.fix
putdmr.z: _putdmr.fix

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
