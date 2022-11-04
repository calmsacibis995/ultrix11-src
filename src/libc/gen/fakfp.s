/ SCCSID: @(#)fakfp.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ fakefp -- fake floating point simulator

.globl	fptrap
signal = 48.
rti = 2

fptrap:
	sub	$2,(sp)
	mov	r0,-(sp)
	sys	signal; 4; 0
	mov	(sp)+,r0
	rti
