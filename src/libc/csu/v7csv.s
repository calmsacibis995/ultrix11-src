/ SCCSID: @(#)v7csv.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C register save and restore -- version 7/75
/ Modified to be compatable with both stock v7 objects
/ and with overlayed objects. 7/10/84 -Dave Borman

.globl	csv
.globl	cret

csv:
	mov	r5,r0
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	tst	-(sp)		/ bump stack by 2 to account for extra
				/ offset in overlayed objects. If the
				/ module isn't overlayed, this will just
				/ show up as an extra word on the stack.
	jsr	pc,(r0)		/ jsr part is sub $2,sp

cret:
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
