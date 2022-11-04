
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)tzname.c	3.0	4/22/86";

/***************************************************************
* tzname
* return timezone string for /etc/profile and /etc/cshprofile
***************************************************************/
#include <stdio.h>
#include <nlist.h>
#include <ctype.h>

#define NULL 0

struct nlist    nl[] = { { "_timezone" }, { "" }, };

static struct zone
{
    int     offset;
    char   *stdzone;
    char   *dlzone;
} zonetab[] =
{
    -1 * 60, "MET", "MET DST",		/* Middle European */
    -2 * 60, "EET", "EET DST",		/* Eastern European */
     4 * 60, "AST", "ADT",		/* Atlantic */
     5 * 60, "EST", "EDT",		/* Eastern */
     6 * 60, "CST", "CDT",		/* Central */
     7 * 60, "MST", "MDT",		/* Mountain */
     8 * 60, "PST", "PDT",		/* Pacific */
     0, "GMT", 0,			/* Greenwich */
/* there's no way to distinguish this from GMT */
     0 * 60, "WET", "WET DST",		/* Western European */
    -10 * 60, "EST", "EST",		/* Aust: Eastern */
    -10 * 60 + 30, "CST", "CST", 	/* Aust: Central */
    -8 * 60, "WST", 0,			/* Aust: Western */
     0, "", 0				/* NULL entry     */
};

main (argc, argv)
int     argc;
char  **argv;
{
    int     zonemins;			/* minutes +/- GMT               */
    int     mem;			/* /dev/mem file descriptor      */
    struct zone *zp;			/* ptr to zone table entries     */

    if ((argc > 2) || (argc == 2 && ((abs (zonemins = atoi (argv[1]))) > 720)))
    {
	fprintf (stderr, "%s: syntax - %s [minutes]\n", argv[0], argv[0]);
	exit (1);
    }

    /******************************
    * look for timezone in /unix
    ******************************/
    if (argc == 1)
    {
	if (nlist ("/unix", nl) < 0)
	{
	    fprintf (stderr, "%s: can't access system namelist in /unix\n",
		    argv[0]);
	    exit (1);
	}
	if ((nl[0].n_type == 0) || (nl[0].n_value == 0))
	{
	    fprintf (stderr, "%s: can't find symbol '_timezone' in /unix\n",
		    argv[0]);
	    exit (2);
	}
	if ((mem = open ("/dev/mem", 0)) < 0)
	{
	    fprintf (stderr, "%s: can't open memory (/dev/mem)\n", argv[0]);
	    exit (3);
	}
	if (lseek (mem, (long) nl[0].n_value, 0) < 0)
	{
	    fprintf (stderr, "%s: can't seek to %d\n", argv[0], nl[0].n_value);
	    exit (4);
	}
	if (read (mem, (char *)&zonemins, sizeof(zonemins)) < sizeof(zonemins))
	{
	    fprintf (stderr, "%s: can't read zonemins\n", argv[0]);
	    exit (5);
	}
    }

    /*******************************************
    * look for matching zonemins in zone table 
    ********************************************/
    for (zp = zonetab; *zp -> stdzone != '\0'; ++zp)
	if (zp -> offset == zonemins)
	{
	    fprintf (stdout, "%s%d%s\n", zp -> stdzone, zp -> offset / 60,
		    zp -> dlzone ? zp -> dlzone : "");
	    return (0);
	}
    exit(6);
}
/****************************************************************
* end of tzname.c
*****************************************************************/
