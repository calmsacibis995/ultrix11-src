/ SCCSID: @(#)sync.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
.globl	_sync
.sync = 36.

_sync:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.sync
	mov	(sp)+,r5
	rts	pc
