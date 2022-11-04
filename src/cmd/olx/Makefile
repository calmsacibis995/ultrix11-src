# SCCSID: @(#)Makefile	3.1	10/21/87
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
CFLAGS= -DEFLG
LIBB= -ljobs
SOURCES= cmx1.c cmx2.c cpx.c fpx.c hkx1.c hkx2.c hpx1.c hpx2.c \
	hxx1.c hxx2.c lpx.c memx1.c memx2.c mtx1.c mtx2.c \
	rax1.c rax2.c rkx1.c rkx2.c rlx1.c rlx2.c rpx1.c rpx2.c \
	sysx_m.c sysx_r.c sysx_c.c sysxstop.c

all: cm cp fp hk hp hx lp mem mt ra rk rl rp sys

sources: ${SOURCES}

${SOURCES}:
	sccs get $@

install: all
	cp cmx cmxr cpx fpx hkx hkxr ${DESTDIR}/usr/usep
	-strip ${DESTDIR}/usr/usep/cmx ${DESTDIR}/usr/usep/cmxr
	-strip ${DESTDIR}/usr/usep/cpx ${DESTDIR}/usr/usep/fpx
	-strip ${DESTDIR}/usr/usep/hkx ${DESTDIR}/usr/usep/hkxr
	chmod 700 ${DESTDIR}/usr/usep/cmx ${DESTDIR}/usr/usep/cmxr
	chmod 700 ${DESTDIR}/usr/usep/cpx ${DESTDIR}/usr/usep/fpx
	chmod 700 ${DESTDIR}/usr/usep/hkx ${DESTDIR}/usr/usep/hkxr
	chog usep ${DESTDIR}/usr/usep/cmx ${DESTDIR}/usr/usep/cmxr
	chog usep ${DESTDIR}/usr/usep/cpx ${DESTDIR}/usr/usep/fpx
	chog usep ${DESTDIR}/usr/usep/hkx ${DESTDIR}/usr/usep/hkxr

	cp hpx hpxr hxx hxxr lpx memx memxr ${DESTDIR}/usr/usep
	-strip ${DESTDIR}/usr/usep/hpx ${DESTDIR}/usr/usep/hpxr
	-strip ${DESTDIR}/usr/usep/hxx
	-strip ${DESTDIR}/usr/usep/hxxr ${DESTDIR}/usr/usep/lpx
	-strip ${DESTDIR}/usr/usep/memx ${DESTDIR}/usr/usep/memxr
	chmod 700 ${DESTDIR}/usr/usep/hpx ${DESTDIR}/usr/usep/hpxr
	chmod 700 ${DESTDIR}/usr/usep/hxx
	chmod 700 ${DESTDIR}/usr/usep/hxxr ${DESTDIR}/usr/usep/lpx
	chmod 700 ${DESTDIR}/usr/usep/memx ${DESTDIR}/usr/usep/memxr
	chog usep ${DESTDIR}/usr/usep/hpx ${DESTDIR}/usr/usep/hpxr
	chog usep ${DESTDIR}/usr/usep/hxx
	chog usep ${DESTDIR}/usr/usep/hxxr ${DESTDIR}/usr/usep/lpx
	chog usep ${DESTDIR}/usr/usep/memx ${DESTDIR}/usr/usep/memxr

	cp mtx mtxr rax raxr rkx rkxr ${DESTDIR}/usr/usep
	-strip ${DESTDIR}/usr/usep/mtx ${DESTDIR}/usr/usep/mtxr
	-strip ${DESTDIR}/usr/usep/rax ${DESTDIR}/usr/usep/raxr
	-strip ${DESTDIR}/usr/usep/rkx ${DESTDIR}/usr/usep/rkxr
	chmod 700 ${DESTDIR}/usr/usep/mtx ${DESTDIR}/usr/usep/mtxr
	chmod 700 ${DESTDIR}/usr/usep/rax ${DESTDIR}/usr/usep/raxr
	chmod 700 ${DESTDIR}/usr/usep/rkx ${DESTDIR}/usr/usep/rkxr
	chog usep ${DESTDIR}/usr/usep/mtx ${DESTDIR}/usr/usep/mtxr
	chog usep ${DESTDIR}/usr/usep/rax ${DESTDIR}/usr/usep/raxr
	chog usep ${DESTDIR}/usr/usep/rkx ${DESTDIR}/usr/usep/rkxr

	cp rlx rlxr rpx rpxr ${DESTDIR}/usr/usep
	-strip ${DESTDIR}/usr/usep/rlx ${DESTDIR}/usr/usep/rlxr
	-strip ${DESTDIR}/usr/usep/rpx ${DESTDIR}/usr/usep/rpxr
	chmod 700 ${DESTDIR}/usr/usep/rlx ${DESTDIR}/usr/usep/rlxr
	chmod 700 ${DESTDIR}/usr/usep/rpx ${DESTDIR}/usr/usep/rpxr
	chog usep ${DESTDIR}/usr/usep/rlx ${DESTDIR}/usr/usep/rlxr
	chog usep ${DESTDIR}/usr/usep/rpx ${DESTDIR}/usr/usep/rpxr

	cp sysx sysx_r sysx_c sysxstop ${DESTDIR}/usr/usep
	-strip ${DESTDIR}/usr/usep/sysx ${DESTDIR}/usr/usep/sysx_r
	-strip ${DESTDIR}/usr/usep/sysx_c ${DESTDIR}/usr/usep/sysxstop
	chmod 700 ${DESTDIR}/usr/usep/sysx ${DESTDIR}/usr/usep/sysx_r
	chmod 700 ${DESTDIR}/usr/usep/sysx_c ${DESTDIR}/usr/usep/sysxstop
	chog usep ${DESTDIR}/usr/usep/sysx ${DESTDIR}/usr/usep/sysx_r
	chog usep ${DESTDIR}/usr/usep/sysx_c ${DESTDIR}/usr/usep/sysxstop
	make clean

