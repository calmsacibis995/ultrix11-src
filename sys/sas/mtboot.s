/ SCCSID: @(#)mtboot.s	3.0	4/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/  File name:
/
/	mtboot.s
/
/  Source file description:
/
/	ULTRIX-11 800 BPI magtape primary boot program.
/
/	BOOTS: TM02/3, TM11
/
/	A copy of this program resides in each of the first two records
/	on a bootable magtape, some hardware bootstraps read the first
/	record, others read the second. This program reads the third tape
/	record into location zero and transfers control to location zero.
/
/	On the ULTRIX-11 distribution tapes the third record is the secondary
/	boot program (/sas/boot). The secondary boot is loaded by reading
/	its first block and using the text and data sizes in the a.out header
/	to determine how many more blocks to read.
/
/	After the boot is loaded, this program strips off the a.out header
/	by shifting all of memory below its stack down 16 bytes. This program
/	leaves a boot device code in R1 which tells the secondary boot what
/	type of tape was booted, i.e., TM02/3 (ht) or TM11 (tm).
/
/	This program determines the boot device type by testing the HTCSR
/	(172440). If it exists (no trap thru 4), the secondary boot is
/	loaded from the TM02/3, otherwise it is loaded from the TM11.
/
/			****** RESTRICTIONS ******
/
/  1.	This program only boots from magtape unit zero.
/
/  2.	Secondary boot program (/sas/boot) must be written on the tape
/	using 512 byte records. If the last record is a partial record,
/	it must be zero filled. This is the only way to insure that the
/	beginning of BSS space is initialized to zero. See the maketape
/	program /usr/sys/sas/maketape.c
/
/  3.	The secondary boot program must begin with the a.out header.
/
/  Functions:
/
/	htread:	TM02/3 block read routine. Reads a number of records
/		starting at the requested record number into memory at
/		the specified address.
/
/	hrrec:	TM02/3 driver. Reads one 512 byte record inot a specified
/		memory address. Also, attempts error recovery via retry.
/
/	htrew:	TM02/3 tape rewind routine.
/
/	tmread:	TM11 block read routine. Reads a number
/		of records starting a the requested record number into
/		memory at the specified address.
/
/	tmrrec:	TM11 driver. Reads one 512 byte record into the specified
/		address. Also, attempts error recovery via retry.
/
/	tmrew:	TM11 tape rewind routine.
/
/  Usage:
/
/	Execute the machine's hardware bootstrap for magtape or the
/	appropriate toggle-in program from appendix A of the Inst. guide.
/
/  Assemble:
/
/	cd /usr/sys/sas; make mtboot; cp mtboot /sas
/	(Don't just assemble it, that will not work!)
/
/  Modification history:
/
/	25 February 1985 -- Fred Canter
/		Added the module header.
/		Fixed yet another signed number bug with loading a boot
/		who's text + data size > 32768. Convert size to number of
/		blocks to read instead of negative word count.
/
/	?? ??? ????? -- Fred Canter
/		This module was part of the original V7 system, but has
/		been modified many times over the years.
/

core = 28.
halt=0
nop=240
.. = [core*2048.]-512.
start:
	nop			/ DEC boot block standard
	br	1f		/ "   "    "     "
1:
	mov	$..,sp
	clr	r0
	mov	sp,r1
	cmp	pc,r1
	bhis	2f
	cmp	(r0),$407
	bne	1f
	mov	$20,r0
1:
	mov	(r0)+,(r1)+
	cmp	r1,$core*2048.
	blo	1b
	jmp	(sp)

2:
	clr	(r0)+		/ clear core (bss in boot) !!!
	cmp	r0,sp
	blo	2b
	mov	$11.,r5		/ set boot device code (assume TM11)
	mov	$1f,*$4
	mov	$340,*$6
	tst	*$htcs1
	mov	$RESET,*$htcs2
	mov	$P800,*$httc
	bit	$MOL,*$htds
	beq	1f
	mov	$9.,r5		/ change boot device code to TM02/3
	mov	$htrew,rew
	mov	$htread,tread
	br	2f
1:
	mov	$tmread,tread
	mov	$tmrew,rew
2:
	jsr	pc,*rew
	mov	$2,tapa
	mov	$1,bc
	jsr	pc,*tread	/ Read first block of boot
				/ Find out how big boot is
	mov	*$2,r0		/ text segment size
	add	*$4,r0		/ data segment size
/	sub	$512.,r0	/ They forgot to skip the a.out header!
	sub	$496.,r0
	add	$511.,r0	/ Convert size to block count
	clc			/ UNSIGNED!
	ror	r0
	ash	$-8.,r0
	beq	1f		/ In case boot size < 496 bytes (FAT CHANCE!)
	mov	r0,bc
	mov	$3,tapa
	mov	$512.,ba
	jsr	pc,*tread
1:
	jsr	pc,*rew
	clr	r0
	mov	$20,r1
	mov	sp,r4
	clc
	ror	r4
1:
	mov	(r1)+,(r0)+
	sob	r4,1b
	clr	r0	/ say booted from unit 0
	mov	r5,r1	/ tell boot program we came from HT or TM
	clr	r4	/ make sure boot doesn't try to use CSR
	clr	pc

htcs1 = 172440
htba  = 172444
htfc  = 172446
htcs2 = 172450
htds  = 172452
httc  = 172472

P800 = 1300
P1600 = 2300
PIP = 20000
RESET = 40
MOL = 10000
ERR = 40000
REV = 33
READ = 71
REW = 7

htread:
1:
	mov	ba,mtma
	cmp	mtapa,tapa
	beq	1f
	bhi	2f
	jsr	pc,hrrec
	br	1b
2:
	jsr	pc,htrew
	br	1b
1:
	mov	bc,r1
1:
	jsr	pc,hrrec
	sob	r1,1b
	rts	pc

hrrec:
	mov	$htds,r0
	tstb	(r0)
	bpl	hrrec
	bit	$PIP,(r0)
	bne	hrrec
	bit	$MOL,(r0)
	beq	hrrec
	mov	$htfc,r0
	mov	$-512.,(r0)
	mov	mtma,-(r0)
	mov	$-256.,-(r0)
	mov	$READ,-(r0)
1:
	tstb	(r0)
	bpl	1b
	bit	$ERR,(r0)
	bpl	1f
	mov	$RESET,*$htcs2
	mov	$-1,*$htfc
	mov	$REV,(r0)
	br	hrrec
1:
	add	$512.,mtma
	inc	mtapa
	rts	pc

htrew:
	mov	$RESET,*$htcs2
	mov	$P800,*$httc
	mov	$REW,*$htcs1
	clr	mtapa
	rts	pc


mts = 172520
mtc = 172522
mtbrc = 172524
mtcma = 172526

tmread:
1:
	mov	ba,mtma
	cmp	mtapa,tapa
	beq	1f
	bhi	2f
	jsr	pc,tmrrec
	br	1b
2:
	jsr	pc,tmrew
	br	1b
1:
	mov	bc,r1
1:
	jsr	pc,tmrrec
	sob	r1,1b
	rts	pc

tmrrec:
	mov	$mts,r0
	bit	$2,(r0)+		/ rewind status
	bne	tmrrec
	tstb	(r0)+		/ cu ready
	bpl	tmrrec
	inc 	r0
	mov	$-512.,(r0)+	/ byte count
	mov	mtma,(r0)	/ bus address
	mov	$mtc,r0
	mov	$60003,(r0)		/ read 800bpi
1:
	tstb	(r0)
	bpl	1b
	tst	(r0)+
	bpl	1f
	mov	$-1,(r0)
	mov	$60013,-(r0)		/ backspace
	br	tmrrec
1:
	add	$512.,mtma
	inc	mtapa
	rts	pc

tmrew:
	mov	$60017,*$mtc
	clr	mtapa
	rts	pc

mtapa:	0
mtma:	0
tapa:	0
bc:	0
ba:	0
rew:	0
tread:	0
