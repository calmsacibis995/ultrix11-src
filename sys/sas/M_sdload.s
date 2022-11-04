/ SCCSID: @(#)M_sdload.s	3.0	4/21/86
/
//////////////////////////////////////////////////////////////////////
/   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    /
/   All Rights Reserved. 					     /
/   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      /
//////////////////////////////////////////////////////////////////////
/
/ Header file for M.s used with sdload.c
/ No  - `Sizing Memory...' message
/ No  - auto configure code
/ Yes - size memory by reading not clearing

MSMSG = 0
AUTOCONF = 0
READMEM = 1
