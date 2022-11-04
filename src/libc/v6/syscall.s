/ SCCSID: @(#)syscall.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ syscall

.globl	_syscall,csv,cret,cerror
_syscall:
	jsr	r5,csv
	mov	r5,r2
	add	$04,r2
	mov	$9f,r3
	mov	(r2)+,r0
	bic	$!0377,r0
	bis	$sys,r0
	mov	r0,(r3)+
	mov	(r2)+,r0
	mov	(r2)+,r1
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	jmp	cret

	.data
9:	.=.+12.
