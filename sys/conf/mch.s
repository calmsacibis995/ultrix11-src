
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)mch.s	3.0	4/21/86
 * machine language assist
 * for all pdp11 CPU's
 *
 * The mch.h header file may contain a #define FPP if
 * you have floating point hardware, otherwise the code
 * to support floating point hardware will be excluded.
 *
 * Many thanks to Bill Shannon for his overlay kernel code !!
 *
 * All unix kernels must now be booted via the
 * two - stage bootstrap.
 *
 * Fred Canter 1/15/83
 *
 * This file now has to be run through '/lib/cpp -P' before assembling it!
 *	Dave Borman 5/29/85
 */

#include "mch.h"
#include <sys/localopts.h>

#define	COPY1	/* Do _copy() and _copyv() at spl1, instead of spl7 ! */

#ifndef	COPY7
# ifndef COPY1
# define	COPY7
# endif  COPY1
#endif  COPY7

#ifdef	SEP_ID
	spl	= 230
#	define	SPL0	spl 0
#	define	SPL1	spl 1
#	define	SPL4	spl 4
#	define	SPL5	spl 5
#	define	SPL6	spl 6
#	define	SPLHIGH	spl HIGH
#else	SEP_ID
#	define	SPL0	bic	$HIPRI,PS
#	define	SPL1	bis	$HIPRI,PS; bic	$300,PS
#	define	SPL4	bis	$HIPRI,PS; bic	$140,PS
#	define	SPL5	bis	$HIPRI,PS; bic	$100,PS
#	define	SPL6	bis	$HIPRI,PS; bic	$40,PS
#	define	SPLHIGH	bis	$HIPRI,PS
#endif	SEP_ID
/ non-UNIX instructions
mfpi	= 6500^tst
mtpi	= 6600^tst
mfpd	= 106500^tst
mtpd	= 106600^tst
mfps	= 106700^tst
stst	= 170300^tst
wait	= 1
rtt	= 6
reset	= 5
halt	= 0

#ifndef PROFILE
HIPRI	= 340
HIGH	= 7
#else	PROFILE
HIGH	= 6
HIPRI	= 300
#endif	PROFILE

/*
 * Temporary stack while KDSA6 is repointed.
 *
 * This is done here and is put into data space, so that it will sit as
 * low in memory as possible. Then, if we overflow this temp stack, we'll
 * scribble over a minimum amount of data.  -Dave Borman 6/5/85
 */
.data
.even
intstk: .=.+500		/ temporary stack while KDSA6 is repointed
eintstk: .=.+2		/ initial top of instk

.text
.globl	start, _end, _edata, _etext, _main

/ startup code for separate I & D space CPU's
/ entry is via a trap thru 34
/ memory management must be enabled

start:
	bit	$1,SSR0
	beq	.
	mov	$trap,34
	clr	PS
#ifdef	SEP_ID

/ set KD6 to first available core
/ If profiling, snag supervisor registers.

	mov	$_etext-8192.+63.,r2
	ash	$-6,r2
	bic	$!1777,r2
	add	KISA1,r2
#	ifdef	PROFILE
/ This is now done in machdep
/	mov	r2,SISA2
/	mov	r2,_proloc
/	mov	$77406,SISD2
/	add	$200,r2
/	mov	r2,SISA2+2
/	mov	$77406,SISD2+2
/	add	$200,r2
#	endif	PROFILE
#endif	SEP_ID
#ifdef	K_OV
	mov	ovend,KDSA6		/ ksr6 = sysu
#else	K_OV
	mov	r2,KDSA6
#endif	K_OV

#ifdef	SEP_ID
/ Turn off write permission on kernel text

	mov	$KISD0,r0
1:
	mov	$77402,(r0)+
	cmp	r0,$KISD7
	blos	1b

/ Take stuff above data out of address space
	mov	$_end+63.,r0
	ash	$-6,r0
	bic	$!1777,r0
	mov	$KDSD0,r1
1:
	cmp	r0,$200
	bge	2f
	dec	r0
	bge	4f
	clr	(r1)
	br	3f
4:
	movb	r0,1(r1)
	br	3f
2:
	movb	$177,1(r1)
3:
	tst	(r1)+
	sub	$200,r0
	cmp	r1,$KDSD5
	blos	1b
#endif	SEP_ID

	mov	$usize-1\<8|6,*$KDSD6

#ifdef	SEP_ID
/ set up supervisor D registers

9:
	mov	$6,SISD0
	mov	$6,SISD1
#endif	SEP_ID

/ set up real sp
/ clear user block
/ test for floating point hardware

.globl	_stksize
	mov	$_u,sp
	add	_stksize,sp
#ifdef	FPP
	mov	$1f,nofault
	setd			/ jump to 1f if this traps
	inc	fpp
1:
#endif	FPP
	clr	nofault		/ no unibus map
	mov	$_u,r0
1:
	clr	(r0)+
	cmp	r0,$_u+[usize*64.]
	blo	1b
#if	defined(SEP_ID) && defined(PROFILE)
/ This is now done in machdep.c
/	mov	$40000,r0
/	mov	$10000,PS	/ prev = super
/1:
/	clr	-(sp)
/	mtpi	(r0)+
/	cmp	r0,$100000
/	blo	1b
/	jsr	pc,_isprof
#endif	defined(SEP_ID) && defined(PROFILE)
#if	!defined(NONSEPERATE) && !defined(SEP_ID)
	tst	_sepid		/ Are we a split I/D processor?
	beq	1f
	bis	$1,*_mmr3	/ yes, enable user level I/D
	br	2f
1:
	mov	$UISD0,cbase	/ no, fix mapping table for copyio().
2:
#endif	!defined(NONSEPERATE) && !defined(SEP_ID)

/ set up previous mode and call main
/ on return, enter user mode at 0R

	mov	$30000,PS
	jsr	pc,_main
	mov	$170000,-(sp)
	clr	-(sp)
	rtt

.globl	trap, call
.globl	_trap

/ all traps and interrupts are
/ vectored thru this routine.

.data
/ The 2 spaces at the end are needed !
rzmsg: <\n\rRED ZONE  \0>
.text

trap:
	mov	PS,saveps
	tst	sp		/ is stack pointer equal to zero ?
	bne	7f		/ no, handle trap normally
	mov	$rzmsg,r0	/ yes, red zone stack violation is FATAL
	br	9f		/ branch to code to save cpu error
				/ register. Return is by check for sp
				/ still equal to zero.
2:
	tstb	(r0)
	bne	3f
4:
	halt
	br	4b
3:
	movb	(r0)+,*$177566
5:
	tstb	*$177564
	bpl	5b
	br	2b
7:
#ifdef	SEP_ID
	tst	_cdreg		/ do following on 11/45 or 11/70 only !
	beq	1f		/ 11/44 has no PIRQ or display register
	clrb	PIRQ+1		/ clear the programmed interrupt request
				/ register, or else the system will hang
				/ in a for ever loop of PIRQ's if the trap
				/ was a PIRQ (trap type 7).
				/ UNIX does not use the PIRQ register.
1:
#endif	SEP_ID
	tst	nofault
	bne	1f
9:				/ entry used by red zone code 
				/ to save cpu error reg
	tst	_cpereg		/ do we have a CPU error reg ?
	bmi	2f		/ no, don't try to save it
	bis	*$CPER,_cpereg	/ yes, or bits into soft copy
	clr	*$CPER		/ clear real CPU error reg
	tst	sp		/ if sp == 0 then red zone
	beq	2b		/ return to red zone code
2:
	mov	SSR0,ssr
#ifdef	SEP_ID
	mov	SSR1,ssr+2
