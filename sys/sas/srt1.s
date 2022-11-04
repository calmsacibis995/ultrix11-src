/ SCCSID: @(#)srt1.s	3.0	4/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Startup code for standalone programs
/ to be loaded and run via `Boot:'.
/
/ Initializes the system call segment of a standalone program.
/ Runs when the first system call is executed.
/ See /usr/sys/sas/README for more information.
/
/ Fred Canter 12/14/83

/ non-UNIX instructions
rtt	= 6
reset	= 5
halt	= 0
mtpi	= 6600^tst
mfpi	= 6500^tst

PS	= 177776

/ The emtcode tell which system call to execute.

emtcode	= 140002

.globl	_end
.globl	__rtt
.globl	_edata
.globl	_devsw
.globl	_lseek, _getc, _getw, _read, _write, _open, _close

/ trap vectors
/ location 0 has address of devsw[] table,
/ address of trap handler loaded later.

	_devsw;357	/ location 0 stray vector
	trap;340	/ bus error
	trap;341	/ illegal instruction
	trap;342	/ BPT
	trap;343	/ IOT
	trap;344	/ POWER FAIL
	sysent;345	/ EMT (system call)
	trap;346	/ TRAP

.=114^.
	trap;352	/ Memory parity

.=240^.
	trap;347	/ Programmed interrupt request
	trap;350	/ Floating point
	trap;351	/ M/M segmentation

.=1000^.

.globl	_segflag

		/ ****** MUST BE AT 1000 ******
_segflag: 0	/ Controls extended addressing for device drivers
		/ Set by syscall interface code @ 140000 (see sci.s)
tpc:	0	/ Save PS & PC from trap, used to start
tps:	0	/ program, and use for return to boot.
saveps: 0	/ save PS after trap (PS is trap type)
first:	0	/ first time thru flag (set up trap catchers)

.text

sysent:
	mov	(sp)+,tpc
	mov	(sp)+,tps
	tst	first
	bne	noset

/ Load stray vector catchers in unused vector locations
/ on first system call only.

	inc	first
	clr	r0
4:
	mov	$trap,(r0)+
	mov	$357,(r0)+
	br	2f
1:
	tst	(r0)+
	tst	(r0)+
2:
	cmp	r0,$1000
	bge	3f
	tst	2(r0)
	bne	1b
	br	4b
3:

/ Clear bss area of program.

	mov	$_edata,r0
	mov	$_end,r1
	sub	r0,r1
	inc	r1
	clc
	ror	r1
1:
	clr	(r0)+
	sob	r1,1b
/ Use the emtcode to dispatch to a system call.
noset:
	cmp	*$emtcode,$104000
	bne	1f
	jsr	pc,*$_lseek
	br	__rtt
1:
	cmp	*$emtcode,$104001
	bne	1f
	jsr	pc,*$_getc
	br	__rtt
1:
	cmp	*$emtcode,$104002
	bne	1f
	jsr	pc,*$_getw
	br	__rtt
1:
	cmp	*$emtcode,$104003
	bne	1f
	jsr	pc,*$_read
	br	__rtt
1:
	cmp	*$emtcode,$104004
	bne	1f
	jsr	pc,*$_write
	br	__rtt
1:
	cmp	*$emtcode,$104005
	bne	1f
	jsr	pc,*$_open
	br	__rtt
1:
	cmp	*$emtcode,$104006
	bne	1f
	jsr	pc,*$_close
	br	__rtt
1:
	halt
	br	1b

/ fix up stack to point at trap ps-pc pair
/ so we can return to the bootstrap
__rtt:
	mov	tps,-(sp)
	mov	tpc,-(sp)
	rtt				/ we hope!
	br	.


.globl	_trap
trap:
	mov	*$PS,saveps
	mov	r0,-(sp)
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	r5,-(sp)
	mov	sp,-(sp)
	sub	$4,(sp)
	mov	saveps,-(sp)
	jsr	pc,_trap
	halt		/ _trap never returns
	br	.

/ mtpi(word,addr)
.globl	_mtpi
_mtpi:
	mov	4(sp),r0
	mov	2(sp),-(sp)
	mtpi	(r0)+
	rts	pc

/ mfpi(addr), word returned in r0
.globl _mfpi
_mfpi:
	mov	2(sp),r1
	mfpi	(r1)
	mov	(sp)+,r0
	rts	pc

