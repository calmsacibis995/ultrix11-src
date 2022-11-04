# SCCSID: @(#)makefile	3.0	4/21/86
#
######################################################################
#   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    #
#   All Rights Reserved. 					     #
#   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      #
######################################################################
#
#	ULTRIX-11 /usr/sys/sas/makefile V3.0
#	Fred Canter (with some help from my friends)

OPTS = -DK450 -DBIGKERNEL
CFLAGS = -O ${OPTS}

all: tdmsg libsa.a srt1.o syscall srt0.o sizchk smagic restor scat copy icheck mkfs dskinit bads rabads htboot mtboot tkboot maketape boot sdload

clean:
	rm -f *.o *.nm
	rm -f bads copy dskinit icheck maketape mkfs 
	rm -f rabads restor scat sdload sizchk smagic syscall
	rm -f htboot mtboot tkboot boot boot8 boot16 boot32

cp cmp:
	@echo "standalone stuff-- examine this makefile before running"

xcp:
	cp syscall sdload restor scat copy mkfs icheck dskinit bads rabads /sas
	rm syscall sdload restor scat copy mkfs icheck dskinit bads rabads
	cp mtboot /sas
	cp htboot /sas
	cp tkboot /sas
	@echo "TEMP: copy tape boots to /mdec (until src moved to mdec)"
	cp mtboot /mdec
	cp htboot /mdec
	cp tkboot /mdec
	chmod 644 /mdec/??boot
	rm htboot mtboot tkboot
	cp maketape /sas
	rm maketape
