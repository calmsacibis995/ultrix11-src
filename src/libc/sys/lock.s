/ SCCSID: @(#)lock.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ lock -- C library

/	lock(f)
/ modified to share same syscall as SYSTEM V plock
/ Bill Burns 3/20/85

.globl	_lock, _plock, cerror

.lock = 53.

_plock:
	mov	r5,-(sp)
	mov	sp,r5
	br	2f
_lock:
	mov	r5,-(sp)
	mov	sp,r5
	tst	4(r5)
	beq	2f
	mov	$1000,4(r5)
2:
	mov	4(r5),0f
	sys	0; 9f
	.data
9:
	sys	.lock; 0:..
	.text
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
