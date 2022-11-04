/ SCCSID: @(#)ghostid.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- ghostid

/ long ghostid();
/ long hostid;
/ hostid = ghostid();

.globl	_ghostid
.local = 58.
.ghostid = 26.

_ghostid:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.local; 9f
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.ghostid
