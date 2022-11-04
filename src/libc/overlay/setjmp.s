/ SCCSID: @(#)setjmp.s	3.0	5/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- setjmp, longjmp
/ Overlay version -- believes in 4 word jump vector

/	longjmp(a,v)
/ will generate a "return(v)" from
/ the last call to
/	setjmp(a)
/ by restoring sp, r5, pc from `a'
/ and doing a return.
/

.globl	_setjmp
.globl	_longjmp
.globl	csv, cret
.globl  __ovno,ovcsv

_setjmp:
	mov	2(sp),r0
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	(sp),(r0)+	/
	mov	__ovno,(r0)	/
	clr	r0
	rts	pc

_longjmp:
	jsr	r5,csv
	mov	4(r5),r1
	mov	6(r5),r0
	bne	1f
	mov	$1,r0
1:
	cmp	(r5),(r1)
	beq	2f
	mov	(r5),r5
	bne	1b
/ panic -- r2-r4 lost
	br	1f
2:
	/ can't use cret as number of args may vary - jsd
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
1:
	mov	(r1)+,r5
	mov	(r1)+,sp
	mov	(r1)+,(sp)	/
	mov	r0, r2		/ preserve r0 in the already destroyed r2
	mov	(r1),r0		/load the proper overlay number
	jsr	r5,ovcsv		/
				/ this routine can be anywhere instead of
				/ forced into the root overlay
	mov	r2,r0		/
	jmp	cret
