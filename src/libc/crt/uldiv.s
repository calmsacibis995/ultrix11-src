/ SCCSID: @(#)uldiv.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Unsigned long quotient
/
/ Based on: (System V) uldiv.s  1.3

.globl	uldiv
.globl	csv, cret
uldiv:
	jsr	r5,csv
	tst	8.(r5)
	bne	hardldiv
	mov	10.(r5),r3
	bmi	hardldiv
	mov	6.(r5),r2
	mov	4.(r5),r1
	bmi	hardest
	clr	r0
	div	r3,r0
	mov	r0,-(sp)		/high quotient
	mov	r1,r0
	mov	r2,r1
	mov	r0,r4
	div	r3,r0
	bvc	1f
	mov	r4,r0
	mov	r2,r1
	sub	r3,r0		/ this is the clever part
	div	r3,r0
	tst	r1
	sxt	r1
	add	r1,r0		/ cannot overflow!
1:
	mov	r0,r1
	mov	(sp)+,r0
	jmp	cret

/ The divisor is known to be >= 2^15 so only 16 cycles are needed.
hardldiv:
	mov	6.(r5),r2
	mov	4.(r5),r1
	clr	r0
	mov	8.(r5),r3
	mov	$16.,r4
1:
	clc
	rol	r2
	rol	r1
	rol	r0
	cmp	r3,r0
	bhi	3f
	blo	2f
	cmp	10.(r5),r1
	blos	2f
3:
	sob	r4,1b
	br	1f
2:
	sub	10.(r5),r1
	sbc	r0
	sub	r3,r0
	inc	r2
	sob	r4,1b
1:
	clr	r0
	mov	r2,r1
	jmp	cret

/ The dividend is known to be >= 2^31 so 32 cycles are needed.
hardest:
	mov	6.(r5),r3
	mov	4.(r5),r2
	clr	r1
	clr	r0
	mov	$32.,r4
1:
	clc
	rol	r3
	rol	r2
	rol	r1
	rol	r0
	cmp	8.(r5),r0
	bhi	3f
	blo	2f
	cmp	10.(r5),r1
	blos	2f
3:
	sob	r4,1b
	br	1f
2:
	sub	10.(r5),r1
	sbc	r0
	sub	8.(r5),r0
	inc	r3
	sob	r4,1b
1:
	mov	r2,r0
	mov	r3,r1
	jmp	cret
