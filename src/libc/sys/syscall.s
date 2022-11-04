/ SCCSID: @(#)syscall.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Based on:	@(#)syscall.s	1.3
/ C library -- general system call
/	syscall(sysnum, r0, r1, arg1, arg2, ...)
/		max of 5 args (size u.u_arg in system)

.globl	_syscall, cerror, csv, cret

_syscall:
	jsr	r5,csv
	mov	r5,r2
	add	$4,r2
	mov	$9f,r3
	mov	(r2)+,r0
	bic	$!177,r0
	bis	$104400,r0
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
9:
	sys	0; .=.+10.
