/ SCCSID: @(#)ovhndlr.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Overlay Handler for new thunk structures.
/ Dave Borman 12/27/83

.globl	ovhndlr1, ovhndlr2, ovhndlr3, ovhndlr4
.globl	ovhndlr5, ovhndlr6, ovhndlr7
.globl	__ovno, __novno

emt=104000
halt=0

/ Ok, so like, here's what a thunk now looks like
/	_foo:
/		mov	$~foo+4,r1
/		jsr	r5,ovhndlrX
/ where X is the overlay number.  ovhndlrX will put
/ X into r0 and then jump to ovhndlr. ovhndlr then
/ switches to the overlay specified in r0, does a
/ csv (in line code) and then jumps through r1 to
/ start executing the sub-routine. Notice that the
/ thunk saved the addr of ~foo+4, so when we jump
/ to the routine we'll skip over the jsr r5,csv.
ovhndlr7:
	mov	$7,r0
	jbr	ovhndlr
ovhndlr6:
	mov	$6,r0
	jbr	ovhndlr
ovhndlr5:
	mov	$5,r0
	jbr	ovhndlr
ovhndlr4:
	mov	$4,r0
	jbr	ovhndlr
ovhndlr3:
	mov	$3,r0
	jbr	ovhndlr
ovhndlr2:
	mov	$2,r0
	jbr	ovhndlr
ovhndlr1:
	mov	$1,r0
ovhndlr:
	cmp	r0, __ovno
	beq	2f
1:
	mov	r0, __novno
	emt
	cmp	r0, __novno
	bne	1b
2:
	mov	sp,r5
	cmp	$7,r0
	bge	1f
	halt
1:
	mov	__ovno,-(sp)
	mov	r0,__ovno
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r1)		/ jsr part is sub $2,sp
