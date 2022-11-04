# SCCSID: @(#)makefile	3.1	10/15/87
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#

STARTUP=/lib/crt0.o		# /lib/fcrt0.o if no floating point

# yy.h removed -- it relys on y.tab.h, thus it screws up source dependencies
HSRCS =	0.h OPnames.h opcode.h tree.h

CSRCS =	ato.c \
	call.c case.c clas.c const.c conv.c cset.c \
	error.c fdec.c func.c gen.c hash.c \
	lab.c lookup.c lval.c \
	main.c nl.c proc.c put.c \
	rec.c rval.c \
	stat.c string.c subr.c \
	tree.c type.c var.c version.c \
	TRdata.c \
	treen.c putn.c yycopy.c \
	yycosts.c yyerror.c yyget.c yyid.c yylex.c yymain.c yyoptions.c \
	yypanic.c yyparse.c yyprint.c yyput.c yyrecover.c yyseman.c yytree.c

OSRCS=	pas.y

SOURCES = ${HSRCS} ${CSRCS} ${OSRCS}

OBJS =	ato.o \
	call.o case.o clas.o const.o conv.o cset.o \
	error.o fdec.o func.o gen.o hash.o \
	lab.o lookup.o lval.o \
	main.o nl.o proc.o put.o \
	rec.o rval.o \
	stat.o string.o subr.o \
	tree.o type.o var.o version.o \
	TRdata.o \
	treen.o putn.o yycopy.o \
	y.tab.o \
	yycosts.o yyerror.o yyget.o yyid.o yylex.o yymain.o yyoptions.o \
	yypanic.o yyparse.o yyprint.o yyput.o yyrecover.o yyseman.o yytree.o

BASE = main.o version.o putn.o rec.o
OV1 = TRdata.o ato.o call.o case.o clas.o const.o conv.o cset.o \
	stat.o
OV4 = rval.o proc.o
OV2 = gen.o hash.o lab.o lookup.o string.o \
	subr.o tree.o treen.o var.o nl.o put.o
OV5 = fdec.o func.o lval.o type.o
OV3 = yycopy.o yycosts.o yyerror.o yyget.o yyid.o yylex.o yymain.o \
	yyoptions.o yypanic.o yyparse.o yyprint.o yyput.o \
	yyseman.o yytree.o error.o
OV6 = y.tab.o yyrecover.o
OV7 = fprintf.o fread.o fseek.o malloc.o printf.o cvt.o abort.o \
      atof.o ctime.o ecvt.o

all:	pi70 pi40

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

pi70:	${OBJS} pix px_header.out
	cc -i -o pi70 ${OBJS}

pi40:	${OBJS} pix px_header.out
	ar x /lib/libc.a ${OV7}
	ld -n -X ${STARTUP} ${BASE} -Z ${OV1} -Z ${OV2} -Z ${OV3} \
	-Z ${OV4} -Z ${OV5} -Z ${OV6} \
	-Z ${OV7} -L -lv7 -lc -o pi40
	size pi40

install: all
	cp pi1.2strings ${DESTDIR}/usr/lib/pi1.2strings
	chmod 644 ${DESTDIR}/usr/lib/pi1.2strings
	chog bin ${DESTDIR}/usr/lib/pi1.2strings
	cp how_pi ${DESTDIR}/usr/lib/how_pi
	chmod 644 ${DESTDIR}/usr/lib/how_pi
	chog bin ${DESTDIR}/usr/lib/how_pi
	cp how_pix ${DESTDIR}/usr/lib/how_pix
	chmod 644 ${DESTDIR}/usr/lib/how_pix
	chog bin ${DESTDIR}/usr/lib/how_pix
	dd if=px_header.out of=${DESTDIR}/usr/lib/npx_header conv=sync
	chmod 644 ${DESTDIR}/usr/lib/npx_header
	chog bin ${DESTDIR}/usr/lib/npx_header
	cp pi70 ${DESTDIR}/usr/bin/pi70
	-strip ${DESTDIR}/usr/bin/pi70
	chmod 755 ${DESTDIR}/usr/bin/pi70
	chog bin ${DESTDIR}/usr/bin/pi70
	cp pi40 ${DESTDIR}/usr/bin/pi40
	-strip ${DESTDIR}/usr/bin/pi40
	chmod 755 ${DESTDIR}/usr/bin/pi40
	chog bin ${DESTDIR}/usr/bin/pi40
	cp ${DESTDIR}/usr/bin/pi70 ${DESTDIR}/usr/bin/pi
	chmod 755 ${DESTDIR}/usr/bin/pi
	chog bin ${DESTDIR}/usr/bin/pi
	cp pix ${DESTDIR}/usr/bin/pix
	-strip ${DESTDIR}/usr/bin/pix
	chmod 755 ${DESTDIR}/usr/bin/pix
	chog bin ${DESTDIR}/usr/bin/pix
	make clean

