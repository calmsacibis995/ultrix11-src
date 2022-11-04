/ SCCSID: @(#)M_boot.s	3.0	4/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Header file for M.s used with boot.c
/ (all but ULCM-16 boot)
/ Yes - `Sizing Memory...' message
/ No  - auto configure code
/ No  - size memory by reading not clearing

MSMSG = 1
AUTOCONF = 0
READMEM = 0