#endif	SEP_ID
	mov	SSR2,ssr+4
	mov	*_mmr3,ssr+6	/ saves loc 0 if no MM SSR3
	mov	$1,SSR0
	jsr	r0,call1; jmp _trap
	/ no return
1:
	mov	$1,SSR0
	tst	_cpereg		/ do we have a CPU error reg ?
	bmi	2f		/ no, don't try to access it
	clr	*$CPER		/ yes, clear it !
2:
	mov	nofault,(sp)
	rtt
.text

.globl	_runrun
call1:
	mov	saveps,-(sp)
	SPL0
	br	1f

call:
	mov	PS,-(sp)
1:
#ifdef	K_OV
	mov	__ovno,-(sp)	/ save overlay number
#else	K_OV
	clr	-(sp)		/ save position on stack for overlay number.
#endif	K_OV
	mov	r1,-(sp)
	mfpd	sp
	mov	6(sp),-(sp)
	bic	$!37,(sp)
	bit	$30000,PS
	beq	1f
	jsr	pc,(r0)+
#ifdef	UCB_NET
	jsr	pc,checknet
#endif	UCB_NET
	tstb	_runrun
	beq	2f
	mov	$12.,(sp)		/ trap 12 is give up cpu
	jsr	pc,_trap
2:
	tst	(sp)+
	mtpd	sp
	br	2f
1:
	bis	$30000,PS
	jsr	pc,(r0)+
#ifdef	UCB_NET
	jsr	pc,checknet
#endif	UCB_NET
	cmp	(sp)+,(sp)+
2:
	mov	(sp)+,r1
#ifdef	K_OV
	mov	(sp)+,r0
/	cmp	r0,__ovno
/	beq	1f
	mov	PS,-(sp)
	SPLHIGH
	mov	r0,__ovno
	asl	r0
	mov	ova(r0),KISA_OV
	mov	ovd(r0),KISD_OV
	mov	(sp)+,PS	/ restor PS, unmask interrupts
1:
	tst	(sp)+
#else
	cmp	(sp)+,(sp)+
#endif	K_OV
	mov	(sp)+,r0
	rtt

.globl	_savfp
_savfp:
#ifdef	FPP
	tst	fpp
	beq	9f		/ No FP hardware
	mov	2(sp),r1
	stfps	(r1)+
	setd
	movf	fr0,(r1)+
	movf	fr1,(r1)+
	movf	fr2,(r1)+
	movf	fr3,(r1)+
	movf	fr4,fr0
	movf	fr0,(r1)+
	movf	fr5,fr0
	movf	fr0,(r1)+
9:
#endif	FPP
	rts	pc

.globl	_restfp
_restfp:
#ifdef	FPP
	tst	fpp
	beq	9f
	mov	2(sp),r1
	mov	r1,r0
	setd
	add	$8.+2.,r1
	movf	(r1)+,fr1
	movf	(r1)+,fr2
	movf	(r1)+,fr3
	movf	(r1)+,fr0
	movf	fr0,fr4
	movf	(r1)+,fr0
	movf	fr0,fr5
	movf	2(r0),fr0
	ldfps	(r0)
9:
#endif	FPP
	rts	pc

/ save floating point error registers
/ argument is a pointer to a two-word
/ structure

.globl	_stst
_stst:
#ifdef	FPP
	tst	fpp
	beq	9f
	stst	*2(sp)
9:
#endif	FPP
	rts	pc

.globl	_addupc
_addupc:
	mov	r2,-(sp)
	mov	6(sp),r2	/ base of prof with base,leng,off,scale
	mov	4(sp),r0	/ pc
	sub	4(r2),r0	/ offset
	clc
	ror	r0
	mov	6(r2),r1
	clc
	ror	r1
	mul	r1,r0		/ scale
	ashc	$-14.,r0
	inc	r1
	bic	$1,r1
	cmp	r1,2(r2)	/ length
	bhis	1f
	add	(r2),r1		/ base
	mov	nofault,-(sp)
	mov	$2f,nofault
	mfpd	(r1)
	add	12.(sp),(sp)
	mtpd	(r1)
	br	3f
2:
	clr	6(r2)
3:
	mov	(sp)+,nofault
1:
	mov	(sp)+,r2
	rts	pc

.globl	_display
_display:
#ifdef	SEP_ID
	tst	_cdreg		/ display register present ?
	beq	2f		/ no
	dec	dispdly		/ yes
	bge	2f
	clr	dispdly
	mov	PS,-(sp)
	mov	$HIPRI,PS
	mov	CSW,r1
	bit	$1,r1
	beq	1f
	bis	$30000,PS
	dec	r1
1:
	jsr	pc,fuword
	mov	r0,CSW
	mov	(sp)+,PS
	cmp	r0,$-1
	bne	2f
	mov	$120.,dispdly		/ 2 sec delay after CSW fault
2:
#endif	SEP_ID
	rts	pc

.globl	_backup
.globl	_regloc
.globl _osp		/ Fix for stack backup problem on 11/60
#ifdef	SEP_ID
_backup:
	mov	2(sp),r0
	movb	ssr+2,r1
	jsr	pc,1f
	movb	ssr+3,r1
	jsr	pc,1f
	movb	_regloc+7,r1
	asl	r1
	add	r0,r1
	mov	ssr+4,(r1)
	clr	r0
2:
	rts	pc
1:
	mov	r1,-(sp)
	asr	(sp)
	asr	(sp)
	asr	(sp)
	bic	$!7,r1
	movb	_regloc(r1),r1
	asl	r1
	add	r0,r1
	sub	(sp)+,(r1)
	rts	pc

#else	SEP_ID

_backup:
	mov	2(sp),ssr+2
	mov	r2,-(sp)
	jsr	pc,backup
	mov	r2,ssr+2
	mov	(sp)+,r2
	movb	jflg,r0
	bne	2f
	mov	2(sp),r0
	movb	ssr+2,r1
	jsr	pc,1f
	movb	ssr+3,r1
	jsr	pc,1f
	movb	_regloc+7,r1
	asl	r1
	add	r0,r1
	mov	ssr+4,(r1)
	clr	r0
2:
	rts	pc
1:
	mov	r1,-(sp)
	asr	(sp)
	asr	(sp)
	asr	(sp)
	bic	$!7,r1
	movb	_regloc(r1),r1
	asl	r1
	add	r0,r1
	sub	(sp)+,(r1)
	rts	pc

/ hard part
/ simulate the ssr2 register missing
/ from non separate I & D space CPUs

backup:
	clr	r2		/ backup register ssr1
	mov	$1,bflg		/ clrs jflg
	clrb	fflg
	clrb	iflg
	mov	ssr+4,r0
	jsr	pc,fetch
	mov	r0,r1
	ash	$-11.,r0
	bic	$!36,r0
	jmp	*0f(r0)
0:		t00; t01; t02; t03; t04; t05; t06; t07
		t10; t11; t12; t13; t14; t15; t16; t17

t00:
	clrb	bflg

t10:
	mov	r1,r0
	swab	r0
	bic	$!16,r0
	jmp	*0f(r0)
0:		u0; u1; u2; u3; u4; u5; u6; u7

u6:	/ single op, m[tf]pi, sxt, illegal
	bit	$400,r1
	beq	u5		/ all but m[tf], sxt
	bit	$200,r1
	beq	1f		/ mfpi
	bit	$100,r1
	bne	u5		/ sxt

/ simulate mtpi with double (sp)+,dd
	bic	$4000,r1	/ turn instr into (sp)+
	br	t01

/ simulate mfpi with double ss,-(sp)
1:
	ash	$6,r1
	bis	$46,r1		/ -(sp)
	br	t01

u4:	/ jsr
	mov	r1,r0
	jsr	pc,setreg	/ assume no fault
	bis	$173000,r2	/ -2 from sp
	rts	pc

