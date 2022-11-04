/ SCCSID: @(#)pipe.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ pipe -- C library

/	pipe(f)
/	int f[2];

.globl	_pipe, cerror

.pipe = 42.

_pipe:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.pipe
	bec	1f
	jmp	cerror
1:
	mov	r2,-(sp)
	mov	4(r5),r2
	mov	r0,(r2)+
	mov	r1,(r2)
	clr	r0
	mov	(sp)+,r2
	mov	(sp)+,r5
	rts	pc
