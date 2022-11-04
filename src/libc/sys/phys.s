/ SCCSID: @(#)phys.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ phys -- C library

/	phys(seg, size, physad)

.globl	_phys, cerror

.phys = 52.

_phys:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	mov	8(r5),0f+4
	sys	0; 9f
	.data
9:
	sys	.phys; 0: ..; ..; ..
	.text
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
