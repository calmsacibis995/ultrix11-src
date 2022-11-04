/ SCCSID: @(#)16dvd.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ FLOATING DIVISION
/
_DVD2:
	tst	(sp)
	sxt	-(sp)
_DVD42:
	movif	(sp)+,fr0
	br	1f
_DVD82:
	movf	(sp)+,fr0
1:
	cfcc
	beq	9f
	tst	(sp)
	sxt	-(sp)
	movif	(sp)+,fr2
	divf	fr0,fr2
	cfcc
	bvs	8f
	movf	fr2,-(sp)
	return
_DVD24:
	tst	(sp)
	sxt	-(sp)
_DVD4:
	movif	(sp)+,fr0
	br	1f
_DVD84:
	movf	(sp)+,fr0
1:
	cfcc
	beq	9f
	movif	(sp)+,fr2
	divf	fr0,fr2
	cfcc
	bvs	8f
	movf	fr2,-(sp)
	return
_DVD28:
	tst	(sp)
	sxt	-(sp)
_DVD48:
	movif	(sp)+,fr0
	br	1f
_DVD8:
	movf	(sp)+,fr0
1:
	cfcc
	beq	9f
	movf	(sp)+,fr2
	divf	fr0,fr2
	cfcc
	bvs	8f
	movf	fr2,-(sp)
	return
9:
	mov	$EFDIVCHK,_perrno
	error	EFDIVCHK
8:
	jmp	fpovflo
