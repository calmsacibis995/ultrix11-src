/ SCCSID: @(#)htboot.s	3.0	4/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/  File name:
/
/	htboot.s
/
/  Source file description:
/
/	ULTRIX-11 1600 BPI magtape primary boot program.
/
/	BOOTS: TM02/3, TS11, TU80, TSV05, TK25
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
/	type of tape was booted, i.e., TM02/3 (ht) or TS11 (ts).
/
/	This program uses the CSR address in R1 to determine which type of
/	magtape controller was booted. If the CSR is in the range of
/	772520 -> 772554, the TS11 is booted, otherwise it was a TM02/3.
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
/  4.	The hardware boot must leave the magtape CSR address in R1, must
/	DEC bootstraps do this. If the toggle-in boot routine is used,
/	the CSR must be deposited in R1, see Appendix A of the Inst. Guide.
/
/  5.	Cannot really boot from a TM02 at 1600 BPI because the hardware
/	bootstrap sets the density to 800 BPI. Works with the TM03 because
/	the TM03 auto-density selects on read and ignores the density
/	set by the hardware bootstrap.
/
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
/	tsread:	TS11 (TU80, TSV05, TK25) block read routine. Reads a number
/		of records starting a the requested record number into
/		memory at the specified address.
/
/	tsrrec:	TS11 driver. Reads one 512 byte record into the specified
/		address. Also, attempts error recovery via retry.
/
/	tsrew:	TS11 tape rewind routine.
/
/	tsinit:	TS11 initialize routine. Does set controller characteristics
/		and all that other TS11 protocol good stuff.
/
/  Usage:
/
/	Execute the machine's hardware bootstrap for magtape or the
/	appropriate toggle-in program from appendix A of the Inst. guide.
/
/  Assemble:
/
/	cd /usr/sys/sas; make htboot; cp htboot /sas
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
	mov	r1,r5
	mov	sp,r1
	cmp	pc,r1
	bhis	2f
	mov	r5,r2
	bic	$3,r2
	mov	r2,r3
	tst	(r3)+
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
	mov	$9.,r4		/ set bdcode for boot (assume TM02/3)
/ If the CSR address in r1 is in the range
/ of 0172520 - 0172554, then the magtape is a TS11,
/ otherwise a TM02/3 magtape is assumed.
/ This is done in order to (hopefully) allow
/ the TM02/3 magtapes to be booted by any
/ hardware boot, even if it does not leave the
/ CSR address in r1.
/ The M9312, which is the only boot for the TS11,
/ leaves the CSR address in r1.

	cmp	r2,$172520
	blo	1f
	cmp	r2,$172554
	bhi	1f
	inc	r4		/ change bdcode to TS11
	mov	$tsread,tread
	mov	$tsrew,rew
	br	2f
1:
	mov	$htrew,rew
	mov	$htread,tread
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
	mov	sp,r2
	clc
	ror	r2
1:
	mov	(r1)+,(r0)+
	sob	r2,1b
	clr	r0	/ say booted from unit 0
	mov	r4,r1	/ tell boot program we came from tape (ht or ts)
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
	mov	$P1600,*$httc
	mov	$REW,*$htcs1
	clr	mtapa
	rts	pc


/(r2) is tsbuf
/(r3) is tssr

TSINIT = 140013
TSCHAR = 140004
TSREW  = 102010
TSREAD = 100001
TSRETRY = 100401

tsread:
1:
	mov	ba,mtma
	cmp	mtapa,tapa
	beq	1f
	bhi	2f
	jsr	pc,tsrrec
	br	1b
2:
	jsr	pc,tsrew
	br	1b
1:
	mov	bc,r1
1:
	jsr	pc,tsrrec
	sob	r1,1b
	rts	pc

tsrrec:
1:
	tstb	(r3)
	bpl	1b
	mov	$136006,r0
	mov	$512.,(r0)
	clr	-(r0)
	mov	mtma,-(r0)
	mov	$TSREAD,-(r0)
	mov	r0,(r2)
1:
	tstb	(r3)
	bpl	1b
	cmp	$1,(r3)
	blos	1f
	mov	$TSRETRY,(r0)
	mov	r0,(r2)
	br	1b
1:
	add	$512.,mtma
	inc	mtapa
	rts	pc

tsrew:
	jsr	pc,tsinit
	mov	$TSREW,136000
	mov	$136000,(r2)
	clr	mtapa
	rts	pc

tsinit:
	tstb	(r3)
	bpl	tsinit
	mov	$136000,r0
	mov	$TSCHAR,(r0)+
	mov	$136010,(r0)+
	clr	(r0)+
	mov	$10,(r0)+
	mov	r0,(r0)+
	clr	(r0)+
	mov	$16,(r0)+
	mov	$136000,(r2)
1:
	tstb	(r3)
	bpl	1b
	rts	pc
mtapa:	0
mtma:	0
tapa:	0
bc:	0
ba:	0
rew:	0
tread:	0
