/ SCCSID: @(#)fpx.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ fpx -- floating point simulation
.globl _reenter

.data
_reenter: 1

.bss
bins:	.=.+30
asign:	.=.+2
areg:	.=.+8
aexp:	.=.+2
bsign:	.=.+2
breg:	.=.+8
bexp:	.=.+2

fpsr:	.=.+2
trapins: .=.+2

ac0:	.=.+8.
ac1:	.=.+8.
ac2:	.=.+8.
ac3:	.=.+8.
ac4:	.=.+8.
ac5:	.=.+8.

sr0:	.=.+2
sr1:	.=.+2
	.=.+2
	.=.+2
	.=.+2
	.=.+2
ssp:	.=.+2
spc:	.=.+2
sps:	.=.+2
pctmp:	.=.+8

