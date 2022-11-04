/ SCCSID: @(#)getpid.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ getpid -- get process ID

.globl	_getpid
.getpid	= 20.

_getpid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.getpid
	mov	(sp)+,r5
	rts	pc
