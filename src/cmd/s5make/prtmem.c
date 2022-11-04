
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)prtmem.c 3.0 4/22/86";

/* System 5	@(#)prtmem.c	1.2 of 3/28/83	*/
 
#ifdef GETU
#define udsize uu[0]
#define ussize uu[1]
#endif


prtmem()
{
#ifdef GETU
#include "stdio.h"
#include "sys/param.h"
#include "sys/dir.h"
#include "sys/user.h"
	unsigned uu[2];
	register int i;

	if(getu( &((struct user *)0)->u_dsize, &uu, sizeof uu) > 0)
	{
		udsize *= 64;
		ussize *= 64;
		printf("mem: data = %u(0%o) stack = %u(0%o)\n",
			udsize, udsize, ussize, ussize);
/*
 *	The following works only when `make' is compiled
 *	with I&D space separated (i.e. cc -i ...).
 *	(Notice the hard coded `65' below!)
 */
		udsize /= 1000;
		ussize /= 1000;
		printf("mem:");
		for(i=1; i<=udsize;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf("d");
		}
		for(;i<=65-ussize;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf(".");
		}
		for(;i<=65;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf("s");
		}
		printf("\n");
		fflush(stdout);
	}
#endif
}
