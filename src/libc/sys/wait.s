/ SCCSID: @(#)wait.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- wait

/ pid = wait(0);
/   or,
/ pid = wait(&status);
/
/ pid == -1 if error
/ status indicates fate of process, if given

.globl	_wait
.globl	cerror
.wait = 7.

_wait:
	mov	r5,-(sp)
	mov	sp,r5
	sys	.wait
	bec	1f
	jmp	cerror
1:
	tst	4(r5)
	beq	1f
	mov	r1,*4(r5)	/ status return
1:
	mov	(sp)+,r5
	rts	pc