pix: pix.c
	cc -n -O -o pix pix.c

y.tab.h y.tab.c: pas.y
	../eyacc/eyacc pas.y >pas.eyacc.list
	ex - y.tab.c < gram

${OBJS}:
	rm -f ./tmp/$*.c
	/usr/ucb/mkstr - pi1.2strings  tmp/ $*.c
	(cd tmp ; cc -I.. -c $*.c ; mv $*.o ../$*.o)
	rm -f ./tmp/$*.c

px_header.out : px_header.c
	cc -O -o px_header.out px_header.c
	strip px_header.out

clean:
	 rm -f *.o ./tmp/*.c y.tab.c y.tab.h pi1.2strings
	 rm -f core px_header.out *.list obj pi70 pi40 pi pix

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info

depend:
	grep '^#[ 	]*include' *.h \
		| sed '/<.*>/d' \
		| sed 's/\(.*\):[^"]*"\([^"]*\)".*/\1: \2/' >makedep
	grep '^#[ 	]*include' *.c \
		| sed '/<.*>/d' \
		| sed 's/:[^"]*"\([^"]*\)".*/: \1/' \
		| sed 's/\.c/.o/' >>makedep
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

yy.h: y.tab.h
TRdata.o: whoami
TRdata.o: 0.h
ato.o: whoami
ato.o: 0.h
call.o: whoami
call.o: 0.h
call.o: tree.h
call.o: opcode.h
case.o: whoami
case.o: 0.h
case.o: tree.h
case.o: opcode.h
clas.o: whoami
clas.o: 0.h
clas.o: tree.h
clas.o: opcode.h
const.o: whoami
const.o: 0.h
const.o: tree.h
conv.o: whoami
conv.o: 0.h
conv.o: opcode.h
cset.o: whoami
cset.o: 0.h
cset.o: tree.h
cset.o: opcode.h
error.o: whoami
error.o: 0.h
error.o: yy.h
fdec.o: whoami
fdec.o: 0.h
fdec.o: tree.h
fdec.o: opcode.h
func.o: whoami
func.o: 0.h
func.o: tree.h
func.o: opcode.h
gen.o: whoami
gen.o: 0.h
gen.o: tree.h
gen.o: opcode.h
hash.o: whoami
hash.o: 0.h
hash.o: yy.h
lab.o: whoami
lab.o: 0.h
lab.o: tree.h
lab.o: opcode.h
lookup.o: whoami
lookup.o: 0.h
lval.o: whoami
lval.o: 0.h
lval.o: tree.h
lval.o: opcode.h
main.o: whoami
main.o: 0.h
main.o: yy.h
nl.o: whoami
nl.o: 0.h
nl.o: opcode.h
proc.o: whoami
proc.o: 0.h
proc.o: tree.h
proc.o: opcode.h
put.o: whoami
put.o: opcode.h
put.o: 0.h
put.o: OPnames.h
rec.o: whoami
rec.o: 0.h
rec.o: tree.h
rec.o: opcode.h
rval.o: whoami
rval.o: 0.h
rval.o: tree.h
rval.o: opcode.h
stat.o: whoami
stat.o: 0.h
stat.o: tree.h
stat.o: opcode.h
string.o: whoami
string.o: 0.h
string.o: send.h
subr.o: whoami
subr.o: 0.h
tree.o: whoami
tree.o: 0.h
type.o: whoami
type.o: 0.h
type.o: tree.h
var.o: whoami
var.o: 0.h
var.o: tree.h
var.o: opcode.h
yycopy.o: 0.h
yycopy.o: yy.h
yycosts.o: whoami
yycosts.o: 0.h
yycosts.o: yy.h
yyerror.o: whoami
yyerror.o: 0.h
yyerror.o: yy.h
yyget.o: whoami
yyget.o: 0.h
yyget.o: yy.h
yyid.o: whoami
yyid.o: 0.h
yyid.o: yy.h
yylex.o: whoami
yylex.o: 0.h
yylex.o: yy.h
yymain.o: whoami
yymain.o: 0.h
yymain.o: yy.h
yyoptions.o: whoami
yyoptions.o: 0.h
yyoptions.o: yy.h
yypanic.o: whoami
yypanic.o: 0.h
yypanic.o: yy.h
yyparse.o: whoami
yyparse.o: 0.h
yyparse.o: yy.h
yyprint.o: whoami
yyprint.o: 0.h
yyprint.o: yy.h
yyput.o: whoami
yyput.o: 0.h
yyput.o: tree.h
yyput.o: yy.h
yyrecover.o: whoami
yyrecover.o: 0.h
yyrecover.o: yy.h
yyseman.o: whoami
yyseman.o: 0.h
yyseman.o: yy.h
yytree.o: whoami
yytree.o: 0.h
yytree.o: tree.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