cm: cmx cmxr

cmx:	cmx1.c
	cc ${CFLAGS} -O cmx1.c -o cmx ${LIBB}

cmxr:	cmx2.c
	cc -n ${CFLAGS} -O cmx2.c -o cmxr ${LIBB}

cp: cpx

cpx:	cpx.c
	cc -f -c ${CFLAGS} -O cpx.c
	cc -f cpx.o -o cpx ${LIBB}

fp: fpx

fpx:	fpx.c
	cc -c ${CFLAGS} -O fpx.c
	cc fpx.o -o fpx ${LIBB}

hk: hkx hkxr

hkx:	hkx1.c
	cc ${CFLAGS} -O hkx1.c -o hkx ${LIBB}

hkxr:	hkx2.c
	cc ${CFLAGS} -O hkx2.c -o hkxr ${LIBB}

hp: hpx hpxr

hpx:	hpx1.c
	cc ${CFLAGS} -O hpx1.c -o hpx ${LIBB}

hpxr:	hpx2.c
	cc ${CFLAGS} -O hpx2.c -o hpxr ${LIBB}

hx: hxx hxxr

hxx:	hxx1.c
	cc ${CFLAGS} -O hxx1.c -o hxx ${LIBB}

hxxr:	hxx2.c
	cc ${CFLAGS} -O hxx2.c -o hxxr ${LIBB}

lp: lpx

lpx:	lpx.c
	cc -c ${CFLAGS} -O lpx.c
	cc lpx.o -o lpx ${LIBB}

mem: memx memxr

memx:	memx1.c
	cc ${CFLAGS} -O memx1.c -o memx ${LIBB}

memxr:	memx2.c
	cc ${CFLAGS} -O memx2.c -o memxr ${LIBB}

mt: mtx mtxr

mtx:	mtx1.c
	cc ${CFLAGS} -O mtx1.c -o mtx ${LIBB}

mtxr:	mtx2.c
	cc -n ${CFLAGS} -O mtx2.c -o mtxr ${LIBB}

ra: rax raxr

rax:	rax1.c
	cc ${CFLAGS} -O rax1.c -o rax ${LIBB}

raxr:	rax2.c
	cc ${CFLAGS} -O rax2.c -o raxr ${LIBB}

rk: rkx rkxr

rkx:	rkx1.c
	cc ${CFLAGS} -O rkx1.c -o rkx ${LIBB}

rkxr:	rkx2.c
	cc ${CFLAGS} -O rkx2.c -o rkxr ${LIBB}

rl: rlx rlxr

rlx:	rlx1.c
	cc ${CFLAGS} -O rlx1.c -o rlx ${LIBB}

rlxr:	rlx2.c
	cc ${CFLAGS} -O rlx2.c -o rlxr ${LIBB}

rp: rpx rpxr

rpx:	rpx1.c
	cc ${CFLAGS} -O rpx1.c -o rpx ${LIBB}

rpxr:	rpx2.c
	cc ${CFLAGS} -O rpx2.c -o rpxr ${LIBB}

sys: sysx sysx_r sysx_c sysxstop

sysx:	sysx_m.c
	cc -c ${CFLAGS} sysx_m.c
	cc sysx_m.o -o sysx ${LIBB}

sysx_r: sysx_r.c
	cc -c ${CFLAGS} -O sysx_r.c
	cc sysx_r.o -o sysx_r ${LIBB}

