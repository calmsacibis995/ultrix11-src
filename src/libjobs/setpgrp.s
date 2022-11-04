/ SCCSID: @(#)setpgrp.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Based on:	setpgrp.s	2.2	(Berkeley)
/ C library -- setpgrp, getpgrp

/ setpgrp(pid, pgrp);	/* set pgrp of pid and descendants to pgrp */
/ if pid==0 use current pid
/
/ getpgrp(pid)
/ implemented as setpgrp(pid, -1)


.globl	_setpgrp
.globl	cerror

_setpgrp:
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
.globl	_getpgrp
_getpgrp:
	mov	r5,-(sp)
	mov	sp,r5
	mov	$0.,0f
	mov	4(r5),r0
	br	2b

.data
9:
	sys	setpgrp; 0:..

