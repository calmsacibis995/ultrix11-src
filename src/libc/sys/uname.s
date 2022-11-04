/ SCCSID: @(#)uname.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- uname

/ uname(unixname);
/ unixname[0], ...unixname[7] contain the unixname

.globl _uname, cerror
.pwbsys = 57.
uname = 0

_uname:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	.pwbsys; uname
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