sysx_c:	sysx_c.c
	cc -c ${CFLAGS} sysx_c.c
	cc sysx_c.o -o sysx_c

sysxstop:	sysxstop.c
	cc -o sysxstop sysxstop.c ${LIBB}

update:	xcm xcp xfp xhk xhp xhx xlp xmem xmt xra xrk xrl xrp xsys

xcm:	${DESTDIR}/usr/usep/cmx ${DESTDIR}/usr/usep/cmxr

${DESTDIR}/usr/usep/cmx:	cmx
	cp cmx ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/cmxr:	cmxr
	cp cmxr ${DESTDIR}/usr/usep

xcp:	${DESTDIR}/usr/usep/cpx

${DESTDIR}/usr/usep/cpx:	cpx
	cp cpx ${DESTDIR}/usr/usep

xfp:	${DESTDIR}/usr/usep/fpx

${DESTDIR}/usr/usep/fpx:	fpx
	cp fpx ${DESTDIR}/usr/usep

xhk:	${DESTDIR}/usr/usep/hkx ${DESTDIR}/usr/usep/hkxr

${DESTDIR}/usr/usep/hkx:	hkx
	cp hkx ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/hkxr:	hkxr
	cp hkxr ${DESTDIR}/usr/usep

xhp:	${DESTDIR}/usr/usep/hpx ${DESTDIR}/usr/usep/hpxr

${DESTDIR}/usr/usep/hpx:	hpx
	cp hpx ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/hpxr:	hpxr
	cp hpxr ${DESTDIR}/usr/usep

xhx:	${DESTDIR}/usr/usep/hxx ${DESTDIR}/usr/usep/hxxr

${DESTDIR}/usr/usep/hxx:	hxx
	cp hxx ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/hxxr:	hxxr
	cp hxxr ${DESTDIR}/usr/usep

xlp:	${DESTDIR}/usr/usep/lpx

${DESTDIR}/usr/usep/lpx:	lpx
	cp lpx ${DESTDIR}/usr/usep

xmem:	${DESTDIR}/usr/usep/memx ${DESTDIR}/usr/usep/memxr

${DESTDIR}/usr/usep/memx:	memx
	cp memx ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/memxr:	memxr
	cp memxr ${DESTDIR}/usr/usep

xmt:	${DESTDIR}/usr/usep/mtx ${DESTDIR}/usr/usep/mtxr

${DESTDIR}/usr/usep/mtx:	mtx
	cp mtx ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/mtxr:	mtxr
	cp mtxr ${DESTDIR}/usr/usep

xra:	${DESTDIR}/usr/usep/rax ${DESTDIR}/usr/usep/raxr

${DESTDIR}/usr/usep/rax:	rax
	cp rax ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/raxr:	raxr
	cp raxr ${DESTDIR}/usr/usep

xrk:	${DESTDIR}/usr/usep/rkx ${DESTDIR}/usr/usep/rkxr

${DESTDIR}/usr/usep/rkx:	rkx
	cp rkx ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/rkxr:	rkxr
	cp rkxr ${DESTDIR}/usr/usep

xrl:	${DESTDIR}/usr/usep/rlx ${DESTDIR}/usr/usep/rlxr

${DESTDIR}/usr/usep/rlx:	rlx
	cp rlx ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/rlxr:	rlxr
	cp rlxr ${DESTDIR}/usr/usep

xrp:	${DESTDIR}/usr/usep/rpx ${DESTDIR}/usr/usep/rpxr

${DESTDIR}/usr/usep/rpx:	rpx
	cp rpx ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/rpxr:	rpxr
	cp rpxr ${DESTDIR}/usr/usep

xsys:	${DESTDIR}/usr/usep/sysx ${DESTDIR}/usr/usep/sysx_r \
	${DESTDIR}/usr/usep/sysx_c ${DESTDIR}/usr/usep/sysxstop

${DESTDIR}/usr/usep/sysx:	sysx
	cp sysx ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/sysx_r:	sysx_r
	cp sysx_r ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/sysx_c:	sysx_c
	cp sysx_c ${DESTDIR}/usr/usep

${DESTDIR}/usr/usep/sysxstop:	sysxstop
	cp sysxstop ${DESTDIR}/usr/usep

clean:
	-rm -f cmx cmxr cpx fpx hkx hkxr hpx hpxr hxx hxxr lpx memx memxr \
		mtx mtxr rax raxr rkx rkxr rlx rlxr rpx rpxr sysx sysx_r \
		sysx_c sysxstop 
	-rm -f log a.out *.o

clobber: clean
	sccs clean

tags:	${SOURCES}
	ctags ${SOURCES}

sccsinfo:
	sccs info
