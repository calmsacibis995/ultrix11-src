
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)inv3.c	3.0	4/22/86";
getargs(s, arps)
	char *s, *arps[];
{
	int i;
i = 0;
while (1)
	{
	arps[i++]=s;
	while (*s != 0 && *s!=' '&& *s != '\t')s++;
	if (*s==0) break;
	*s++ =0;
	while (*s==' ' || *s=='\t')s++;
	if (*s==0)break;
	}
return(i);
}
