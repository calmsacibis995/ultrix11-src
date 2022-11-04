/ SCCSID: @(#)getuid.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- getuid, geteuid

/ uid = getuid();

.globl	_getuid
.getuid = 24.

_getuid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.getuid
	mov	(sp)+,r5
	rts	pc


/ uid = geteuid();
/  returns effective uid

.globl	_geteuid

_geteuid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.getuid
	mov	r1,r0
	mov	(sp)+,r5
	rts	pc
