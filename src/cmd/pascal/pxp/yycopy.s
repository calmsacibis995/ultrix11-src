/ SCCSID: @(#)yycopy.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/
.globl _Y, _OY, _OYcopy
/.globl _Ycopy

_OYcopy:
	mov	$_OY,r0
	mov	$_Y,r1
	mov	r2,-(sp)
/0:
	mov	$8.,r2
1:
	mov	(r1)+,(r0)+
	sob	r2,1b
	mov	(sp)+,r2
	rts	pc
/
/_Ycopy:
/	mov	2(sp),r0
/	mov	4(sp),r1
/	mov	r2,-(sp)
/	br	0b
