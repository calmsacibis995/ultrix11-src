
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mkey3.c	3.0	4/22/86";
# include "stdio.h"
char *comname "/usr/lib/eign";
static int cgate 0;
extern char *comname;
# define COMNUM 500
# define COMTSIZE 997
int comcount 100;
static char cbuf[COMNUM*9];
static char *cwds[COMTSIZE];
static char *cbp;

common (s)
	char *s;
{
if (cgate==0) cominit();
return (c_look(s, 1));
}
cominit()
{
int i;
FILE *f;
cgate=1;
f = fopen(comname, "r");
if (f==NULL) return;
cbp=cbuf;
for(i=0; i<comcount; i++)
	{
	if (fgets(cbp, 15, f)==NULL)
		break;
	trimnl(cbp);
	c_look (cbp, 0);
	while (*cbp++);
	}
fclose(f);
}
c_look (s, fl)
	char *s;
{
int h;
h = hash(s) % (COMTSIZE);
while (cwds[h] != 0)
	{
	if (strcmp(s, cwds[h])==0)
		return(1);
	h = (h+1) % (COMTSIZE);
	}
if (fl==0)
	cwds[h] = s;
return(0);
}
