/ SCCSID: @(#)03bool.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ BOOLEAN OPERATIONS
/
_AND:
	tst	(sp)+
	bne	1f
	clr	(sp)
1:
	return
_OR:
	bis	(sp)+,(sp)
	return
_NOT:
	inc	(sp)
	bic	$!1,(sp)
	return
