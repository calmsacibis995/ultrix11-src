
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)shell.c	3.0	4/22/86";
shell (n, comp, exch)
	int (*comp)(), (*exch)();
/* SORTS UP.  IF THERE ARE NO EXCHANGES (IEX=0) ON A SWEEP
  THE COMPARISON GAP (IGAP) IS HALVED FOR THE NEXT SWEEP */
{
	int igap, iplusg, iex, i, imax;
	igap=n;
while (igap > 1)
	{
	igap /= 2;
	imax = n-igap;
	do
		{
		iex=0;
		for(i=0; i<imax; i++)
			{
			iplusg = i + igap;
			if ((*comp) (i, iplusg) ) continue;
			(*exch) (i, iplusg);
			iex=1;
			}
		} while (iex>0);
	}
}
