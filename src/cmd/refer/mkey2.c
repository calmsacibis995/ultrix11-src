
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mkey2.c	3.0	4/22/86";
# include "stdio.h"
# include "ctype.h"

	static int eof 0;
	static long lp, lim;
	static int alph, used, prevc;
	static char *p, key[20];

extern int minlen, keycount, labels;
extern char *iglist;
extern int wholefile;

dofile(f, name)
	FILE *f;
	char *name;
{

/* read file f & spit out keys & ptrs */
# define MAXLINE 500
char line[MAXLINE], *s;
int c;
long grec();
alph=used=prevc=eof=0;

lp=0;
if (wholefile==0)
while (lim = grec(line,f))
	{
# if D1
fprintf(stderr, "line: /%s",line);
# endif
	used=alph=0;
	p = key;
	for(s=line; (c= *s) && (used<keycount); s++)
		chkey(c, name);
	lp += lim;
	if (used) putchar('\n');
	}
else
	{
	p=key;
	used=alph=0;
	while ( (c=getc(f)) != EOF && used<keycount)
		chkey (c, name);
	if (used) putchar('\n');
	}
fclose(f);
}

outkey( ky, lead, trail)
	char *ky;
{
	int n;
n = strlen(ky);
if (n<minlen) return (0);
if (n<3)
	{
	if (trail == '.') return(0);
	if (mindex(".%,!#$%&'();+:*", lead)!=0) return(0);
	}
if (isdigit(ky[0]))
	if (ky[0] != '1' || ky[1] != '9' || n!= 4) return(0);
if (common(ky))
	return(0);
return(1);
}
long grec (s, f)
	char *s;
	FILE *f;
{
	char tm[200];
	int curtype 0;
	long len 0L, tlen 0L;
	if (eof) return(0);
	*s = 0;
	while (fgets(tm, 200, f))
		{
		tlen += strlen(tm);
		if (tm[0] == '%' || tm[0] == '.')
			curtype = tm[1];
		if (tlen < MAXLINE && mindex(iglist,curtype)==0)
				strcat(s, tm);
		len = tlen;
		if (wholefile==0 && tm[0] == '\n')
			return(len);
		if (wholefile>0 && len >= MAXLINE)
			{
			fseek (f, 0L, 2);
			return(ftell(f));
			}
		}
	eof=1;
	return(s[0] ? len : 0L);
}
trimnl(ln)
	char *ln;
{
register char *p ln;
while (*p) p++;
p--;
if (*p == '\n') *p=0;
return(ln);
}
chkey (c, name)
	{
	if (isalpha(c) || isdigit(c))
		{
		if (alph++ < 6)
			*p++ = c;
		}
	else
		{
		*p = 0;
		for(p=key; *p; p++)
			*p |= 040;
		if (outkey(p=key,prevc,c))
			{
			if (used==0)
			    {
			    if (labels)
				{
				if (wholefile==0)
					printf("%s:%ld,%ld\t", name, lp, lim);
				else
					printf("%s\t", name);
			    }
				}
			else
				putchar(' ');
			fputs(key, stdout);
			used++;
			}
		prevc=c;
		alph=0;
		}
	}
