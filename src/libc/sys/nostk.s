/ SCCSID: @(#)nostk.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library-- nostk

/ nostk()

.globl	_nostk,
.globl	cerror
.nostk = 84.

_nostk:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.nostk
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
