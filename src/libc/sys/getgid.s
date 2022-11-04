/ SCCSID: @(#)getgid.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- getgid, getegid

/ gid = getgid();

.globl	_getgid
.getgid = 47.

_getgid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.getgid
	mov	(sp)+,r5
	rts	pc

/ gid = getegid();
/ returns effective gid

.globl	_getegid

_getegid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.getgid
	mov	r1,r0
	mov	(sp)+,r5
	rts	pc
