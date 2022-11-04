/ SCCSID: @(#)hton.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/

.globl  _htonl,_htons,_ntohl,_ntohs
_htonl:
_ntohl:
	mov     2(sp),r0
	mov     4(sp),r1
	swab    r0
	swab    r1
	rts     pc

_htons:
_ntohs:
	mov     2(sp),r0
	swab    r0
	rts     pc

