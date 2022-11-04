/ SCCSID: @(#)alarm.s	3.0	(ULTRIX-11)	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library - alarm

.globl	_alarm
.alarm = 27.

_alarm:
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	.alarm
	mov	(sp)+,r5
	rts	pc
