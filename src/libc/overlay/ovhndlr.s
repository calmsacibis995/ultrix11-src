/ SCCSID: @(#)ovhndlr.s	3.0	5/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Overlay Handler for new thunk structures.
/ Jerry Brenner 12/27/83

.globl	ovhndlr
.globl	__ovno, __novno

emt=104000
halt=0

ovhndlr:
	cmp	r0, __ovno
	beq	2f
1:
	mov	r0, __novno
	emt
	cmp	r0, __novno
	bne	1b
2:
	jmp	(r1)
