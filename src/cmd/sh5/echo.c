
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)echo.c	3.0	4/22/86
 *	(System V)  echo.c  1.1
 */
/*
 *	UNIX shell
 *
 *	Bell Telephone Laboratories
 *
 */
#include	"defs.h"

#define	exit(a)	flushb();return(a)

extern int exitval;

echo(argc, argv)
char **argv;
{
	register char	*cp;
	register int	i, wd;
	int	j;
	
	if(--argc == 0) {
		prc_buff('\n');
		exit(0);
	}

	for(i = 1; i <= argc; i++) 
	{
		sigchk();
		for(cp = argv[i]; *cp; cp++) 
		{
			if(*cp == '\\')
			switch(*++cp) 
			{
				case 'b':
					prc_buff('\b');
					continue;

				case 'c':
					exit(0);

				case 'f':
					prc_buff('\f');
					continue;

				case 'n':
					prc_buff('\n');
					continue;

				case 'r':
					prc_buff('\r');
					continue;

				case 't':
					prc_buff('\t');
					continue;

				case 'v':
					prc_buff('\v');
					continue;

				case '\\':
					prc_buff('\\');
					continue;
				case '0':
					j = wd = 0;
					while ((*++cp >= '0' && *cp <= '7') && j++ < 3) {
						wd <<= 3;
						wd |= (*cp - '0');
					}
					prc_buff(wd);
					--cp;
					continue;

				default:
					cp--;
			}
			prc_buff(*cp);
		}
		prc_buff(i == argc? '\n': ' ');
	}
	exit(0);
}

