/ SCCSID: @(#)mount.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- mount

/ error = mount(dev, file, flag)

.globl	_mount,
.globl	cerror
.mount = 21.

_mount:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),0f
	mov	6(sp),0f+2
	mov	8(sp),0f+4
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.mount; 0:..; ..; ..
