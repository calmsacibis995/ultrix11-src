/ SCCSID: @(#)symlink.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C library -- symlink

/ error = symlink(name1, name2);
/	  char *name1, *name2;

.globl  _symlink
.globl	cerror
.symlink = 109.

_symlink:
	mov	r5,-(sp)
	mov	sp,r5
	mov     4(r5),r0
	mov     6(r5),0f
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	clr	r0
	rts	pc
.data
9:
	sys	.symlink; 0:..
