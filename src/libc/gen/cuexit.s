/ SCCSID: @(#)cuexit.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- exit

/ exit(code)
/ code is return in r0 to system

.globl	_exit
.globl	__cleanup
exit = 1

_exit:
	mov	r5,-(sp)
	mov	sp,r5
	jsr	pc,__cleanup
	mov	4(r5),r0
	sys	exit

