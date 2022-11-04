/ SCCSID: @(#)execle.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- execle

/ execle(file, arg1, arg2, ... , 0, env);
/

.globl	_execle
.globl	cerror
.exece	= 59.

_execle:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	r5,r0
	add	$6,r0
	mov	r0,0f+2
1:
	tst	(r0)+
	bne	1b
	mov	(r0),0f+4
	sys	0; 9f
	jmp	cerror
.data
9:
	sys	.exece; 0:..; ..; ..
