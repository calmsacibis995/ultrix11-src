/ SCCSID: @(#)ldiv.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Long quotient
/
/ Based on: (System V) ldiv.s  1.3

.globl	ldiv
.globl	csv, cret

ldiv:
	jsr	r5,csv
	mov	10.(r5),r3
	sxt	r4
	bpl	1f
	neg	r3
	bmi	hardldiv
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
	tst	(sp)+
	bpl	9f
	neg	r0
	neg	r1
	sbc	r0
9:
	jmp	cret

/ The divisor is known to be >= 2^15 so only 17 cycles are needed.
hardldiv:
	clr	-(sp)
	mov	6.(r5),r2
	mov	4.(r5),r1
	bpl	1f
	com	(sp)
	neg	r1
	neg	r2
	sbc	r1
1:
	clr	r0
	mov	8.(r5),r3
	bge	1f
	neg	r3
	neg	10.(r5)
	sbc	r3
	com	(sp)
1:
	clr	-(sp)
	cmp	r3,r0
	bhi	1f
	blo	2f
	cmp	10.(r5),r1
	bhi	1f
2:
	inc	(sp)
	sub	10.(r5),r1
	sbc	r0
	sub	r3,r0
1:
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
	mov	r2,r1
	mov	(sp)+,r0
	tst	(sp)+
	beq	1f
	neg	r0
	neg	r1
	sbc	r0
1:
	jmp	cret