# 10/11/83 - no need for two copies of these files (see /sas)
#	cp td800.sv /sas
#	cp td1600.sv /sas
#	cp td800.fs /sas
#	cp td1600.fs /sas
#	cp td800.bv /sas
#	cp td1600.bv /sas
#	cp contents.sv /sas
#	cp contents.fs /sas
#	cp contents.bv /sas
	cp boot /sas
	cp boot /
	rm boot
	chmod 644 /sas/*
	chmod 744 /sas/maketape

libsa.a: SYS.o conf.o ra.o tk.o hp.o ht.o prf.o rp.o tm.o hk.o ts.o rl.o rk.o md.o nlist.o dkbad.o
	ar rv /usr/lib/libsa.a $?
	ranlib /usr/lib/libsa.a
	rm SYS.o conf.o

sizchk:	sizchk.o
	cc -o sizchk sizchk.o

smagic:	smagic.o
	cc -o smagic smagic.o

syscall: tdmsg sizchk smagic srt1.o
	ld -o syscall srt1.o -lsa -lc
	nm -ng syscall >syscall.nm
	strip syscall
	size syscall
# Fred Canter -- 8/20/85
#	HP & HK drivers need KISA5 for mapping to users' data buffer,
#	see fixecc() & fixhdr() in dkbad.c
	sizchk syscall 40960
	smagic syscall

restor:	/usr/src/cmd/restor/restor.c sizchk smagic srt0.o sci.o prf.o
	cc -c -O -DSTANDALONE -DRESTOR40 /usr/src/cmd/restor/restor.c
	cc -c -O -DNOSYSCALL SYS.c
	ld -o restor srt0.o restor.o sci.o SYS.o prf.o -lc
	nm -ng restor >restor.nm
	strip restor
	size restor
	sizchk restor 49152
	smagic restor
	rm restor.o SYS.o

/usr/src/cmd/restor/restor.c:
	(cd /usr/src/cmd/restor; sccs get restor.c)

scat:	sizchk smagic srt0.o sci.o prf.o
	cc -c scat.c
	cc -c -O -DNOSYSCALL SYS.c
	ld -o scat srt0.o scat.o sci.o SYS.o prf.o -lc
	nm -ng scat >scat.nm
	strip scat
	size scat
	sizchk scat 49152
	smagic scat
	rm scat.o SYS.o

copy:	sizchk smagic srt0.o sci.o prf.o
	cc -c copy.c
	cc -c -O -DNOSYSCALL SYS.c
	ld -o copy srt0.o copy.o sci.o SYS.o prf.o -lc
	nm -ng copy >copy.nm
	strip copy
	size copy
	sizchk copy 49152
	smagic copy
	rm copy.o SYS.o

mkfs:	/usr/src/cmd/mkfs.c sizchk smagic srt0.o sci.o prf.o
	cc -c -O -DSTANDALONE /usr/src/cmd/mkfs.c
	cc -c -O -DNOSYSCALL SYS.c
	ld -o mkfs srt0.o mkfs.o sci.o SYS.o prf.o -lc
	nm -ng mkfs >mkfs.nm
	strip mkfs
	size mkfs
	sizchk mkfs 49152
	smagic mkfs
	rm mkfs.o SYS.o

/usr/src/cmd/mkfs.c:
	(cd /usr/src/cmd; sccs get mkfs.c)

icheck:	/usr/src/cmd/icheck/icheck.c sizchk smagic srt0.o sci.o prf.o
	cc -c -O -DSTANDALONE /usr/src/cmd/icheck/icheck.c
	cc -c -O -DNOSYSCALL SYS.c
	ld -o icheck srt0.o icheck.o sci.o SYS.o prf.o -lc
	nm -ng icheck >icheck.nm
	strip icheck
	size icheck
	sizchk icheck 49152
	smagic icheck
	rm icheck.o SYS.o

/usr/src/cmd/icheck/icheck.c:
	(cd /usr/src/cmd/icheck; sccs get icheck.c)

dskinit: sizchk smagic srt0.o sci.o prf.o
	cc -c -O dskinit.c
	cc -c -O -DNOSYSCALL SYS.c
	ld -o dskinit srt0.o dskinit.o sci.o SYS.o prf.o -lc
	nm -ng dskinit >dskinit.nm
	strip dskinit
	size dskinit
	sizchk dskinit 49152
	smagic dskinit
	rm dskinit.o SYS.o

bads: 	sizchk smagic srt0.o sci.o prf.o
	cc -c -O bads.c
	cc -c -O -DNOSYSCALL SYS.c
	ld -o bads srt0.o bads.o sci.o SYS.o prf.o -lc
	nm -ng bads >bads.nm
	strip bads
	size bads
	sizchk bads 49152
	smagic bads
	rm bads.o SYS.o

rabads:	sizchk smagic srt2.o rabads.o
	cc -c -O -DK450 -DBIGKERNEL -DNO_FIO ra.c
	cc -c -O -DK450 -DBIGKERNEL -DNO_FIO prf.c
	cc -c -O -DNO_FIO SYS.c
	cc -c -O -DRABADS -DNO_FIO conf.c
	ld -o rabads srt2.o rabads.o SYS.o conf.o ra.o prf.o -lc
	nm -ng rabads >rabads.nm
	strip rabads
	size rabads
# Stack is at end + 512 (sizchk uses 1024 for early warning)
	sizchk rabads 56320
	smagic rabads
	rm rabads.o SYS.o conf.o ra.o prf.o

htboot:	htboot.s
	as htboot.s
	strip a.out
	dd if=a.out of=htboot bs=16 skip=1
	chmod 644 htboot
	rm -f a.out

mtboot:	mtboot.s
	as mtboot.s
	strip a.out
	dd if=a.out of=mtboot bs=16 skip=1
	chmod 644 mtboot
	rm -f a.out

tkboot:	tkboot.s
	as tkboot.s
	strip a.out
	dd if=a.out of=tkboot bs=16 skip=1
	chmod 644 tkboot
	rm -f a.out

maketape:	maketape.c
	cc -o maketape maketape.c

bmsg:
	@echo "Many versions of boot !"
	rm -f boot.o boot8.o boot16.o boot32.o

tdmsg:
	@echo "SYS.c & /sas tape directories must match !"

boot8:	bmsg sizchk smagic boot8.o
	as - -o M.o M_boot.s M.s
	ld -o boot8 M.o boot8.o -lsa -lc
	nm -ng boot8 >boot8.nm
	strip boot8
# Make sure room for stack at _end_512 and don't overlap block
# zero bootstrap (??uboot), see /usr/sys/sas/README.
	sizchk boot8 55552
	smagic boot8

boot16:	bmsg sizchk smagic boot16.o
	as - -o M.o M_boot.s M.s
	ld -o boot16 M.o boot16.o -lsa -lc
	nm -ng boot16 >boot16.nm
	strip boot16
# Make sure room for stack at _end_512 and don't overlap block
# zero bootstrap (??uboot), see /usr/sys/sas/README.
	sizchk boot16 55552
	smagic boot16

boot32: bmsg sizchk smagic boot32.o
	as - -o M.o M_boot.s M.s
	ld -o boot32 M.o boot32.o -lsa -lc
	nm -ng boot32 >boot32.nm
	strip boot32
# Make sure room for stack at _end_512 and don't overlap block
# zero bootstrap (??uboot), see /usr/sys/sas/README.
	sizchk boot32 55552
	smagic boot32

boot:	bmsg sizchk smagic boot.o
	as - -o M.o M_boot.s M.s
	ld -o boot M.o boot.o -lsa -lc
	nm -ng boot >boot.nm
	strip boot
# Make sure room for stack at _end_512 and don't overlap block
# zero bootstrap (??uboot), see /usr/sys/sas/README.
	sizchk boot 55552
	smagic boot

sdload:	mtsdload
	mv mtload sdload

mtsdload: sizchk smagic
	rm -f M.o sdload.o
	cc -c ${CFLAGS} -DMTLOAD sdload.c
	as - -o M.o M_sdload.s M.s
	ld -o mtload M.o sdload.o -lsa -lc
	nm -ng mtload >mtload.nm
	strip mtload
# Stack is at end + 512 (sizchk uses 1024 for early warning)
	sizchk mtload 56320
	smagic mtload
	rm -f mtload.o M.o

rlsdload: sizchk smagic
	rm -f M.o sdload.o
	cc -c ${CFLAGS} -DRLLOAD sdload.c
	as - -o M.o M_sdload.s M.s
	ld -o rlload M.o sdload.o -lsa -lc
	nm -ng rlload >rlload.nm
	strip rlload
# Stack is at end + 512 (sizchk uses 1024 for early warning)
	sizchk rlload 56320
	smagic rlload
	rm -f rlload.o M.o

rcsdload: sizchk smagic
	rm -f M.o sdload.o
	cc -c ${CFLAGS} -DRCLOAD sdload.c
	as - -o M.o M_sdload.s M.s
	ld -o rcload M.o sdload.o -lsa -lc
	nm -ng rcload >rcload.nm
	strip rcload
# Stack is at end + 512 (sizchk uses 1024 for early warning)
	sizchk rcload 56320
	smagic rcload
	rm -f rcload.o M.o

rxsdload: sizchk smagic
	rm -f M.o sdload.o
	cc -c ${CFLAGS} -DRXLOAD sdload.c
	as - -o M.o M_sdload.s M.s
	ld -o rxload M.o sdload.o -lsa -lc
	nm -ng rxload >rxload.nm
	strip rxload
# Stack is at end + 512 (sizchk uses 1024 for early warning)
	sizchk rxload 56320
	smagic rxload
	rm -f rxload.o M.o

boot.o:
	cc -c ${CFLAGS} boot.c

rabads.o:
	cc -c -O -DNO_FIO rabads.c

boot8.o:
	cc -c ${CFLAGS} -DUMAX=023041 boot.c
	mv boot.o boot8.o

boot16.o:
	cc -c ${CFLAGS} -DUMAX=035056 boot.c
	mv boot.o boot16.o

boot32.o:
	cc -c ${CFLAGS} -DUMAX=022045 boot.c
	mv boot.o boot32.o
