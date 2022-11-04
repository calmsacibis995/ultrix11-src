/ SCCSID: @(#)csv1.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ C register save
/
/ The loader finagles the .o files so that
/ calls to csv in overlays get changed to
/ calls to ovcsv.

.globl	csv, ovcsv
.globl	csv__com
.globl	__ovno
.globl  ovhndlr

halt= 0


/ csv for routines in overlays
/ the previous overlay is in __ovno, which is saved on the stack.
/ after it is saved, __ovno is set to the current overlay number
/ which has been put in r0 by the thunk.
ovcsv:
	mov	r5,r1
	mov	sp,r5
	cmp	$7,r0
	bge	1f
	halt
1:
	mov	__ovno,-(sp)
	mov	r0,__ovno
	jmp	csv__com	/ puts us into the regular csv routine
