/	SCCSID: @(#)_tolower.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Based on:	(System V)	1.3
/ C library -- tolower
/
/	This routine is a fast machine language version of the
/	following C routine:
/
/		int tolower (c)
/			register int c;
/		{
/			if (c >= 'A' && c <= 'Z')
/				c -= 'A' - 'a';
/			return c;
/		}
/
	.globl	_tolower

_tolower:
/	MCOUNT
	mov	2(sp),r0	/ pick up the argument
	cmp	$'A,r0		/ Is it too low?
	jgt	1f		/  ...yes
	cmp	$'Z,r0		/ Too high?
	jlt	1f		/  ...yes
	sub	$'A-'a,r0	/ Make it lower case
1:
	rts	pc		/ Return
