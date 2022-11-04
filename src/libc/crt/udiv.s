/ SCCSID: @(#)udiv.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ unsigned divide routine
/
/ Based on: (System V) udiv.s  1.3

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
