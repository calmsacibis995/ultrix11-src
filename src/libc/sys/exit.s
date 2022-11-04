/ SCCSID: @(#)exit.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- _exit

/ _exit(code)
/ code is return in r0 to system
/ Same as plain exit, for user who want to define their own exit.

.globl	__exit
.exit = 1.

__exit:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	.exit

