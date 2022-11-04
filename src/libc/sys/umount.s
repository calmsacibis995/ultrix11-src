/ SCCSID: @(#)umount.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- umount

.globl	_umount
.globl	cerror
indir	= 0
.umount = 22.
.comm	_errno,2

_umount:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),0f
	sys	indir; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

.data
9:
	sys	.umount; 0:..