f5:	/ movei, movfi
	incb	iflg
	clrb	bflg
	mov	r1,r0
	jsr	pc,setreg
	/ 11/60 stuff, OHMS 1/10/84
	cmp	_cputype,$60.
	bne	1f
	tstb	iflg		/ clear for int output, set for long
	beq	1f		/ strange 11/60 is different
	jsr	pc,fixstk
	clr	r2		/ zero the adjustment for 11/60 fp inst
1:				/ cause the reg hasn't been modified yet
	rts	pc		/ end 11/60 stuff - OHMS
	

t07:	/ EIS
	clrb	bflg

u0:	/ jmp, swab

	/ moved sop's. see below

ff1:	/ ldfps
ff2:	/ stfps
ff3:	/ stst
	mov	r1,r0
	jbr	setreg		/ br too far

	/ new sop handling
u5:	/ single op
					/
	mov	r1,r0			/
	jsr	pc,setreg		/
	mov	r1,r0		/ do this for mode 2 only !
	bic	$!70,r0		/
	cmp	$20,r0		/
	bne	1f		/
	cmp	_cputype,$34.		/
	bne	1f			/
	jsr	pc,fixstk	/ for stack prob.
	clr	r2
1:					/
	rts	pc			/

t01:	/ mov
t02:	/ cmp
t03:	/ bit
t04:	/ bic
t05:	/ bis
t06:	/ add
t16:	/ sub
	clrb	bflg

t11:	/ movb
t12:	/ cmpb
t13:	/ bitb
t14:	/ bicb
t15:	/ bisb
	mov	r1,r0
	ash	$-6,r0
	jsr	pc,setreg
	mov	r1,r0		/ sm2 for 34 type - OHMS
	bic	$!7000,r0	/ do this for source mode 2 only !
	cmp	$2000,r0	/
	bne	1f		/
	cmp	_cputype,$34.		/
	bne	1f			/
	bic	$370,r2			/
1:				/ end OHMS
	swab	r2
	mov	r1,r0
	jsr	pc,setreg

/ if delta(dest) is zero,
/ no need to fetch source

	bit	$370,r2
	beq	1f

/ if mode(source) is R,
/ no source fault is possible

	bit	$7000,r1
	beq	1f

/ if reg(source) is reg(dest),
/ too bad.

	mov	r2,-(sp)
	bic	$174370,(sp)
	cmpb	1(sp),(sp)+
	beq	u7

/ start source cycle
/ pick up value of reg

	mov	r1,r0
	ash	$-6,r0
	bic	$!7,r0
	movb	_regloc(r0),r0
	asl	r0
	add	ssr+2,r0
	mov	(r0),r0

/ if reg has been incremented,
/ must decrement it before fetch
/ ----- not on the 11/34 - OHMS

/	cmp	_cputype,$34.		/ skip for 34 types - OHMS
/	beq	2f			/
	bit	$174000,r2
	ble	2f
	dec	r0
	bit	$10000,r2
	beq	2f
	dec	r0
2:

/ if mode is 6,7 fetch and add X(R) to R

	bit	$4000,r1
	beq	2f
	bit	$2000,r1
	beq	2f
	mov	r0,-(sp)
	mov	ssr+4,r0
	add	$2,r0
	jsr	pc,fetch
	add	(sp)+,r0
2:

/ fetch operand
/ if mode is 3,5,7 fetch *

	jsr	pc,fetch
	bit	$1000,r1
	beq	1f
	bit	$6000,r1
	jne	fetch
1:
	rts	pc

t17:	/ floating point instructions
	clrb	bflg
	mov	r1,r0
	swab	r0
	bic	$!16,r0
	jmp	*0f(r0)
0:		f0; f1; f2; f3; f4; f5; f6; f7

f0:
	mov	r1,r0
	ash	$-5,r0
	bic	$!16,r0
	jmp	*0f(r0)
0:		ff0; ff1; ff2; ff3; ff4; ff5; ff6; ff7

f1:	/ mulf, modf
f2:	/ addf, movf
f3:	/ subf, cmpf
f4:	/ movf, divf
ff4:	/ clrf
ff5:	/ tstf
ff6:	/ absf
ff7:	/ negf
	incb	fflg		/ was inc fflg
	mov	r1,r0
	jsr	pc,setreg
	/ 11/60 stuff OHMS 1/10/84
	cmp	_cputype,$60.
	bne	1f
	jsr	pc,fixstk
	clr	r2		/ zero the adjustment for 11/60 fp inst
1:				/ cause the reg hasn't been modified yet
	rts	pc		/ end 11/60 stuff - OHMS

f6:
	bit	$400,r1
	beq	f1	/ movfo
	jbr	f5	/ movie  - br to far - OHMS

f7:
	bit	$400,r1
	jeq	f5	/ movif - beq too far - OHMS
	br	f1	/ movof

ff0:	/ cfcc, setf, setd, seti, setl
u1:	/ br
u2:	/ br
u3:	/ br
u7:	/ illegal
	incb	jflg
	rts	pc

setreg:
	mov	r0,-(sp)
	bic	$!7,r0
	bis	r0,r2
	mov	(sp)+,r0
	ash	$-3,r0
	bic	$!7,r0
	movb	0f(r0),r0
	tstb	bflg
	beq	1f
	bit	$2,r2
	beq	2f
	bit	$4,r2
	beq	2f
1:
	cmp	r0,$20
	beq	2f
	cmp	r0,$-20
	beq	2f
	asl	r0
2:
	tstb	fflg
	beq	3f
	bit	$10,r1		/ handle deferred mode fp errors - OHMS
	bne	5f		/ if odd mode skip adjustment
	asl	r0
	stfps	r1
	bit	$200,r1
	beq	5f
	asl	r0
3:
	tstb	iflg		/ for float to int and long - OHMS
	beq	5f
	mov	r1,-(sp)
	stfps	r1		/ see if output is int or long
	bit	$100,r1
	bne	4f
	clrb	iflg		/ clear iflg for int
4:
	mov	(sp)+,r1
	bit	$10,r1		/ handle deferred mode fp errors - OHMS
	bne	5f		/ if odd mode skip adjustment
	tstb	iflg
	beq	5f
	asl	r0
5:				/ end - OHMS
	bisb	r0,r2
	rts	pc

0:	.byte	0,0,10,20,-10,-20,0,0

fixstk:		/ new routine for stack growth problem - OHMS
	mov	r2,r0
	bic	$!7,r0
	cmp	$6,r0
	bne	1f
	movb	r2,r0
	ash	$-3,r0
	add	r0,_osp
1:
	rts	pc	/ end - OHMS
	
fetch:
	bic	$1,r0
	mov	nofault,-(sp)
	mov	$1f,nofault
	mfpi	(r0)
	mov	(sp)+,r0
	mov	(sp)+,nofault
	rts	pc

1:
 	mov	(sp)+,nofault
	clrb	r2			/ clear out dest on fault
	mov	$-1,r0
	rts	pc

.data
bflg:	.=.+1
jflg:	.=.+1
fflg:	.=.+1
iflg:	.=.+1
.text
#endif	SEP_ID


.globl	_fubyte, _subyte
.globl	_fuword, _suword
.globl	_fuibyte, _suibyte
.globl	_fuiword, _suiword
_fuibyte:
#ifndef	NONSEPERATE
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,giword
	br	2f
#endif	NONSEPERATE
_fubyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword

2:
	cmp	r1,2(sp)
	beq	1f
	swab	r0
1:
	bic	$!377,r0
	rts	pc

_suibyte:
#ifndef	NONSEPERATE
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,giword
	mov	r0,-(sp)
	cmp	r1,4(sp)
	beq	1f
	movb	6(sp),1(sp)
	br	2f
