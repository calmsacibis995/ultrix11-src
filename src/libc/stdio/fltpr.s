/	SCCSID: @(#)fltpr.s	3.0	(ULTRIX-11)	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/	(System V)  fltpr.s	1.2
/ Interface to floating output

.globl	fltcvt
.globl	fltused

.globl	__cvt

fltused:
fltcvt:
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r0,-(sp)
	movf	(r4)+,fr0
	movf	fr0,-(sp)
	jsr	pc,__cvt
	add	$16.,sp
	rts	pc
