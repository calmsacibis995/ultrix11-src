
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)gio.c	3.0	4/22/86";

/***************************
 *  front end routines for the "g" packet protocol routines
 */

#define USER 1
#include "pk.p"
#include <sys/types.h>
#include "pk.h"
#include <setjmp.h>
#include "uucp.h"

extern	time_t	time();


jmp_buf Failbuf;

struct pack *Pk;

pkfail()
{
	longjmp(Failbuf, 1);
}

/* initialize protocol data structures and synchronize 
 * source and destination sites.
 */
gturnon()
{
	int ret;
	struct pack *pkopen();
	if (setjmp(Failbuf))
		return(FAIL);
	if (Pkdrvon) { 
	/* Pkdrvon can be set to 1 if the packet driver is not needed */
		ret = pkon(Ofn, PACKSIZE);
		DEBUG(4, "pkon - %d ", ret);
		DEBUG(4, "Ofn - %d\n", Ofn);
		if (ret <= 0)
			return(FAIL);
	}
	else {
		if (Debug > 4)
			pkdebug = 1;
		Pk = pkopen(Ifn, Ofn);
		if ((int) Pk == NULL)
			return(FAIL);
	}
	return(0);
}

/* Free any allocated space, inform destination that
 * circuit is closing down.
 */

gturnoff()
{
	if(setjmp(Failbuf))
		return(FAIL);
	if (Pkdrvon)
		pkoff(Ofn);
	else
		pkclose(Pk);
	return(0);
}


/* transmit the string that str points to */

gwrmsg(type, str, fn)
char type, *str;
{
	char bufr[BUFSIZ], *s;
	int len, i;

	if(setjmp(Failbuf))
		return(FAIL);
	bufr[0] = type;
	s = &bufr[1];
	while (*str)
		*s++ = *str++;
	*s = '\0';
	if (*(--s) == '\n')
		*s = '\0';
	len = strlen(bufr) + 1;
	if ((i = len % PACKSIZE)) {
		len = len + PACKSIZE - i;
		bufr[len - 1] = '\0';
	}
	gwrblk(bufr, len, fn);
	return(0);
}


/* read in a message and place in string pointed to by str */

grdmsg(str, fn)
char *str;
{
	unsigned len;

	if(setjmp(Failbuf))
		return(FAIL);
	for (;;) {
		if (Pkdrvon)
			len = read(fn, str, PACKSIZE);
		else
			len = pkread(Pk, str, PACKSIZE);
		if (len == 0)
			continue;
		str += len;
		if (*(str - 1) == '\0')
			break;
	}
	return(0);
}


/* transmit data in the file that fp1 points to */

gwrdata(fp1, fn)
FILE *fp1;
{
	char bufr[BUFSIZ];
	int len;
	int ret;
	time_t t1, t2;
	long bytes;
	char text[BUFSIZ];

	if(setjmp(Failbuf))
		return(FAIL);
	bytes = 0L;
	Totalrxmts = Totalpackets = 0;
	time(&t1);
	while ((len = fread(bufr, sizeof (char), BUFSIZ, fp1)) > 0) {
		int i = 0;
		bytes += len;
		ret = gwrblk(bufr, len, fn);
		if (ret != len) {
			return(FAIL);
		}
		if (len != BUFSIZ)  /* less than BUFSIZ implies end */
			break;
	}
	ret = gwrblk(bufr, 0, fn);
	time(&t2);
	sprintf(text, "sent %ld b %ld secs, Pk: %d, Rxmt: %d", 
		bytes, t2 - t1, Totalpackets, Totalrxmts);
	DEBUG(1, "%s\n", text);
	syslog(text);
	sysacct(bytes, t2 - t1);
	return(0);
}


/* read in data and place in the file pointed to by fp2 */

grddata(fn, fp2)
FILE *fp2;
{
	int len;
	char bufr[BUFSIZ];
	time_t t1, t2;
	long bytes;
	char text[BUFSIZ];

	if(setjmp(Failbuf))
		return(FAIL);
	bytes = 0L;
	time(&t1);
	for (;;) {
		len = grdblk(bufr, BUFSIZ, fn);
		if (len < 0) {
			return(FAIL);
		}
		bytes += len;
		/* ittvax!swatt: check return value of fwrite */
		if (fwrite(bufr, sizeof (char), len, fp2) != len)
			return(FAIL);
		if (len < BUFSIZ)
			break;
	}
	time(&t2);
	sprintf(text, "received %ld b %ld secs", bytes, t2 - t1);
	DEBUG(1, "%s\n", text);
	syslog(text);
	sysacct(bytes, t2 - t1);
	return(0);
}


/* call ultouch every TC calls to either grdblk or gwrblk -- rti!trt */
#define	TC	20
static	int tc = TC;

grdblk(blk, len,  fn)
char *blk;
{
	int i, ret;

	/* call ultouch occasionally -- rti!trt */
	/* so LCK.files will not be removed precipitously */

	if (--tc < 0) {
		tc = TC;
		ultouch();
	}
	for (i = 0; i < len; i += ret) {
		if (Pkdrvon)
			ret = read(fn, blk, len - i);
		else
			ret = pkread(Pk, blk, len - i);
		if (ret < 0)
			return(FAIL);
		blk += ret;
		if (ret == 0)
			return(i);
	}
	return(i);
}


gwrblk(blk, len, fn)
char *blk;
{
	int ret;

	/* call ultouch occasionally -- rti!trt */
	if (--tc < 0) {
		tc = TC;
		ultouch();
	}
	if (Pkdrvon)
		ret = write(fn, blk, len);
	else
		ret = pkwrite(Pk, blk, len);
	return(ret);
}
