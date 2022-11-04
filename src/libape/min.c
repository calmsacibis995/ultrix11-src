
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)min.c	3.0	4/22/86	*/
/*	(2.9BSD)  min.c	2.1	8/13/82 */

#include "lint.h"
#include <ctype.h>
#include <stdio.h>
#include <ape.h>

m_in(a,b,f)  /* input a number base b from file f into a */
MINT *a; FILE *f;
{	MINT x,y,ten;
	int sign;
	register int c;
	short qten,qy;

	if (b<2) aperror("m_in: bad input base");
	xfree(a);
	sign=1;
	ten.len=1;
	ten.val= &qten;
	qten=b;
	x.len=0;
	y.len=1;
	y.val= &qy;
	while ((c=getc(f)) != EOF && (c == '\n' || c == ' '
			|| c == '\t')) ; /* skip over white space to number */
	if (c == '+' || c == '-')	/* catch sign */
		sign = (c = '+') ? 1 : -1;
	else
		ignore(ungetc (c,f));
	while ((c=getc(f)) != EOF)
	switch(c)
	{
	case '\\':
		c = getc(f); /* should be '\n'; more number follows */
		if (c != '\n') ignore(ungetc(c,f));
		continue;
	case '\t':
	case '\n': a->len *= sign;
		xfree(&x);
		return(0); /* tabs or newlines will end the number;
				the first one is eaten */
	case ' ':
		continue; /* spaces allowed in middle of big numbers
				for the sake of clarity */
	default: if (isdigit(c))
		{	qy=c-'0';
			mult(&x,&ten,a);
			madd(a,&y,a);
			move(a,&x);
			continue;
		}
		else if (isalpha(c))
		{
			if (((c == 'x') || (c == 'X')) && (b == 16))
				continue; /* allow 0x as hex marker */
			qy = c + 10 - ((c >= 'a') ? 'a' : 'A');
				/* ascii only! */
			mult(&x,&ten,a);
			madd(a,&y,a);
			move(a,&x);
			continue;
		}
		else
		{	ignore(ungetc(c,f));
			a->len *= sign;
			xfree(&x);
			return(0);
		}
	}
	xfree(&x);
	return(EOF);
}

sm_in(a,b,s)	/* like m_in, but for a number represented
		 * as a character string s.
		 * Spaces are allowed only in longer numbers--
		 * otherwise, any non-digits
		 * other than initial white space or signs
		 * (or letters for bases > 10) cause termination
		 * of input at that point */
PMINT a;
char *s;
{
	int sign=1, length;
	long int ln;
	MINT x,y,ten;
	short qten,qy;


	if (b<2) aperror("m_in: bad input base");
	xfree(a);

	/* First check to see whether ltom (or itom) would
	 * be adequate for input.  The magic numbers used
	 * here depend on long ints being 32 bits. */

	length = strlen(s);
		/* do the most important cases the easy way: */
	if ((b==10 && length < 10) ||
		(b==8 && length <=10) ||
			(b==16 && length <=7))
		{
		simpleconvert(a,b,s);
		return;
		}
	if (b <10 && length < (18-b) )
		/* a crude estimate, to be sure! */
		{
		/* skip intitial white space, if any */
		while (*s == ' ' || *s == '\n' || *s == '\t') ++s;
		ln=0L;
		if (*s == '+' || *s == '-')	/* catch sign */
			sign = (*s++ = '+') ? 1 : -1;
		while ( *s >= '0' && *s < b+'0')
			{
			ln *= b;
			ln += (*s++ - '0');
			}
		ln *= sign;
		makemint(a,ln);
		return;
		}
		/* For bases between 10 and 16,
		 * allow for letter digits: */
	if (b < 16 && length <= 7)
		{
		/* skip intitial white space, if any */
		while (*s == ' ' || *s == '\n' || *s == '\t') ++s;
		ln=0L;
		if (*s == '+' || *s == '-')	/* catch sign */
			sign = (*s++ = '+') ? 1 : -1;
		while (s != '\0')
			{
			ln *= b;
			if (isdigit(*s))
				{
				ln += (*s++ - '0');
				}
			/* This works for sensible alphabets only! */
			else if ( (*s >= 'a' && *s <= 'f') ||
				  (*s >= 'A' && *s <= 'F') ) 
				{
				ln += *s + 10 - ( (*s >= 'a') ? 'a' : 'A');
				++s;
				}
			else break;
			}
		ln *= sign;
		makemint(a,ln);
		return;
		}

		/* Otherwise we have to use the ape
		 * routines to do the mults and adds: */

	ten.len=1;
	ten.val= &qten;
	qten=b;
	x.len=0;
	y.len=1;
	y.val= &qy;
	while (*s != '\0' && (*s == '\n' || *s == ' '
			|| *s == '\t')) ++s;
		/* skip over white space to number */
	if (*s == '+' || *s == '-')	/* catch sign */
		sign = (*s++ = '+') ? 1 : -1;
	for(; *s != '\0'; ++s)
		if (*s ==  ' ')
			continue; /* spaces allowed in middle of big numbers
					for the sake of clarity */
		else if (isdigit(*s))
			{	qy = *s-'0';
				mult(&x,&ten,a);
				madd(a,&y,a);
				move(a,&x);
				continue;
			}
		else if (isalpha(*s))
			{
				if (((*s == 'x') || (*s == 'X')) && (b == 16))
					continue; /* allow 0x as hex marker */
				qy = *s + 10 -  ((*s >= 'a') ? 'a' : 'A');
					/* ascii only! */
				mult(&x,&ten,a);
				madd(a,&y,a);
				move(a,&x);
				continue;
			}
		else
			{
			a->len *= sign;
			xfree(&x);
			return;
			}
	xfree(&x);
	return;
}

simpleconvert(a,b,s)
PMINT a;
int b;
char *s;
{
	long int ln;
	int ok;

	switch(b)
		{
		case(10) : ok = sscanf(s,"%ld",&ln); break;
		case(8)  : ok = sscanf(s,"%lo",&ln); break;
		case(16) : ok = sscanf(s,"%lx",&ln); break;
		default  : fprintf(stderr, "Not so simple!\n"); ok=0; break;
		}
	if (ok)
		makemint(a,ln);
	else
		fprintf(stderr, "Can\'t convert %s\n",s);
	return;
}
