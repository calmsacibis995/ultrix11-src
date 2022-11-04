
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ctime.c	3.2	8/7/87
 */
/*
 * This routine converts time as follows.
 * The epoch is 0000 Jan 1 1970 GMT.
 * The argument time is in seconds since then.
 * The localtime(t) entry returns a pointer to an array
 * containing
 *  seconds (0-59)
 *  minutes (0-59)
 *  hours (0-23)
 *  day of month (1-31)
 *  month (0-11)
 *  year-1970
 *  weekday (0-6, Sun is 0)
 *  day of the year
 *  daylight savings flag
 *
 * The routine calls the system to determine the local
 * timezone and whether Daylight Saving Time is permitted locally.
 * (DST is then determined by the current US standard rules)
 * There is a table that accounts for the peculiarities
 * undergone by daylight time in 1974-1975.
 *
 * The routine does not work
 * in Saudi Arabia which runs on Solar time.
 *
 * asctime(tvec))
 * where tvec is produced by localtime
 * returns a ptr to a character string
 * that has the ascii time in the form
 *	Thu Jan 01 00:00:00 1970n0\\
 *	01234567890123456789012345
 *	0	  1	    2
 *
 * ctime(t) just calls localtime, then asctime.
 */

#include <sys/types.h>
#include <sys/timeb.h> 
#include <sys/time.h>

static	char	cbuf[26];
static	int	dmsize[12] =
{
	31,
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31
};

/*
 * The following table is used for 1974 and 1975 and
 * gives the day number of the first day after the Sunday of the
 * change.  Please note that these are 0 origin days.
 */
struct dstab {
	int	dayyr;
	int	daylb;
	int	dayle;
};

static struct dstab usdaytab[] = {
	1974,	5,	333,	/* 1974: Jan 6 - last Sun. in Nov */
	1975,	58,	303,	/* 1975: Last Sun. in Feb - last Sun in Oct */
	1976,	119,	303,	/* 1976: end Apr - end Oct */
	1977,	119,	303,	/* 1977: end Apr - end Oct */
	1978,	119,	303,	/* 1978: end Apr - end Oct */
	1979,	119,	303,	/* 1979: end Apr - end Oct */
	1980,	119,	303,	/* 1980: end Apr - end Oct */
	1981,	119,	303,	/* 1981: end Apr - end Oct */
	1982,	119,	303,	/* 1982: end Apr - end Oct */
	1983,	119,	303,	/* 1983: end Apr - end Oct */
	1984,	119,	303,	/* 1984: end Apr - end Oct */
	1985,	119,	303,	/* 1985: end Apr - end Oct */
	1986,	119,	303,	/* 1986: end Apr - end Oct */
	0,	96,	303,	/* all other years: beg Apr - end Oct */
};
static struct dstab ausdaytab[] = {
	1970,	400,	0,	/* 1970: no daylight saving at all */
	1971,	303,	0,	/* 1971: daylight saving from Oct 31 */
	1972,	303,	58,	/* 1972: Jan 1 -> Feb 27 & Oct 31 -> dec 31 */
	0,	303,	65,	/* others: -> Mar 7, Oct 31 -> */
};

/*
 * The European tables ... based on hearsay
 * Believed correct for:
 *	WE:	Great Britain, Ireland, Portugal
 *	ME:	Belgium, Luxembourg, Netherlands, Denmark, Norway,
 *		Austria, Poland, Czechoslovakia, Sweden, Switzerland,
 *		DDR, DBR, France, Spain, Hungary, Italy, Jugoslavia
 * Eastern European dst is unknown, we'll make it ME until someone speaks up.
 *	EE:	Bulgaria, Finland, Greece, Rumania, Turkey, Western Russia
 */
static struct dstab wedaytab[] = {
	1983,	86,	303,	/* 1983: end March - end Oct */
	1984,	86,	303,	/* 1984: end March - end Oct */
	1985,	86,	303,	/* 1985: end March - end Oct */
	0,	86,	303,	/* others: end March - end Oct */
};

static struct dstab medaytab[] = {
	1983,	86,	272,	/* 1983: end March - end Sep */
	1984,	86,	272,	/* 1984: end March - end Sep */
	1985,	86,	272,	/* 1985: end March - end Sep */
	0,	86,	272,	/* others: saving end March - end Sep */
};

static struct dayrules {
	int		dst_type;	/* number obtained from system */
	int		dst_hrs;	/* hours to add when dst on */
	struct	dstab *	dst_rules;	/* one of the above */
	enum {STH,NTH}	dst_hemi;	/* southern, northern hemisphere */
} dayrules [] = {
	DST_USA,	1,	usdaytab,	NTH,
	DST_AUST,	1,	ausdaytab,	STH,
	DST_WET,	1,	wedaytab,	NTH,
	DST_MET,	1,	medaytab,	NTH,
	DST_EET,	1,	medaytab,	NTH,	/* XXX */
	-1,
};

#define NULL_DAYRULES ((struct dayrules *)0)
struct tm	*gmtime();
char		*ct_numb();
struct tm	*localtime();
char	*ctime();
char	*ct_num();
char	*asctime();

dysize(y)
{
	if((y%4) == 0)
		return(366);
	return(365);
}


char *
ctime(t)
long *t;
{
	return(asctime(localtime(t)));
}

