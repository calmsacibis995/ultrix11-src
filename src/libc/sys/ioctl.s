/ SCCSID: @(#)ioctl.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Based on:  @(#)ioctl.s	2.2	(Berkeley)
/ C library -- ioctl

/ ioctl(fdes, command, arg)
/ struct * arg;
/
/ result == -1 if error

.globl	_ioctl, cerror
.ioctl = 54.

_ioctl:
	mov	r5,-(sp)
	mov	sp,r5
	mov	8(r5), -(sp)
	mov	6(r5), -(sp)
	mov	4(r5), -(sp)
	sys	.ioctl+200
	bec	1f
	add	$6, sp
	jmp	cerror
1:
	add	$6, sp
	mov	(sp)+,r5
	clr	r0
	rts	pc
