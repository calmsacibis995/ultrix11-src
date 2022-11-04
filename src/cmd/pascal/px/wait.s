/ SCCSID: @(#)wait.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- wait

/ pid = wait(&status);
/
/ pid == -1 if error
/ status indicates fate of process

.globl	_wait, cerror

_wait:
	mov	r5,-(sp)
	mov	sp,r5
	sys	wait
	bec	1f
	jmp	cerror
1:
	mov	r1,*4(r5)	/ status return
	mov	(sp)+,r5
	rts	pc
