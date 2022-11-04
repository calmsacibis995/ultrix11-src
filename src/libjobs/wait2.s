/ SCCSID: @(#)wait2.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Based on:	wait2.s	2.2	(Berkeley)
/ C library -- wait2

/ pid = wait2(0,flags);
/   or,
/ pid = wait2(&status,flags);
/
/ pid == -1 if error
/ status indicates fate of process, if given
/ to be a syscall miser, we pass the second param
/ thru as bits in the ps

.globl	_wait2
.globl	cerror

_wait2:
	mov	r5,-(sp)
	mov	sp,r5
	mov	6(r5),r0
	sec|sev|sez|sen
	sys	wait
	bec	1f
	jmp	cerror
1:
	tst	4(r5)
	beq	1f
	mov	r1,*4(r5)	/ status return
1:
	mov	(sp)+,r5
	rts	pc
