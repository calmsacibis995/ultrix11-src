/ SCCSID: @(#)ffs.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ ffs(bits)	return an index to the first bit that is set in 'bits'.
/		if 'bits' is zero, return -1.  Index is based at 1.
/    long bits
.globl	_ffs, csv, cret
.text
_ffs:
	jsr	r5,csv
	mov	4(r5),r2
	jne	2f
	mov	6(r5),r3
	jne	2f
	mov	$-1,r0
1:
	jmp	cret
2:
	mov	$1,r0
3:
	bit	$1,r3
	jne	1b
	inc	r0
	ashc	$-1,r2
	jbr	3b
