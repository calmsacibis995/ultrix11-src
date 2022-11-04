/ SCCSID: @(#)14neg.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ NEGATION & ABSOLUTE VALUE
/
_ABS2:
	tst	(sp)
	bge	1f
_NEG2:
	neg	(sp)
1:
	sxt	-(sp)
	return
_ABS4:
	tst	(sp)
	bge	1f
_NEG4:
	mov	(sp)+,r0
	neg	r0
	neg	(sp)
	sbc	r0
	mov	r0,-(sp)
1:
	return
_ABS8:
	absf	(sp)
	return
_NEG8:
	negf	(sp)
	cfcc
	return
