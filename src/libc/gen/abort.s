/ SCCSID: @(#)abort.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- abort

.globl	_abort
iot	= 4
.globl	csv,cret

_abort:
	jsr	r5,csv
	iot
	clr	r0
	jmp	cret
