
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mkc_btab.c	3.0	4/21/86";
/*
 * The following device name tables (btab & ctab)
 * specify the major device numbers for each device.
 * The rules is simple, the rules is these:
 *
 * 1.	Changes in these tables will most likely require
 *	rebuilding all system and device libraries.
 *	cd /sys/conf; sysgen (s command)
 *	Also any program that `includes' the devmaj.h file
 *	must be recompiled, lots of them !
 *
 * 2.	The order of the block mode device names must be the
 *	same in both tables. Also any name that appears in
 *	btab[] must also be in ctab[], for example "tc" has
 *	no RAW I/O interface, but is in both tables. The mkconf
 *	table for "tc" loads `nodev' into the cdevsw[] table
 *	for the "tc" RAW interface.
 *	1/22/86 -- Fred Canter (tc no longer in tables).
 *
 * 3.	To add a character mode device, such as "dz", insert it's
 *	name before "tty" in the ctab[].
 *
 * 4.	Block mode devices may be added anywhere in the tables
 *	as long as rule 1 is followed.
 *
 * 5.	The Block and Raw major device numbers for the block
 *	mode devices cannot overlap, to prevent this, "dummy"
 *	lines are inserted in ctab ahead of the "rk" line.
 *	The "rk" line must be first in btab and ctab.
 *	This is necessary so that the error log print program
 *	can tell whether the major device number is for 
 *	Block or Raw mode.
 *
 * 6.	The u1 u2 u3 u4 entries in btab & ctab are place markers
 *	for users to add their own device drivers.
 *
 * 7.	The u1 -> u3 entries must be the last entries in btab & ctab.
 *	CDA, ELP, others?, depend on u1_bmaj being equal to the number
 *	of real block devices.
 *
 */

char	*btab[] =
{
	"rk",	/* WARNING: rk must be first (see main.c, labelit.c) */
	"rp",
	"ra",
	"rl",
	"hx",
	"tm",
	"tk",
	"ts",
	"ht",
	"hp",
	"hm",
	"hj",
	"hk",
	"u1",	/* WARNING: u1->u4 must be last in btab */
	"u2",
	"u3",
	"u4",
	0
};
char	*ctab[] =
{
	"console",
	"ct",
	"lp",
	"dc",
	"dh",
	"dp",
	"uh|uhv",	/* dhu/dhv (was dj) */
	"dn",
	"dz|dzv",
	"du",
	"tty",
	"mem",
	"ptc",
	"pts",
	"dummy",	/* Prevent Block & Raw overlap ! */
	"dummy",
	"dummy",
	"rk",	/* WARNING: rk must be first (see main.c, labelit.c) */
	"rp",
	"ra",
	"rl",
	"hx",
	"tm",
	"tk",
	"ts",
	"ht",
	"hp",
	"hm",
	"hj",
	"hk",
	"u1",	/* WARNING: u1->u4 must be last in ctab */
	"u2",
	"u3",
	"u4",
	0
};
