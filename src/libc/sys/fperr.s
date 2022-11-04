/ SCCSID: @(#)fperr.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- fperr

/ fperr(bufp)
/ bufp - points to a three word array which
/	 holds floating point status registers
/ bufp is not passed to the system call !
/ no error

.globl	_fperr,
.globl	cerror
.fperr = 95.

_fperr:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),-(sp)	/ save bufp on stack
	sys	0; 9f		/ get FP error code & address
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5	/ get buffer pointer
	stfps	(r5)		/ load FP status register
	mov	r0,2(r5)	/ load FP error code
	mov	r1,4(r5)	/ load FP error address
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.fperr
