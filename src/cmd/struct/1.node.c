
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)1.node.c	3.0	4/22/86";
#include <stdio.h>
#include "def.h"
#include "1.incl.h"

makenode(type,addimp,addcom, labe,arcnum,arctype,arclab)
LOGICAL addimp,addcom;
int type, arctype[], arcnum;
long arclab[], labe;
	{
	int i;
	VERT num;
	
	ASSERT(arcsper[type] < 0 || arcnum == arcsper[type], makenode);
	num = create(type,arcnum);
	
	if (addimp)  fiximp(num,labe);
	
	for (i = 0; i < arcnum; ++i)
		{
		if (arctype[i] == -2)
			addref(arclab[i],&ARC(num,i));
		else
			ARC(num,i) = arctype[i];
		}
	
	
	if (hascom[type] )
		{
		if (!addcom || endcom < begline)
			BEGCOM(num) = UNDEFINED;
		else{
			BEGCOM(num) = begchar - rtnbeg;
			if((unsigned)(BEGCOM(num))!=begchar-rtnbeg)
				faterr("program too long","","");
			}
		}
	return(num);
	}





fiximp(num,labe)		/* fix implicit links, check nesting */
VERT num;
long labe;
	{
	fixvalue(implicit, num);		/* set implicit links to this node */
	clear(implicit);
	if(labe != implicit) fixvalue(labe, num);
	}
