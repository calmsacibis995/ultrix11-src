/	SCCSID: @(#)ffltpr.s	3.0	(ULTRIX-11)	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Based on:	(System V)  ffltpr.s	1.2
/ C library-- fake floating output
.globl	pfloat
.globl	pscien
.globl	pgen
.globl	fltcvt

pfloat:
pscien:
pgen:
fltcvt:
	add	$8,r4
	movb	$'?,(r3)+
	mov	r3,r0
	rts	pc
