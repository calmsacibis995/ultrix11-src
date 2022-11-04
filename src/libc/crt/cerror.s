/ SCCSID: @(#)cerror.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C return sequence which sets errno, returns -1.

.globl	cerror
.comm	_errno,2

cerror:
	mov	r0,_errno
	mov	$-1,r0
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
