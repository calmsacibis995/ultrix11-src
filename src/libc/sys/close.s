/ SCCSID: @(#)close.s	3.0	(ULTRIX-11)	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- close

/ error =  close(file);

.globl	_close,
.globl	cerror
.close = 6.

_close:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	.close
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
