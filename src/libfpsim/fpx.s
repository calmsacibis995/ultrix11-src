/ SCCSID = @(#)fpx.s	3.0	4/22/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ fpx -- floating point simulation

.getfp = 45.
.data
reenter: 1
calsys:
	sys	.getfp; calarg: ..; ..
agndat:
	i.cfcc		/ 170000
	i.setf		/ 170001
	i.seti		/ 170002
	badins
	badins
	badins
	badins
	badins
	badins
	i.setd		/ 170011
	i.setl		/ 170012
cls2dat:
	badins		/ 1700xx
	i.ldfps		/ 1701xx
	i.stfps		/ 1702xx
	badins		/ 1703xx - stst
	i.clrx		/ 1704xx
	i.tstx		/ 1705xx
	i.absx		/ 1706xx
	i.negx		/ 1707xx
cls3dat:
	badins		/ 1700xx
	badins		/ 1704xx
	i.mulx		/ 1710xx
	i.modx		/ 1714xx
	i.addx		/ 1720xx
	i.ldx		/ 1724xx
	i.subx		/ 1730xx
	i.cmpx		/ 1734xx
	i.stx		/ 1740xx
	i.divx		/ 1744xx
	i.stexp		/ 1750xx
	i.stcxj		/ 1754xx
	i.stcxy		/ 1760xx
	i.ldexp		/ 1764xx
	i.ldcjx		/ 1770xx
	i.ldcyx		/ 1774xx
moddat:
	mod0
	mod1
	mod2
	mod3
	mod4
	mod5
	mod6
	mod7
.bss
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
fpbuf:	.=.+2
modctl: .=.+4

