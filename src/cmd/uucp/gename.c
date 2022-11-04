
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)gename.c	3.0	4/22/86";

/* ASSERT mod Sep 3 -- rti!trt */
/*
 * GENAME.C - as obtained from ittvax!watt, and modified by duke!dbl
 * 7/30/82
 *
 * decvax!larry - no changes since  BSD 4.2
 */

#include "uucp.h"
#include <sys/types.h>


#define TSSEQ
#define SEQHUNK 10
#define BASE 62


/*******
 *	gename(pre, sys, grade, file)	generate file name
 *	char grade, *sys, pre, *file;
 *
 *	return codes:  none
 */

gename(pre, sys, grade, file)
char pre, *sys, grade, *file;
{
	char sqnum[5];

	getseq(sqnum);
	sprintf(file, "%c.%.7s%c%.4s", pre, sys, grade, sqnum);
	DEBUG(4, "file - %s\n", file);
	return;
}


#define SLOCKTIME 15
#define SLOCKTRIES 15
#define SEQLEN 4

/*******
 *	getseq(snum)	get next sequence number
 *	char *snum;
 *
 *	return codes:  none
 *
 * Fri Jan 15 15:34:02 EST 1982 ittvax!swatt:
 * if "TSSEQ" is defined, use "ts" routines to keep sequence numbers
 * in either base 36 or 62.
 * if "SEQHUNK" is defined, then sequence numbers are allocated in
 * hunks of that size.  Using SEQHUNK on very busy systems is not
 * advisable unless TSSEQ is also enabled.
 *
 * Also fix so wraparound is correct; the old code did 9999=>1000
 *
 * The hunk idea saves a lot of filesystem activity.  To get a
 * new sequence number from the sequnce file invovles:
 *
 *  1)	Lock the file (creat + link)
 *  2)	Open file
 *  3)	Read from file
 *  4)	Reopen file for writing
 *  6)	Write to file
 *  7)  Unlock file (unlink)
 */

#ifndef SEQHUNK
# define SEQHUNK 1
#endif !SEQHUNK

#ifdef TSSEQ
 typedef long ts_t;
 ts_t	atots();
 char	*tstoa();
#else !TSSEQ
 typedef int ts_t;
#endif TSSEQ


getseq(snum)
char *snum;
{
	FILE *fp;
	char tseqbuf[64];
	static ts_t n;
	static int nseq = 0;

	if (nseq <= 0) {
		for (n = 0; n < SLOCKTRIES; n++) {
			if (!ulockf( Seqlock, (time_t)SLOCKTIME))
				break;
			sleep(5);
		}

		ASSERT(n < SLOCKTRIES, "CAN NOT GET", Seqlock, 0);

		/* @@@(ittvax!swatt):
		 * can save something by using "r+" on fopen
		 */
		if ((fp = fopen(Seqfile, "r")) != NULL) {
#ifdef TSSEQ
			fgets (tseqbuf, sizeof tseqbuf, fp);
			n = atots (tseqbuf);
#else !TSSEQ
			/* read sequence number file */
			fscanf(fp, "%4d", &n);
#endif TSSEQ
			fp = freopen(Seqfile, "w", fp);
			ASSERT(fp != NULL, "CAN NOT OPEN", Seqfile, 0);
		}
		else {
			/* can not read file - create a new one */
			if ((fp = fopen(Seqfile, "w")) == NULL)
				/* can not write new seqeunce file */
				return(FAIL);
			n = 0;
		}
#ifdef TSSEQ
		tstoa (n+SEQHUNK, tseqbuf, SEQLEN);
#else !TSSEQ
		sprintf (tseqbuf, "%04d", n+SEQHUNK);
#endif TSSEQ
		/* discard high order digits on overflow */
		while (strlen (tseqbuf) > SEQLEN)
			strcpy (tseqbuf, tseqbuf+1);
		fprintf (fp, "%s", tseqbuf);
		fclose (fp);
		rmlock (Seqlock);
		nseq = SEQHUNK;
	}

#ifdef TSSEQ
	/* Convert n to base 36 (or base 62) digit string */
	tstoa (n, tseqbuf, SEQLEN);
#else !TSSEQ
	sprintf (tseqbuf, "%04d", n);
#endif TSSEQ
	/* discard high-order digits on overflow */
	while (strlen (tseqbuf) > SEQLEN)
		strcpy (tseqbuf, tseqbuf+1);
	strcpy (snum, tseqbuf);
	++n;
	--nseq;
	return(0);
}

#ifdef TSSEQ
/*********************************************************************
function:	ts
description:	Convert between binary integers and base thirty-six
		strings (hence the name "ts")
programmer:	Alan S. Watt

history:
	01/14/82	original version
*********************************************************************/


/* Alphanumeric string-to-integer conversion routines */

/* BASE is either 36 or 62
 * Base 36 allows magnitudes from 0 to 1679615
 * Base 62 allows magnitudes from 0 to 14776365
 */
#ifndef BASE
# define BASE	62
#endif BASE
#define EOS	'\0'
static char tsdigits[]	=
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#include <ctype.h>

/* Ordinal values for characters
 * Change for non-ASCII
 */
#define DIGORD(c)	(c-'0')
#define UCORD(c)	(c-'A')
#define LCORD(c)	(c-'a')


/* Convert TS string to binary integer */
ts_t
atots (str)
register char *str;
{

	register ts_t ret;
	register digit;

	for (ret = 0; *str != EOS; str++) {
		if (isupper (*str))
			digit = UCORD(*str) + 10;
		else if (islower (*str))
			digit = LCORD(*str) + 10 + (BASE-36);
		else if (isdigit (*str))
			digit = DIGORD(*str);
		else
			break;
		ret = ret * BASE + digit;
	}
	return (ret);
}

/* Convert binary integer to TS string.
 * String is left-padded with zeros up to the
 * width given by 'prec'.  Truncation does not
 * take place here; calling routine must do it
 * if required.
 */
char *
tstoa (num, buf, prec)
register ts_t num;
register char *buf;
int prec;
{
	register ts_t quot = num / BASE;

	if (--prec > 0 || quot != 0)
		buf = tstoa (quot, buf, prec);
	*buf++ = tsdigits[num % BASE];
	*buf = EOS;
	return (buf);
}
#endif TSSEQ
