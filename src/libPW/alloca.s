/	Sccsid:	@(#)alloca.s 3.0 4/22/86

//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////

/	wdptr = alloca(nbytes);
/ like alloc, but automatic
/ automatic free upon return from calling function

.globl _alloca
_alloca:
	mov	(sp)+,r1	/ return
	mov	(sp),r0		/count
	inc	r0
	bic	$1,r0		/ round up
	sub	r0,sp		/ take core
	mov	sp,r0
	tst	(r0)+		/ returned value; will generate
				/ a memory fault if there isn't enough core
	jmp	(r1)
.data
	<@(#)alloca	3.1\0>
