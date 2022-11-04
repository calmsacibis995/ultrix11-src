
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)dialout.c	3.0	4/22/86";

/* Warning: this dialout() routine has a non-standard argument list */
/* rti!trt: this needs the "getnextfd" trick used in conn.c */
/* decvax!larry - we dont use this routine */

#ifdef	DIALOUT
#include "uucp.h"
#include <sgtty.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <ascii.h>

struct listp {
	char	*acu;
	char	*line;
};

static struct listp dial300[] = {
	{ "/dev/ttyjb", "/dev/ttyjc" },
	0
};
static struct listp dial1200[] = {
	{ "/dev/ttyjb", "/dev/ttyjc" },
	0
};
static struct listp test1200[] = {
	{ "/tmp/dn9", "/dev/ttyjc" },
	0
};
static struct listp cunc[] = {
	{ "/dev/null", "/dev/ttyi8" },
	0
};
static struct listp cduke[] = {
	{ "/dev/null", "/dev/ttyh4" },
	0
};
static struct listp cweb40[] = {
	{ "/dev/null", "/dev/ttyj1" },
	0
};
static struct listp csimon[] = {
	{ "/dev/null", "/dev/tty04" },
	0
};
static struct listp crti[] = {
	{ "/dev/null", "/dev/tty00" },
	0
};

static struct modlist {
	char	*type;
	struct	listp *list;
	char speed;
} modlist[] = {
	{ "1200", dial1200, B1200},
	{ "T1200", test1200, B1200},
	{ "300", dial300, B300},
	{ "U4800", cunc, B4800},
	{ "D4800", cduke, B4800},
	{ "W4800", cweb40, B4800},
	{ "S4800", csimon, B4800},
	{ "R1200", crti, B1200},
	0
};

static int acu = -1;
static int dh = -1;

dialout(telno, flds)
char *telno, *flds[];
{
	extern errno;
	register char *p;
	char digits[30];
	char *dev;
	char *type;
	register int d, m;
	int r, pid, retval;
	int sigalrm(), (*sal)();
	struct sgttyb vec;
	char rvc;

	type = flds[F_CLASS];
	DEBUG(6, "telno=%s ", telno);
	DEBUG(6, " type=%s\n", type);
	digits[0] = STX;
	for (p = digits+1; *telno; telno++)
		if (isdigit(*telno) || *telno == ';') *p++ = *telno;
		else if (*telno == '-') *p++ = ':';
		else if (*telno == '<' || *telno == '#') break;
		else if (*telno == '*') ;
		else return(-9);
	*p++ = '?';
	*p++ = ETX;
	*p++ = '\0';
	sal = signal(SIGALRM, sigalrm);
	for (m=0; modlist[m].type; m++) {
		if (strcmp(modlist[m].type, type)==0)
			goto linefound;
	}
	retval = -3;		/* unknown type */
	goto ret;

linefound:
	retval = -1;		/* All lines busy */
	for(d=1; d<7; d++, sleep(6)) {
		dev = modlist[m].list[d-1].line;
		if (dev==0)
			break;
		/*
			if (access(modlist[m].list[d-1].acu, 2)) {
				retval = -5;
				continue;
			}
			if (access(dev, 6)) {
				retval = -6;
				continue;
			}
		*/
		DEBUG(6, "ACU=%s ", modlist[m].list[d-1].acu);
		DEBUG(6, "line=%s\n", dev);
		acu = open(modlist[m].list[d-1].acu, 2);
		if (acu < 0) acu = open(modlist[m].list[d-1].acu, 1);
		if (acu < 0) {
			if (errno != EBUSY) retval = -5;
			continue;
		}

		retval = -2;	/* Hmm, found a line */
		ioctl(acu, TIOCHPCL, 0);
		ioctl(acu, TIOCEXCL, 0);
		vec.sg_ispeed = B1200;
		vec.sg_ospeed = B1200;
		vec.sg_flags = RAW;
		ioctl(acu, TIOCSETP, &vec);
		if ((pid=fork())==0) {
			close(acu);
			dh = open(dev, 2);
			if (dh >= 0) ioctl(dh, TIOCHPCL, 0);
			for(;;)
				pause();
		}
		else if (pid < 0) {
			close(acu);
			retval = -4;
			continue;
		}
		alarm(45 > 2*strlen(digits) ? 45 : 2*strlen(digits));
		DEBUG(6, "len=%d\n", strlen(digits));
		r = write(acu, digits, strlen(digits));
		alarm(0);
		if (r != strlen(digits)) {
			close(acu);
			kill(pid, 9);
			wait(0);
			continue;
		}
		DEBUG(6, "write done\n",0);
		alarm(65);
		r = read(acu, &rvc, 1);
		alarm(0);
		if (r == 1) {
			DEBUG(6, "r=%d ", r);
			DEBUG(6, "char='%c'\n",rvc);
			if (rvc != 'C') {
				close(acu);
				kill(pid, 9);
				wait(0);
				switch(rvc) {
					case 'A': retval = -2; break;
					case 'B': retval = -8; break;
					case 'D': retval = -7; break;
					case 'E': retval = -9; break;
					default: retval = -9;
				}
				continue;
			}
		}
		else 
		{
			DEBUG(6, "r=%d\n", r);
		}
		alarm(10);
		dh = open(dev, 2);
		alarm(0);
		kill(pid, 9);
		wait(0);
		if (dh>=0) {
			ioctl(dh, TIOCGETP, &vec);
			vec.sg_ispeed = vec.sg_ospeed = modlist[m].speed;
			vec.sg_flags &= ~ECHO;
			vec.sg_flags |= RAW|EVENP|ODDP;
			ioctl(dh, TIOCSETP, &vec);
			ioctl(dh, TIOCHPCL, 0);
			ioctl(dh, TIOCEXCL, 0);
			retval = dh;
			goto ret;
		}
		if (errno != EBUSY) retval = -6;
		close(acu);
	}
ret:
	signal(SIGALRM, sal);
	DEBUG(6, "retval=%d\n", retval);
	if (retval < 0) acu = -1;
	return(retval);
}

dialend()
{
	if (acu >= 0) {
		close(acu);
		acu = -1;
	}
	if (dh >= 0) {
		close(dh);
		dh = -1;
	}
}

sigalrm()
{
	signal(SIGALRM, sigalrm);
	return;
}
#endif
