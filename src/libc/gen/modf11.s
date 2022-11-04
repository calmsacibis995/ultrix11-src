/ SCCSID: @(#)modf11.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ double modf(x, *fp)
/ double x, *fp;
/ return fractional part
/ stash integer part (as double)

.globl	_modf
.globl	csv, cret
one	= 040200

_modf:
	jsr	r5,csv
	movf	4(r5),fr0
	modf	$one,fr0
	movf	fr1,*12.(r5)
	jmp	cret