struct tm *
localtime(gmt_value)
	time_t *gmt_value;		/* Greenwich Mean Time value */
{
	register struct tm *ct;
	register struct dayrules *dr;
	time_t local_time;

	/* We assume that the timezone will not change across calls to
	 * this routine, so the seconds west of Greenwich and the daylight
	 * savings time rules for the time zone can be saved across calls.
	 */
	static time_t seconds_west;
	static struct dayrules *saved_dr = NULL_DAYRULES;

	/* Get the pointer to the saved daylight savings time rules.  If it
	 * is NULL then this is the first time through then we have to set up 
	 * the static information about this timezone.
	 */
	dr = saved_dr;
	if (dr == NULL_DAYRULES){
		struct timeb systime;

		/* Get the time zone information.  Not interested in the
		 * timeofday.
		 */
		ftime(&systime);
		
		/* Calculate the seconds west of Greewich Mean Time for
		 * this time zone.
		 */
		seconds_west =  (long)systime.timezone*60;

		/* Get the pointer to the daylight savings rules for the
		 * time zone, and save the pointer for the next call.
		 */
		dr = dayrules ; 
		while (dr->dst_type >= 0 && dr->dst_type != systime.dstflag)
			dr++;
		saved_dr = dr;
	}

	local_time = *gmt_value - seconds_west;
	ct = gmtime(&local_time);

	if (dr->dst_type >= 0) {
	
		register int dayno = ct->tm_yday;
		register daylbeg, daylend;
		register struct dstab *ds = dr->dst_rules;
		int year = ct->tm_year + 1900;

		while (ds->dayyr && ds->dayyr != year)
			ds++;

		/* Calculate first Sunday after DST begins and first
		 * Sunday after it ends.
		 */
		daylbeg = sunday(ct,ds->daylb);
		daylend = sunday(ct,ds->dayle);

		switch (dr->dst_hemi) {
		case NTH:
		    if (!(
		       (dayno>daylbeg || (dayno==daylbeg && ct->tm_hour>=2)) &&
		       (dayno<daylend || (dayno==daylend && ct->tm_hour<1))
		    ))
			    return(ct);
		    break;
		case STH:
		    if (!(
		       (dayno>daylbeg || (dayno==daylbeg && ct->tm_hour>=2)) ||
		       (dayno<daylend || (dayno==daylend && ct->tm_hour<2))
		    ))
			    return(ct);
		    break;
		default:
		    return(ct);
		}
	        local_time += dr->dst_hrs*60*60;
		ct = gmtime(&local_time);
		ct->tm_isdst++;
	}
	return(ct);
}

/*
 * The argument is a 0-origin day number.
 * The value is the day number of the first
 * Sunday on or after the day.
 */
static
sunday(t, d)
register struct tm *t;
register int d;
{
	if (d >= 58)
		d += dysize(t->tm_year) - 365;
	return(d - (d - t->tm_yday + t->tm_wday + 700) % 7);
}

struct tm *
gmtime(tim)
long *tim;
{
	register int d0, d1;
	long hms, day;
	register int *tp;
	static struct tm xtime;

	/*
	 * break initial number into days
	 */
	hms = *tim % 86400;
	day = *tim / 86400;
	if (hms<0) {
		hms += 86400;
		day -= 1;
	}
	tp = (int *)&xtime;

	/*
	 * generate hours:minutes:seconds
	 */
	*tp++ = hms%60;
	d1 = hms/60;
	*tp++ = d1%60;
	d1 /= 60;
	*tp++ = d1;

	/*
	 * day is the day number.
	 * generate day of the week.
	 * The addend is 4 mod 7 (1/1/1970 was Thursday)
	 */

	xtime.tm_wday = (day+7340036)%7;

	/*
	 * year number
	 */
	if (day>=0) for(d1=70; day >= dysize(d1); d1++)
		day -= dysize(d1);
	else for (d1=70; day<0; d1--)
		day += dysize(d1-1);
	xtime.tm_year = d1;
	xtime.tm_yday = d0 = day;

	/*
	 * generate month
	 */

	if (dysize(d1)==366)
		dmsize[1] = 29;
	for(d1=0; d0 >= dmsize[d1]; d1++)
		d0 -= dmsize[d1];
	dmsize[1] = 28;
	*tp++ = d0+1;
	*tp++ = d1;
	xtime.tm_isdst = 0;
	return(&xtime);
}

char *
asctime(t)
struct tm *t;
{
	register char *cp, *ncp;
	register int *tp;

	cp = cbuf;
	for (ncp = "Day Mon 00 00:00:00 1900\n"; *cp++ = *ncp++;);
	ncp = &"SunMonTueWedThuFriSat"[3*t->tm_wday];
	cp = cbuf;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp++;
	tp = &t->tm_mon;
	ncp = &"JanFebMarAprMayJunJulAugSepOctNovDec"[(*tp)*3];
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	cp = ct_numb(cp, *--tp);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	cp = ct_numb(cp, *--tp+100);
	if (t->tm_year>=100) {
		cp++;
		*cp++ = '2';
		if (t->tm_year >= 200) {
			*cp = '1';
			t->tm_year -= 200;
		} else {
			*cp = '0';
			t->tm_year -= 100;
		}
	} else {
		cp += 2;
	}
	cp = ct_numb(cp, t->tm_year+100);
	return(cbuf);
}

static char *
ct_numb(cp, n)
register char *cp;
{
	cp++;
	if (n>=10)
		*cp++ = (n/10)%10 + '0';
	else
		*cp++ = ' ';
	*cp++ = n%10 + '0';
	return(cp);
}
