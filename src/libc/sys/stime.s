/ SCCSID: @(#)stime.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ error = stime(&long)

.globl	_stime
.globl	cerror
.stime = 25.

_stime:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),r1
	mov	(r1)+,r0
	mov	(r1),r1
	sys	.stime
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
