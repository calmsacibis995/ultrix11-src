
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ctest.c	3.0	4/22/86
 */
#include "stdio.h"
char buf[256];
main()
{	int w,dp,sign;
	char *s;
	double x;
	for(;;)
	{
		scanf("%d %lf",&w,&x);
		if(feof(stdin)) exit(0);
		s=fcvt(x,w,&dp,&sign);
		strcpy(buf,s);
		printf("%d,%f:\t%d\t%s\n",w,x,dp,buf);
		s=ecvt(x,w,&dp,&sign);
		printf("\t\t%d\t%s\n",dp,s);
	}
}
