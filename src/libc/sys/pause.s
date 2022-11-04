/ SCCSID: @(#)pause.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library - pause

.globl	_pause
.pause = 29.

_pause:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.pause
	mov	(sp)+,r5
	rts	pc