1:
	movb	6(sp),(sp)
2:
	mov	(sp)+,r0
	jsr	pc,piword
	clr	r0
	rts	pc
#endif	NONSEPERATE

_subyte:
	mov	2(sp),r1
	bic	$1,r1
	jsr	pc,gword
	mov	r0,-(sp)
	cmp	r1,4(sp)
	beq	1f
	movb	6(sp),1(sp)
	br	2f
1:
	movb	6(sp),(sp)
2:
	mov	(sp)+,r0
	jsr	pc,pword
	clr	r0
	rts	pc

_fuiword:
#ifndef	NONSEPERATE
	mov	2(sp),r1
fuiword:
	jsr	pc,giword
	rts	pc
#endif	NONSEPERATE

_fuword:
	mov	2(sp),r1
fuword:
	jsr	pc,gword
	rts	pc

#ifndef	NONSEPERATE
giword:
	mov	PS,-(sp)
	SPLHIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mfpi	(r1)
	mov	(sp)+,r0
	br	1f
#endif	NONSEPERATE

gword:
	mov	PS,-(sp)
	SPLHIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mfpd	(r1)
	mov	(sp)+,r0
	br	1f

_suiword:
#ifndef	NONSEPERATE
	mov	2(sp),r1
	mov	4(sp),r0
suiword:
	jsr	pc,piword
	rts	pc
#endif	NONSEPERATE

_suword:
	mov	2(sp),r1
	mov	4(sp),r0
suword:
	jsr	pc,pword
	rts	pc

#ifndef	NONSEPERATE
piword:
	mov	PS,-(sp)
	SPLHIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,-(sp)
	mtpi	(r1)
	br	1f
#endif	NONSEPERATE

pword:
	mov	PS,-(sp)
	SPLHIGH
	mov	nofault,-(sp)
	mov	$err,nofault
	mov	r0,-(sp)
	mtpd	(r1)
1:
	mov	(sp)+,nofault
	mov	(sp)+,PS
	rts	pc

err:
	mov	(sp)+,nofault
	mov	(sp)+,PS
	tst	(sp)+
	mov	$-1,r0
	rts	pc

.globl _copysin, _copysout

/*
 * copysin(src, dst, maxcnt)
 * This routine copies in bytes until it reaches
 * maxcnt or hits a null.  The return value is
 * the number of character transfered, or -1 if
 * there is a memory fault.
 */

_copysin:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	6(sp),r1
	mov	10(sp),r0
	mov	12(sp),r2
	mov	nofault,-(sp)
	mov	$9f,nofault
	bit	$1,r1
	beq	1f
	/* source starts on odd boundry, special case first byte */
	dec	r1
	mfpd	(r1)+
	swab	(sp)
	movb	(sp)+,(r0)+
	beq	6f
	dec	r2
1:
	mov	r2,r3
	asr	r2
	beq	2f
1:
	mfpd	(r1)+
	movb	(sp),(r0)+
	beq	5f
	swab	(sp)
	movb	(sp)+,(r0)+
	beq	6f
	sob	r2,1b
2:
	asr	r3
	bcc	6f
	mfpd	(r1)
	movb	(sp)+,(r0)+
6:
	sub	12(sp),r0
	br	8f
5:
	tst	(sp)+
	br	6b

/*
 * copysout(src, dst, maxcnt)
 * This routine copies out bytes until it reaches
 * maxcnt or hits a null.  The return value is
 * the number of characters transfered, or -1 if
 * there is a memory fault.
 */
_copysout:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	6(sp),r0
	mov	10(sp),r1
	mov	12(sp),r2
	mov	nofault,-(sp)
	mov	$9f,nofault
	bit	$1,r1
	beq	1f
	/* Odd destination, special case first byte */
	dec	r1
	mfpd	(r1)
	movb	(r0)+,1(sp)
	beq	5f
	mtpd	(r1)+
	dec	r2
1:
	/* save r2 so we can check for odd count at the end */
	mov	r2,r3
	asr	r2
	beq	3f
1:
	movb	(r0)+,-(sp)
	beq	4f
	movb	(r0)+,1(sp)
	beq	5f
	mtpd	(r1)+
	sob	r2,1b
3:
	asr	r3
	bcc	6f
	/* count was odd, copy last byte */
	mfpd	(r1)
	movb	(r0)+,(sp)
	mtpd	(r1)
	br	6f
4:
	cmpb	-(r0),(sp)+	/* bump r0 and sp back */
	mfpd	(r1)
	movb	(r0)+,(sp)
5:
	mtpd	(r1)		/* copy out last word */
6:
	sub	10(sp),r0	/* return value equals # of bytes transfered */
	jbr	8f
	
.globl	_copyin, _copyout

_copyin:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	6(sp),r0
	mov	10(sp),r1
	mov	12(sp),r2
	mov	nofault,-(sp)
	mov	$9f,nofault
	bit	$1,r0
	beq	1f
	dec	r0
	mfpd	(r0)+
	movb	1(sp),(r1)+
	tst	(sp)+
	dec	r2
1:
	mov	r2,r3
	asr	r2
	beq	4f
	bit	$1,r1
	bne	2f
1:
	mfpd	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	4f
2:
	mfpd	(r0)+
	movb	(sp),(r1)+
	swab	(sp)
	movb	(sp)+,(r1)+
	sob	r2,2b
4:
	asr	r3
	bcc	7f
	mfpd	(r0)
	movb	(sp)+,(r1)+
7:
	clr	r0
8:
	mov	(sp)+,nofault
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc
9:
	mov	$-1,r0
	br	8b

_copyout:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	6(sp),r0
	mov	10(sp),r1
	mov	12(sp),r2
	mov	nofault,-(sp)
	mov	$9b,nofault
	bit	$1,r1
	beq	1f
	dec	r1
	mfpd	(r1)
	movb	(r0)+,1(sp)
	mtpd	(r1)+
	dec	r2
1:
	mov	r2,r3
	asr	r2
	beq	4f
	bit	$1,r0
	bne	2f
1:
	mov	(r0)+,-(sp)
	mtpd	(r1)+
	sob	r2,1b
	br	4f
2:
	movb	(r0)+,-(sp)
	movb	(r0)+,1(sp)
	mtpd	(r1)+
	sob	r2,2b
4:
	asr	r3
	bcc	7b
	mfpd	(r1)
	movb	(r0)+,(sp)
	mtpd	(r1)
	br	7b


/* end of string copy routines */
.globl	_idle, _waitloc
_idle:
	mov	PS,-(sp)
	SPL0
	wait
waitloc:
	mov	(sp)+,PS
	rts	pc

	.data
_waitloc:
	waitloc
	.text

#ifdef	PROFILE
/ These words are to insure that times reported for _save
/ do not include those spent while in idle mode, when
/ statistics are gathered for system profiling.
	rts	pc
	rts	pc
	rts	pc
#endif	PROFILE

.globl	_save
_save:
	mov	(sp)+,r1
	mov	(sp),r0
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
#ifdef	K_OV
	mov	__ovno,(r0)+
#endif	K_OV
	mov	r1,(r0)+
	clr	r0
	jmp	(r1)

.globl	_resume
_resume:
	mov	2(sp),r0		/ new process
	mov	4(sp),r1		/ new stack
	SPLHIGH
	mov	r0,KDSA6		/ In new process
	mov	(r1)+,r2
	mov	(r1)+,r3
	mov	(r1)+,r4
	mov	(r1)+,r5
	mov	(r1)+,sp
#ifdef	K_OV
	mov	(r1)+,r0
	mov	r0,__ovno
	asl	r0
	mov	ova(r0),KISA_OV
	mov	ovd(r0),KISD_OV
#endif	K_OV
	mov	$1,r0
	SPL0
	jmp	*(r1)+

