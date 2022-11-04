/ SCCSID: @(#)27conv.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ CONVERSIONS
/
_STOI:
	tst	(sp)
	sxt	-(sp)
	return
_STOD:
	tst	(sp)
	sxt	-(sp)
_ITOD:
	movif	(sp)+,fr0
	movf	fr0,-(sp)
	return
_ITOS:
	tst	(sp)+
	return
