/ SCCSID: @(#)nice.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library-- nice

/ error = nice(hownice)

.globl	_nice,
.globl	cerror
.nice = 34.

_nice:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),r0
	sys	.nice
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