.globl	_spl0, _spl1, _spl4, _spl5, _spl6, _spl7, _splx
_spl0:
	mov	PS,r0
	SPL0
	rts	pc

_spl1:
	mov	PS,r0
	SPL1
	rts	pc

_spl4:
	mov	PS,r0
	SPL4
	rts	pc

_spl5:
	mov	PS,r0
	SPL5
	rts	pc

_spl6:
	mov	PS,r0
	SPL6
	rts	pc

_spl7:
	mov	PS,r0
	SPLHIGH
	rts	pc

_splx:
	mov	2(sp),PS
	rts	pc

.globl	_copy, _clear, _kdsa6
/*
 * Copy count clicks from src to dst.
 * Uses KDSA5 and 6 to copy with mov instructions.
 * Interrupt routines must restor segmentation registers if needed;
 * see seg.h.
 *
 * copy(src, dst, count)
 * memaddr src, dst;
 * int count;
 */
_copy:
	jsr	r5, csv
	mov	PS,-(sp)	/ Have to lock out interrupts...
#ifdef	COPY7
	SPLHIGH
#endif	COPY7
#ifdef	COPY1
	bit	$0340,(sp)	/ Are we currently at spl?
	bne	1f
	SPL1			/ Nope, lock out the network interrupts.
1:
#endif COPY1
	mov	KDSA5,-(sp)	/ save seg5
	mov	KDSD5,-(sp)	/ save seg5
	mov	10(r5),r3	/ count
	beq	3f
	mov	4(r5),KDSA5	/ point KDSA5 at source
	mov	$2,KDSD5	/ 64 bytes, read-only
	mov	sp,r4
	mov	$eintstk,sp	/ switch to intstk
	mov	KDSA6,_kdsa6
	mov	6(r5),KDSA6	/ point KDSA6 at destination
	mov	$6,KDSD6	/ 64 bytes, read-write
1:
	mov	$5*8192.,r0
	mov	$6*8192.,r1
	mov	$8.,r2		/ copy one click (8*8)
2:
	mov	(r0)+,(r1)+
	mov	(r0)+,(r1)+
	mov	(r0)+,(r1)+
	mov	(r0)+,(r1)+
	sob	r2,2b

	inc	KDSA5		/ next click
	inc	KDSA6
	dec	r3
	bne	1b
	mov	_kdsa6,KDSA6
	mov	$usize-1\<8.|6, KDSD6
	clr	_kdsa6
	mov	r4,sp
3:
	mov	(sp)+,KDSD5	/ restore seg5
	mov	(sp)+,KDSA5	/ restore seg5
	mov	(sp)+,PS	/ back to normal priority
	jmp	cret

/*
 * Clear count clicks at dst.
 * Uses KDSA5.
 * Interrupt routines must restor segmentation registers if needed;
 * see seg.h.
 *
 * clear(dst, count)
 * mdmaddr dst;
 * int count;
 */
_clear:
	jsr	r5, csv
	mov	KDSA5,-(sp)	/ save seg5
	mov	KDSD5,-(sp)	/ save seg5
	mov	4(r5),KDSA5	/ point KDSA5 at source
	mov	$6,KDSD5	/ 64 bytes, read-write
	mov	6(r5),r3	/count
	beq	3f
1:
	mov	$5*8192.,r0
	mov	$8.,r2		/ clear one click (8*8)
2:
	clr	(r0)+
	clr	(r0)+
	clr	(r0)+
	clr	(r0)+
	sob	r2,2b

	inc	KDSA5		/ next click
	dec	r3
	bne	1b
3:
	mov	(sp)+,KDSD5	/ restore seg5
	mov	(sp)+,KDSA5	/ restore seg5
	jmp	cret

/* New copy routine */

.globl	_copyio

/* copyio(long phyaddr, char *cp, count, mode) */
_copyio:
	mov	r5,-(sp)
	mov	sp,r5
	cmp	(r5)+,(r5)+
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	mov	KDSA5,-(sp)
	mov	KDSD5,-(sp)
	mov	_kdsa6,-(sp)
	bne	1f
	mov	KDSA6,_kdsa6
1:
	/* go to spl1, and switch stacks (if we have to) */
	/* note that we can still access arguments because we */
	/* haven't re-mapped KDSA6 yet. We'll get all the info */
	/* we need from the arguments before we re-map KDSA6 */

	mov	PS,-(sp)
	bit	$340,(sp)
	bne	1f
	SPL1
1:
	mov	sp, r0		/* Save the current stack pointer */
	cmp	$eintstk, sp	/* are we in the tmp stack already? */
	bhis	1f
	mov	$eintstk, sp	/* nope, switch stacks. */
1:
	mov	r0,-(sp)	/* save the old stack pointer */
	mov	KDSA6, -(sp)
	mov	KDSD6, -(sp)

	/* get mode bits: Kernel, User-I, or User-D; read/write */
	mov	10(r5),r3
	mov	r3,r4
	bic	$177771,r4
	clr	r0
	mov	4(r5),r1		/ get dest address
	ashc	$3,r0			/ shift APF into r0
	asl	r0
	add	cbase(r4),r0
	cmp	-(sp),-(sp)		/* space for second KDS[AD]6 value */
	mov	40(r0),-(sp)		/* initial KDSA6 value */
	mov	(r0)+,-(sp)		/* initial KDSD6 value */
	bit	$17,r0
	bne	1f
	sub	$20,r0
1:
	mov	40(r0),6(sp)		/* second KDSA6 value */
	mov	(r0),4(sp)		/* second KDSD6 value */

	mov	$6,r0			/* 0140000, APF bits */
	ashc	$13.,r0			/* convert to bytes, adding in offset */
	mov	r0,r4

	mov	(r5)+,r0		/* get paddr into r0/r1 register pair */
	mov	(r5)+,r1
	ashc	$10.,r0			/* click part of paddr is now in r0 */
	mov	r0,KDSA5		/*   so map KDSA5 to it */
	mov	$128.-1\<8.|6,KDSD5	/* set up the descriptor part */
	mov	$1200,r0		/* KDSA5 in clicks */
	ashc	$6,r0			/* convert to bytes, add in offset */

	/* check to see if the dst buffer crosses an 8k boundry.  If */
	/* it does, we'll have to do copy in two parts. */
	mov	(r5)+,r2		/ get buffer address
	mov	(r5)+,r5		/ get count
	bis	$160000,r2		/ get length left until
	neg	r2			/    the next 8k boundry
	cmp	r2,r5
	bhis	1f
	/* buffer crosses boundry.  Set copy count to be just up to the */
	/* 8k boundry, and save residual count on the stack. */
	sub	r2,r5
	br	2f
1:
	/* buffer fits.  We can do this in one copy. */
	/* save a residual count of 0 */
	mov	r5,r2
	clr	r5
2:
	/* copy in or out?  Check bit, and swap src/dest if we need to. */
	bit	$1,r3
	bne	1f
	mov	r0,r1
	mov	r4,r0
	br	2f
1:
	mov	r4,r1
2:
	mov	(sp)+,KDSD6
	mov	(sp),KDSA6
	mov	nofault,(sp)
	mov	$cerr,nofault
domore:
	bit	$1,r1
	bne	1f
	bit	$1,r0
	beq	gcete
	br	gcote
1:
	bit	$1,r0
	beq	gceto
gcoto:				/* copy odd to odd */
	dec	r2
	movb	(r0)+,(r1)+
gcete:				/* copy even to even */
	asr	r2
	beq	2f
1:
	mov	(r0)+,(r1)+
	sob	r2,1b
2:
	bcc	9f
	movb	(r0)+,(r1)+
	br	9f
