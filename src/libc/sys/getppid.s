/ SCCSID: @(#)getppid.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ getppid -- get parent process ID

.globl	_getppid
.getpid	= 20.

_getppid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.getpid
	mov	r1,r0
	mov	(sp)+,r5
	rts	pc
