/ SCCSID: @(#)select.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/  nfound = select(nfds, readfds, writefds, execptfds, timeout)
/	int	nfound, nfds;
/	long	*readfds, *writefds, *execptfds, *timeout;
/
/ execptfds is not used.  It's here so that the C calling sequence
/ is the same as on the VAX, even though the assembly interface
/ is different.

.globl	cerror
.globl  _select

.select = 98.

_select:
	mov	r5,-(sp)
	mov	sp,r5
	mov     4.(r5),0f
	mov     6.(r5),0f+2
	mov     8.(r5),0f+4
	mov	12.(r5),0f+6
	clr     r0
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys     .select; 0:.. ; .. ; .. ; ..
.text