gceto:				/* copy even to odd - have to do byte by byte */
gcote:				/* copy odd to even - have to do byte by byte */
1:
	movb	(r0)+,(r1)+
	sob	r2,1b
9:
	mov	r5,r2		/* get the residual count */
	beq	2f
				/* residual not zero. more to do... */
	mov	4(sp),KDSA6	/* get new mapping values */
	mov	2(sp),KDSD6
	clr	r5		/* clear residual so we don't do this again */
	bit	$1,r3
	bne	1f
	mov	$140000,r0
	jbr	domore
1:
	mov	$140000,r1
	jbr	domore
2:
	clr	r0
1:
	/* And now to restore everything... */
	mov	(sp)+,nofault
	cmp	(sp)+,(sp)+
	mov	(sp)+,KDSD6
	mov	(sp)+,KDSA6
	mov	(sp)+,sp
	mov	(sp)+,PS
	mov	(sp)+,_kdsa6
	mov	(sp)+,KDSD5
	mov	(sp)+,KDSA5
	mov	(sp)+,r2
	mov	(sp)+,r3
	mov	(sp)+,r4
	mov	(sp)+,r5
	rts	pc
cerr:
	mov	$-1,r0
	br	1b
.data
cbase:
	UDSD0
	KDSD0
	UISD0
	KISD0
.text

#ifdef	UCB_NET
	.globl _netisr, _netintr
checknet:
	mov	PS,-(sp)
	SPLHIGH
	tst	_netisr		/ net requesting soft interrupt
	beq	3f
/ZARF	bit	$340, 16.(sp)	/ check to see if we were at spl0
	bit	$340, 18.(sp)	/ check to see if we were at spl0
	bne	3f
	SPL1
	jsr	pc, *$_netintr
3:
	mov	(sp)+,PS
	rts	pc

#endif	UCB_NET

/*
 *	copyv(fromaddr,toaddr,count)
 *	virtual_addr	fromaddr,toaddr;
 *	unsigned count;
 *
 *	Copy two arbitrary pieces of PDP11 virtual memory from one location
 *	to another.  Up to 8k bytes can be copied at one time.
 *
 *	A PDP11 virtual address is a two word value; a 16 bit "click" that
 *	defines the start in physical memory of an 8KB segment and and offset.
 */

	.globl	_copyv

#ifdef	COPY7
	.data
copyvsave: 0
	.text
#endif	COPY7

_copyv:
	jsr	r5,csv
	tst	14(r5)		/* if (count == 0)		*/
	jeq	copyvexit	/*	return;			*/
	cmp	$20000,14(r5)	/* if (count >= 8192		*/
	jlos	copyvexit	/*	return;			*/

	mov	4(r5),r4	/* fromclick = fromclick	*/
	mov	6(r5),r2	/* foff = fromoffset		*/
	mov	10(r5),r1	/* click = toclick		*/
	mov	12(r5),r3	/* toff = tooffset		*/
	mov	14(r5),r0	/* count = count		*/

docopyv:
	add	$5*8192.,r2	/* foff = virtual addr (page 5) */
	add	$6*8192.,r3	/* toff = virtual addr (page 6) */
	mov	PS,-(sp)	/* Lock out interrupts. sigh...	*/
#ifdef	COPY7
	SPLHIGH
#endif	COPY7
#ifdef	COPY1
	bit	$0340,(sp)	/* Are we currently at SPL? */
	bne	1f
	SPL1			/* Nope, lock out the network */
1:
#endif	COPY1
	mov	KDSA5,-(sp)	/* save seg5			*/
	mov	KDSD5,-(sp)

	mov	r4,KDSA5	/* seg5 = fromclick		*/
	mov	$128.-1\<8.|2,KDSD5

#ifdef	COPY7
	/* Note: the switched stack is only for use of a fatal	*/
	/* kernel trap occurring during the copy; otherwise we	*/
	/* might conflict with the other copy routine		*/
	mov	sp,r4		/* switch stacks		*/
	mov	$eintstk,sp
	mov	KDSA6,copyvsave
#endif	COPY7
#ifdef	COPY1
	mov	sp,r4		/* switch stacks		*/
	cmp	$eintstk,sp	/* are we already in the intstk? */
	bhis	1f
	mov	$eintstk,sp	/* No, point sp into intstk	*/
1:
	mov	_kdsa6,-(sp)	/* save the current saved kdsa6 */
	bne	1f		/* was it set to anything? */
	mov	KDSA6,_kdsa6	/* nope, set it to the current kdsa6 */
1:
	mov	KDSA6,-(sp)	/* save the u block address	*/
	mov	KDSD6,-(sp)	/* save the u block descriptor	*/
#endif	COPY1

	mov	r1,KDSA6	/* seg6 = click			*/
	mov	$128.-1\<8.|6,KDSD6

	/****** Finally do the copy			   ******/
	mov	r3,r1		/* Odd addresses or count?	*/
	bis	r2,r1
	bis	r0,r1
	bit	$1,r1
	bne	copyvodd	/* Branch if odd		*/

	asr	r0		/* Copy a word at at time	*/
1:
	mov	(r2)+,(r3)+
	sob	r0,1b
	br	copyvdone

copyvodd:
	movb	(r2)+,(r3)+	/* Copy a byte at at time	*/
	sob	r0,copyvodd
/	br	copyvdone

copyvdone:
#ifdef	COPY7
	mov	copyvsave,KDSA6	/* remap in the stack		*/
	mov	$usize-1\<8.|6,*$KDSD6
#endif	COPY7
#ifdef	COPY1
	mov	(sp)+,*$KDSD6
	mov	(sp)+,KDSA6	/* remap in the stack		*/
/	mov	$usize-1\<8.|6,*$KDSD6
	mov	(sp)+,_kdsa6	/* restore old _kdsa6 value	*/
#endif	COPY1
	mov	r4,sp
	mov	(sp)+,KDSD5	/* restor seg5			*/
	mov	(sp)+,KDSA5
	mov	(sp)+,PS	/* unlock interrupts		*/

copyvexit:
	clr	r0
	clr	r1
	jmp	cret

/ unsigned divide routine
/
/ From System V.2  udiv.s  1.3

.globl	udiv, urem

udiv:
	cmp	r1,$1
	ble	9f
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	(sp)+,r0
	rts	pc
9:
	bne	9f
	tst	r0
	rts	pc
9:
	cmp	r1,r0
	blos	9f
	clr	r0
	rts	pc
9:
	mov	$1,r0
	rts	pc

urem:
	cmp	r1,$1
	ble	9f
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	(sp)+,r0
	mov	r1,r0
	rts	pc
9:
	bne	9f
	clr	r0
	rts	pc
9:
	cmp	r0,r1
	blo	9f
	sub	r1,r0
9:
	tst	r0
	rts	pc

/ Long quotient

	.globl	ldiv
ldiv:
	jsr	r5,csv
	mov	10.(r5),r3
	sxt	r4
	bpl	1f
	neg	r3
1:
	cmp	r4,8.(r5)
	bne	hardldiv
	mov	6.(r5),r2
	mov	4.(r5),r1
	bge	1f
	neg	r1
	neg	r2
	sbc	r1
	com	r4
1:
	mov	r4,-(sp)
	clr	r0
	div	r3,r0
	mov	r0,r4		/high quotient
	mov	r1,r0
	mov	r2,r1
	mov	r0,-(sp)	/ *
	div	r3,r0
	bvc	1f
	mov	r2,r1		/ *
	mov	(sp),r0		/ *
	sub	r3,r0		/ this is the clever part
	div	r3,r0
	tst	r1
	sxt	r1
	add	r1,r0		/ cannot overflow!
1:
	tst	(sp)+		/ *
	mov	r0,r1
	mov	r4,r0
	tst	(sp)+
	bpl	9f
	neg	r0
	neg	r1
	sbc	r0
