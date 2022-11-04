/ SCCSID: @(#)fpsim.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- fpsim

/ ret = fpsim(arg);
/ arguments:
/	0	turn off fpsim
/	1	turn on fpsim
/	2	get status of fpsim
/ return value:
/	-1	error is setting fpsim
/	0	fpsim turned off
/	1	fpsim turned on
/	2	fpsim not configured

.globl	_fpsim, cerror
.fpsim = 70.

_fpsim:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.fpsim; 0:..
