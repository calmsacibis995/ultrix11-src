/ SCCSID: @(#)bzero.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/

.globl  _bzero
_bzero:
	mov     2(sp),r0
	mov     4(sp),r1
	bit     $1,r0
	bne     1f
	bit     $1,r1
	bne     1f
	asr     r1
2:      clr     (r0)+
	sob     r1,2b
	rts     pc

1:      clrb    (r0)+
	sob     r1,1b
	rts     pc