9:
	jmp	cret

hardldiv:
	4

/ Long remainder

	.globl	lrem
lrem:
	jsr	r5,csv
	mov	10.(r5),r3
	sxt	r4
	bpl	1f
	neg	r3
1:
	cmp	r4,8.(r5)
	bne	hardlrem
	mov	6.(r5),r2
	mov	4.(r5),r1
	mov	r1,r4
	bge	1f
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	div	r3,r0
	mov	r1,r0
	mov	r2,r1
	mov	r0,-(sp)	/ *
	div	r3,r0
	bvc	1f
	mov	r2,r1		/ *
	mov	(sp),r0		/ *
	sub	r3,r0
	div	r3,r0
	tst	r1
	beq	9f
	add	r3,r1
1:
	tst	(sp)+		/ *
	tst	r4
	bpl	9f
	neg	r1
9:
	sxt	r0
	jmp	cret

/ The divisor is known to be >= 2^15.  Only 16 cycles are
/ needed to get a remainder.
hardlrem:
	4

.globl	lmul
lmul:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	8(sp),r2
	sxt	r1
	sub	6(sp),r1
	mov	12.(sp),r0
	sxt	r3
	sub	10.(sp),r3
	mul	r0,r1
	mul	r2,r3
	add	r1,r3
	mul	r2,r0
	sub	r3,r0
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

#ifdef	UCB_NET

.globl	_badaddr
_badaddr:
	mov	2(sp),r0
	mov	nofault,-(sp)
	mov	$1f,nofault    
	tst	(r0)
	clr	r0
	br	2f
1:
	mov	$-1,r0
2:
	mov	(sp)+,nofault
	rts	pc

/.globl	_bzero
/_bzero:
/	mov	2(sp),r0
/	beq	4f		/ error checking
/	mov	4(sp),r1
/	beq	4f		/ error checking
/	bit	$1,r0		/ start on odd byte?
/	bne	1f
/	clrb	(r0)+		/ clear the odd byte
/	dec	r1		/ and adjust the count
/	beq	4f		/ if it was only one byte we're done
/1:
/	mov	r1,-(sp)	/ save so we can check for odd count later
/	asr	r1
/	bne	2f		/ make sure we still have a non-zero count.
/	tst	(sp)+		/ only 1 byte. fix the stack
/	br	3f		/   and go clear the byte, then return.
/2:
/	clr	(r0)+
/	sob	r1,2b
/	bit	$1,(sp)+	/ was the count odd?
/	beq	4f
/3:
/	clrb	(r0)+		/ clear last odd byte
/4:
/	rts	pc
.globl _bzero
_bzero:
	mov	2(sp),r0
	beq	3f		/ error checking... dgc
	mov	4(sp),r1
	beq	3f		/ error checking... dgc
	bit	$1,r0
	bne	1f
	bit	$1,r1
	bne	1f
	asr	r1
2:
	clr	(r0)+
	sob	r1,2b
	rts	pc
1:
	clrb	(r0)+
	sob	r1,1b
3:
	rts	pc
#endif	UCB_NET

#ifndef	K_OV
.globl	__ovno
.globl	csv

.data
__ovno:	-1	/ tells the world this is not overlay kernel
.text

csv:
	mov	r5,r0
	mov	sp,r5
	clr	-(sp)	 / leave space for (non-exsistant) __ovno
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r0)

.globl	cret
cret:
	mov	r5,r2
	tst	-(r2)	/ skip (non-existant) __ovno
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
#else	K_OV
/ C register save and restore -- version 7/75
/ modified by wnj && cbh 6/79 for overlayed text registers
/ modified by was 3/80 for use in kernel
/ inter-overlay calls call thunk which calls ovhndlr to
/ save registers.  intra-overlay calls call function
/ directly which calls csv to save registers.
/ New thunks, -Dave Borman. thunk looks like
/	_foo:
/		mov	$~foo+4,r1	/ skip over jsr	r5,csv
/		jsr	r5,ovhndlrX
/ where X is the overlay that we need to switch to.
/ ovhndlr does the csv itself, so is no need to do the jsr r5,csv

.globl	csv
.globl	cret
.globl	__ovno
.globl	_etext
.data
__ovno:	0
.text
csv:
	mov	r5,r1
	mov	sp,r5
	mov	__ovno,-(sp)	/ overlay is extra (first) word in mark
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r1)		/ jsr part is sub $2,sp

cret:
	mov	r5,r2
/ get the overlay out of the mark, and if it is non-zero
/ make sure it is the currently loaded one
	mov	-(r2),r4
	bne	1f		/ zero is easy
2:
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
/ not returning to base segment, so check that the right
/ overlay is mapped in, and if not change the mapping
1:
	cmp	r4,__ovno
	beq	2b		/ lucked out!
#ifdef	SEP_ID
/ if return address is in base segment, then nothing to do
/ Non-split I/D kernels have extracted strings in the overlays, so
/ we can only do this shortcut if we are a split I/D kernel.
	cmp	2(r5),$_etext
	blt	2b
#endif	SEP_ID
/ returning to wrong overlay --- do something!
	mov	PS,-(sp)	/ save PS 
	SPLHIGH
	mov	r4,__ovno
	asl	r4
	mov	ova(r4),KISA_OV
	mov	ovd(r4),KISD_OV
	mov	(sp)+,PS	/ restore PS, unmask interrupts
/ could measure switches[ovno][r4]++ here
	jmp	2b

/ ovhndlr makes the argument (in r0) be the current overlay,
/ saves the registers ala csv (but saves the previous overlay number),
/ and then jmp's to the function, skipping the function's initial
/ call to csv.

.globl	ovhndlr1, ovhndlr2, ovhndlr3, ovhndlr4
.globl	ovhndlr5, ovhndlr6, ovhndlr7
.globl	ovhndlr8, ovhndlr9, ovhndlra, ovhndlrb
.globl	ovhndlrc, ovhndlrd, ovhndlre, ovhndlrf

ovhndlrf:
	mov	$15.,r0
	jbr	ovhndlr
ovhndlre:
	mov	$14.,r0
	jbr	ovhndlr
ovhndlrd:
	mov	$13.,r0
	jbr	ovhndlr
ovhndlrc:
	mov	$12.,r0
	jbr	ovhndlr
ovhndlrb:
	mov	$11.,r0
	jbr	ovhndlr
ovhndlra:
	mov	$10.,r0
	jbr	ovhndlr
ovhndlr9:
	mov	$9.,r0
	jbr	ovhndlr
ovhndlr8:
	mov	$8.,r0
	jbr	ovhndlr
ovhndlr7:
	mov	$7.,r0
	jbr	ovhndlr
ovhndlr6:
	mov	$6.,r0
	jbr	ovhndlr
ovhndlr5:
	mov	$5.,r0
	jbr	ovhndlr
ovhndlr4:
	mov	$4.,r0
	jbr	ovhndlr
ovhndlr3:
	mov	$3.,r0
	jbr	ovhndlr
ovhndlr2:
	mov	$2.,r0
	jbr	ovhndlr
ovhndlr1:
	mov	$1.,r0
ovhndlr:
	cmp	r0,__ovno
	bne	1f
	mov	sp,r5
	mov	__ovno,-(sp)
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r1)
1:
	mov	sp,r5
	mov	__ovno,-(sp)	/ save previous overlay number
	mov	PS,-(sp)	/ save PS
	SPLHIGH
	mov	r0,__ovno	/ set new overlay number
	asl	r0
	mov	ova(r0),KISA_OV
	mov	ovd(r0),KISD_OV
	mov	(sp)+,PS	/ restore PS, unmask interrupts
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r1)
#endif	K_OV

