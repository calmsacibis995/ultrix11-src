/ SCCSID: @(#)spgrp.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Based on: 	@(#)setpgrp.s	2.0 (ULTRIX-11)	2/27/85
/ Based on:	@(#)setpgrp.s	2.2	(Berkeley)
/ code taken from Berkeley s/getpgrp which is in libjobs
/ this code is called on to simulate the System III/V s/getpgrp calls
/ see sgpgrp.c

setpgrp = 39.
.globl	_spgrp
.globl	cerror

_spgrp:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),0f
2:	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc

.data
9:
	sys	setpgrp; 0:..
