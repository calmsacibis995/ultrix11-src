/ SCCSID: @(#)mcount.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ count subroutine calls during profiling

.globl	mcount
.comm	countbase,2

mcount:
	mov	(r0),r1
	bne	1f
	mov	countbase,r1
	beq	2f
	add	$6,countbase
	mov	(sp),(r1)+
	mov	r1,(r0)
1:
	inc	2(r1)
	bne	2f
	inc	(r1)
2:
	rts	pc