.globl	_u
_u	= 140000
usize	= 32.

CSW	= 177570
PS	= 177776
SSR0	= 177572
SSR1	= 177574
SSR2	= 177576
SSR3	= 172516
KISA0	= 172340
KISA1	= 172342
KISA2	= 172344
KISA5	= 172352
KISA6	= 172354
KISA7	= 172356
KISD0	= 172300
KISD2	= 172304
KISD5	= 172312
KISD6	= 172314
KISD7	= 172316
#ifdef	SEP_ID
KDSA0	= 172360
KDSA5	= 172372
KDSA6	= 172374
KDSA7	= 172376
KDSD0	= 172320
KDSD5	= 172332
KDSD6	= 172334
KISA_OV	= KISA7		/ KISA register used for overlays
KISD_OV	= KISD7		/ KISD register used for overlays
#else	SEP_ID
KDSA0	= KISA0
KDSA5	= KISA5
KDSA6	= KISA6
KDSA7	= KISA7
KDSD0	= KISD0
KDSD5	= KISD5
KDSD6	= KISD6
KISA_OV	= KISA2		/ KISA register used for overlays
KISD_OV	= KISD2		/ KISD register used for overlays
#endif	SEP_ID
SISA0	= 172240
SISA1	= 172242
SISA2	= 172244
SISD0	= 172200
SISD1	= 172202
SISD2	= 172204
UISA0	= 177640
UISA1	= 177642
UISD0	= 177600
UISD1	= 177602
#if	defined(SEP_ID) || !defined(NONSEPERATE)
UDSD0	= 177620
#else	defined(SEP_ID) || !defined(NONSEPERATE)
UDSD0	= UISD0
#endif	defined(SEP_ID) || !defined(NONSEPERATE)
MSCR	= 177746	/ 11/70 memory control register
			/ 11/44 cache control register
UBMR0	= 170200	/ unibus map reg. base address
PIRQ	= 177772	/ programmed interrupt request register
CPER	= 177766

IO	= 177600
SWR	= 177570
.data
.globl	_ka6
.globl _cputype
.globl _maxmem, _el_prcw, _rn_ssr3
.globl _sepid, _ubmaps, _cdreg, _nmser
.globl _cpereg, _mmr3

#ifdef	SEP_ID
_ka6:	KDSA6
_cputype: 45.
#else	SEP_ID
.text
_ka6:	KISA6
_cputype: 40.
#endif	SEP_ID

/ The following parameters describe the
/ hardware features available on this CPU.
/ They are initialized by the standalone boot code.
/ *** MUST BE IN TEXT SPACE ON OVERLAY KERNEL ***
/ *** MUST BE IN DATA (NOT BSS) SPACE OTHERWISE ***

_sepid:	0	/ seperate I & D space
_ubmaps: 0	/ unibus map
_nmser: 0	/ number of memory system error registers
_cdreg: 0	/ console display register
_maxmem: 0	/ Initially set to total number of 64 byte memory
		/ segments available by the standalone `boot'.
		/ Later gets changed to the maximum memory size
		/ of a user process (2048).
_el_prcw: 0	/ Parity CSR configuration word.
_rn_ssr3: 0	/ release # & M/M SSR 3
_cpereg: 0	/ soft copy of CPU error register if it exists, otherwise -1
_mmr3: 0	/ address of M/M SSR 3 if it exists, otherwise 0

#ifdef	K_OV
/ overlay descriptor tables

.globl ova, ovd, ovend

ova:	.=.+32.	/ overlay addresses
ovd:	.=.+32.	/ overlay sizes
ovend:	.=.+2	/ end of overlays

#endif	K_OV
.data
stk:	0

.data
.globl	nofault		/ global declaration needed for fpsim
nofault:.=.+2
#ifdef	FPP
fpp:	.=.+2
#endif	FPP
ssr:	.=.+8.
dispdly:.=.+2
saveps:	.=.+2
.globl	fltused		/ this guy isn't used, he's just here to
fltused: .=.+2		/ resolve references if you use floating
			/ point in a driver.

#if	defined(SEP_ID) && defined(PROFILE)
.text
/ system profiler
/  Expects to have a KW11-P in addition to the line-frequency
/  clock, and it should be set to BR7.
/  Uses supervisor I space register 2&3 (40000-100000)
/  to maintain the profile.

#ifndef	KWV11
CCSB	= 172542
CCSR	= 172540
IVECT	= 104
BITS	= 113
COUNT	= 100.
#else	KWV11
CCSR	= 170420
CCSB	= 170422
IVECT	= 440
BITS	= 173		/* INTOV, RATE = Line(50/60Hz), MODE = Mode 1, GO */
COUNT	= -54.		/* 2's complement. (interrupt every .9 seconds) */
#endif	KWV11

#ifdef	DONT_USE_THIS_CODE
.globl	_sprof, _xprobuf, _probsiz, _mode
#else	DONT_USE_THIS_CODE
.globl	_sprof, _probsiz, _mode, _isprof
#endif	DONT_USE_THIS_CODE
_probsiz = 37777

_isprof:
	mov	$1f,nofault
	mov	$_sprof,IVECT	/ interrupt
	mov	$340,IVECT+2	/ pri
	mov	$COUNT,CCSB	/ count set = 100
	mov	$BITS,CCSR	/ count down, 10kHz, repeat
1:
	clr	nofault
	rts	pc

_sprof:
	mov	r0,-(sp)
	mov	PS,r0
	ash	$-10.,r0
	bic	$!14,r0		/ mask out all but previous mode
	add	$1,_mode+2(r0)
	adc	_mode(r0)
	cmp	r0,$14		/ user
	beq	done
	mov	2(sp),r0	/ pc
	asr	r0
	asr	r0
	bic	$140001,r0
#ifdef	NOOVPROF
	cmp	r0,$_probsiz
	blo	1f
	inc	_outside
	br	done
1:
#else	NOOVPROF
	cmp	$34000,r0	/ 0160000 - start of overlay
	bhi	1f
	mov	r0,-(sp)
	mov	__ovno,r0
	ash	$11.,r0
	add	(sp)+,r0
1:
#endif
	mov	$10340,PS		/ prev = super
	mfpi	40000(r0)
	inc	(sp)
	mtpi	40000(r0)
#ifdef	DONT_USE_THIS_CODE
	bne	done
	mov	r1,-(sp)
	mov	$_xprobuf,r1
2:
	cmp	(r1)+,r0
	bne	3f
	inc	(r1)
	br	4f
3:
	tst	(r1)+
	bne	2b
	sub	$4,r1
	mov	r0,(r1)+
	mov	$1,(r1)+
4:
	mov	(sp)+,r1
#endif	DONT_USE_THIS_CODE
done:
	mov	(sp)+,r0
	mov	$BITS,CCSR
	rtt

#ifdef	DONT_USE_THIS_CODE
/ count subroutine calls during profiling
/  of the system.

.globl	mcount, _profcnts, _profsize

.data
_profcnts:
	.=.+[6*340.]

.globl countbase
.data
countbase:
	_profcnts
_profsize:
	340.
.text

mcount:
	mov	(r0),r1
	bne	1f
	mov	countbase,r1
	beq	2f
	add	$6,countbase
	cmp	countbase,$_profcnts+[6*340.]
	blo	3f
	clr	countbase
	rts	pc
3:
	mov	(sp),(r1)+
	mov	r1,(r0)
1:
	inc	2(r1)
	bne	2f
	inc	(r1)
2:
	rts	pc

.data
_xprobuf:.=.+512.
_proloc:.=.+2
#endif	DONT_USE_THIS_CODE
.data
_mode:	.=.+16.
_outside: .=.+2

#endif	defined(SEP_ID) && defined(PROFILE)
