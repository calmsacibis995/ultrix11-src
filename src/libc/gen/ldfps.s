/ SCCSID: @(#)ldfps.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
ldfps = 170100^tst
/
/ ldfps(number);

.globl	_ldfps
_ldfps:
	mov	r5,-(sp)
	mov	sp,r5
	ldfps	4(r5)
	mov	(sp)+,r5
	rts	pc
