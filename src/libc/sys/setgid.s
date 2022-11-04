/ SCCSID: @(#)setgid.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- setgid

/ error = setgid(uid);

.globl	_setgid
.globl	cerror
.setgid = 46.

_setgid:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	.setgid
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
