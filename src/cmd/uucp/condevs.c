
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


#ifndef lint
static char sccsid[] = "@(#)condevs.c	3.0	4/22/86";
#endif

/*
 * Here are various dialers to establish the machine-machine connection.
 * conn.c/condevs.c was glued together by Mike Mitchell.
 * The dialers were supplied by many people, to whom we are grateful.
 *
 * ---------------------------------------------------------------------
 * NOTE:
 * There is a bug that occurs at least on PDP11s due to a limitation of
 * setjmp/longjmp.   If the routine that does a setjmp is interrupted
 * and longjmp-ed to,  it loses its register variables (on a pdp11).
 * What works is if the routine that does the setjmp
 * calls a routine and it is the *subroutine* that is interrupted.
 * 
 * Anyway, in conclusion, condevs.c is plagued with register variables
 * that are used inside
 * 	if (setjmp(...)) {
 * 		....
 * 	}
 * 
 * THE FIX: In dnopn(), for example, delete the 'register' Devices *dev.
 * (That was causing a core dump; deleting register fixed it.)
 * Also for dnopn delete 'register' int dnf... .
 * In pkopn, delete 'register' flds... .
 * There may be others, especially mcm's version of hysopen.
 * You could just delete all references to register, that is safest.
 * This problem might not occur on 4.1bsd, I am not sure.
 * 	Tom Truscott
 *
 * decvax!larry - revamped df03/df02 code , should run on ULTRIX/11.
 * 
 *		- cleaned up hayes smartmodem code,  resets before trying,
 *			converts dialer characters to standard uucp dialing
 *			characters.
 *
 *		- split out external definitions for dialing routines.  Should
 *		   be possible to add new dialer code without recompiling other
 *	 	   source modules (except condefs.c).
 *	
 *		- only df02/03, hayes, and direct connect code 
 *		      has been tested significantly.
 *
 */




#include <sys/types.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <sgtty.h>
#include "uucp.h"
#include <sys/file.h>

extern int Dcf;	/* so clsacu will work */
extern char devSel[];	/* name to pass to delock() in close */
extern int errno, next_fd;
extern jmp_buf Sjbuf;
extern int alarmtr();
int nulldev(), nodev(), Acuopn(), diropn(), dircls();


/***
 *	nulldev		a null device (returns CF_DIAL)
 */
int nulldev()
{
	return(CF_DIAL);
}

/***
 *	nodev		a null device (returns CF_NODEV)
 */
int nodev()
{
	return(CF_NODEV);
}


/*
 * The first things in this file are the generic devices. 
 * Generic devices look through L-devices and call the CU_open routines for
 * appropriate devices.  Some things, like the Unet interface, or direct
 * connect, do not use the CU_open entry.  ACUs must search to find the'
 * right routine to call.
 */

/***
 *	diropn(flds)	connect to hardware line
 *	char *flds[];
 *
 *	return codes:
 *		>0  -  file number  -  ok
 *		FAIL  -  failed
 */

diropn(flds)
register char *flds[];
{
	register int dcr, status;
	int ret;
	struct Devices dev;
	char dcname[20];
	char msg[50];
	FILE *dfp;
	dfp = fopen(DEVFILE, "r");
	ASSERT(dfp != NULL, "CAN'T OPEN", DEVFILE, 0);
nextd:	while ((status = rddev(dfp, &dev)) != FAIL) {
		if (strcmp(flds[F_CLASS], dev.D_class) != SAME)
			continue;
		if (strcmp(flds[F_PHONE], dev.D_line) != SAME)
			continue;
		if (mlock(dev.D_line) != FAIL)
			break;
	}
	if (status == FAIL) {
		fclose(dfp);
		return(CF_NODEV);
	}

	sprintf(dcname, "/dev/%s", dev.D_line);
	if (setjmp(Sjbuf)) {
		delock(dev.D_line);
		fclose(dfp);
		return(FAIL);
	}
	signal(SIGALRM, alarmtr);
	alarm(10);
	getnextfd();
	errno = 0;
#ifdef ONDELAY
	dcr = open(dcname, O_RDWR|O_NDELAY); /* read/write */
#else
	dcr = open(dcname, 2); /* read/write */
#endif
	next_fd = -1;
	if (dcr < 0 && errno == EACCES)
		logent(dcname, "CAN'T OPEN");
	if (dcr < 0 && errno == EBUSY) {
		logent(dcname, "Direct line already in use");
		goto nextd;
	}
	fclose(dfp);
	alarm(0);
	if (dcr < 0) {
		delock(dev.D_line);
		return(FAIL);
	}
#ifdef ULTRIX
	{
	int pgrp = getpgrp(0);
	int temp = 0;
	/* ensure correct process group if run in background by cron */
	ioctl(dcr, TIOCSPGRP, &pgrp);
#ifdef ONDELAY
	/* ignore modem signals - reset to default mode on last close */
	ret = ioctl(dcr, TIOCNMODEM, &temp);
	if (ret < 0)
		DEBUG(6,"ioctl(TIOCNMODEM), errno=%d\n", errno);
#endif
	}
#endif
	fflush(stdout);
	fixline(dcr, dev.D_speed);
	strcpy(devSel, dev.D_line);	/* for latter unlock */
	sprintf(msg, "%s, fd= %d",devSel, dcr);
	logent(msg, "using device");
	CU_end = dircls;
	return(dcr);
}

dircls(fd)
register int fd;
{
	if (fd > 0) {
		close(fd);
		delock(devSel);
		}
	}

#ifdef DATAKIT

#include <dk.h>
#define DKTRIES 2

/***
 *	dkopn(flds)	make datakit connection
 *
 *	return codes:
 *		>0 - file number - ok
 *		FAIL - failed
 */

dkopn(flds)
char *flds[];
{
	int dkphone;
	register char *cp;
	register ret, i;

	if (setjmp(Sjbuf))
		return(FAIL);

	signal(SIGALRM, alarmtr);
	dkphone = 0;
	cp = flds[F_PHONE];
	while(*cp)
		dkphone = 10 * dkphone + (*cp++ - '0');
	DEBUG(4, "dkphone (%d) ", dkphone);
	for (i = 0; i < DKTRIES; i++) {
		getnextfd();
		ret = dkdial(D_SH, dkphone, 0);
		next_fd = -1;
		DEBUG(4, "dkdial (%d)\n", ret);
		if (ret > -1)
			break;
	}
	return(ret);
}
#endif

#ifdef PNET
/***
 *	pnetopn(flds)
 *
 *	call remote machine via Purdue network
 *	use dial string as host name, speed as socket number
 * Author: Steve Bellovin
 */

pnetopn(flds)
char *flds[];
{
	int fd;
	int socket;
	register char *cp;

	fd = pnetfile();
	DEBUG(4, "pnet fd - %d\n", fd);
	if (fd < 0) {
		logent("AVAILABLE DEVICE", "NO");
		return(CF_NODEV);
	}
	socket = 0;
	for (cp = flds[F_CLASS]; *cp; cp++)
		socket = 10*socket + (*cp - '0');
	DEBUG(4, "socket - %d\n", socket);
	if (setjmp(Sjbuf)) {
		DEBUG(4, "pnet timeout  - %s\n", flds[F_PHONE]);
		return(FAIL);
	}
	signal(SIGALRM, alarmtr);
	DEBUG(4, "host - %s\n", flds[F_PHONE]);
	alarm(15);
	if (pnetscon(fd, flds[F_PHONE], socket) < 0) {
		DEBUG(4, "pnet connect failed - %s\n", flds[F_PHONE]);
		return(FAIL);
	}
	alarm(0);
	return(fd);
}
#endif	PNET

#ifdef UNET
/***
 *	unetopn -- make UNET (tcp-ip) connection
 *
 *	return codes:
 *		>0 - file number - ok
 *		FAIL - failed
 */

/* Default port of uucico server */
#define	DFLTPORT	33

unetopn(flds)
register char *flds[];
{
	register int ret, port;
	int unetcls();

	port = atoi(flds[F_PHONE]);
	if (port <= 0 || port > 255)
		port = DFLTPORT;
	DEBUG(4, "unetopn host %s, ", flds[F_NAME]);
	DEBUG(4, "port %d\n", port);
	if (setjmp(Sjbuf)) {
		logent("tcpopen", "TIMEOUT");
		endhnent();	/* see below */
		return(CF_DIAL);
	}
	signal(SIGALRM, alarmtr);
	alarm(30);
	ret = tcpopen(flds[F_NAME], port, 0, TO_ACTIVE, "rw");
	alarm(0);
	endhnent();	/* wave magic wand at 3com and incant "eat it, bruce" */
	if (ret < 0) {
		DEBUG(5, "tcpopen failed: errno %d\n", errno);
		logent("tcpopen", "FAILED");
		return(CF_DIAL);
	}
	CU_end = unetcls;
	return(ret);
}

/*
 * unetcls -- close UNET connection.
 */
unetcls(fd)
register int fd;
{
	DEBUG(4, "UNET CLOSE called\n", 0);
	if (fd > 0) {
		/* disable this until a timeout is put in
		if (ioctl(fd, UIOCCLOSE, STBNULL))
			logent("UNET CLOSE", "FAILED");
		 */
		close(fd);
		DEBUG(4, "closed fd %d\n", fd);
	}
}
#endif UNET

#ifdef MICOM

/*
 *	micopn: establish connection through a micom.
 *	Returns descriptor open to tty for reading and writing.
 *	Negative values (-1...-7) denote errors in connmsg.
 *	Be sure to disconnect tty when done, via HUPCL or stty 0.
 */
micopn(flds)
register char *flds[];
{
	extern errno;
	char *rindex(), *fdig(), dcname[20];
	int dh, ok = 0, speed;
	register struct condev *cd;
	register FILE *dfp;
	struct Devices dev;

	dfp = fopen(DEVFILE, "r");
	ASSERT(dfp != NULL, "Can't open", DEVFILE, 0);

	signal(SIGALRM, alarmtr);
	dh = -1;
	for(cd = condevs; ((cd->CU_meth != NULL)&&(dh < 0)); cd++) {
		if (snccmp(flds[F_LINE], cd->CU_meth) == SAME) {
			fseek(dfp, (off_t)0, 0);
			while(rddev(dfp, &dev) != FAIL) {
				if (strcmp(flds[F_CLASS], dev.D_class) != SAME)
					continue;
				if (snccmp(flds[F_LINE], dev.D_type) != SAME)
					continue;
				if (mlock(dev.D_line) == FAIL)
					continue;

				sprintf(dcname, "/dev/%s", dev.D_line);
				getnextfd();
				alarm(10);
				if (setjmp(Sjbuf)) {
					delock(dev.D_line);
					logent(dev.D_line,"micom open TIMEOUT");
					dh = -1;
					break;
					}
				dh = open(dcname, 2);
				alarm(0);
				next_fd = -1;
				if (dh > 0) {
					break;
					}
				devSel[0] = '\0';
				delock(dev.D_line);
				}
			}
		}
	fclose(dfp);
	if (dh < 0)
		return(CF_NODEV);

	speed = atoi(fdig(flds[F_CLASS]));
	fixline(dh, speed);
	sleep(1);

	/* negotiate with micom */
	if (speed != 4800)	/* damn their eyes! */
		write(dh, "\r", 1);
	else
		write(dh, " ", 1);
		
	DEBUG(4, "wanted %s ", "NAME");
	ok = expect("NAME", dh);
	DEBUG(4, "got %s\n", ok ? "?" : "that");
	if (ok == 0) {
		write(dh, flds[F_PHONE], strlen(flds[F_PHONE]));
		sleep(1);
		write(dh, "\r", 1);
		DEBUG(4, "wanted %s ", "GO");
		ok = expect("GO", dh);
		DEBUG(4, "got %s\n", ok ? "?" : "that");
	}

	if (ok != 0) {
		if (dh > 2)
			close(dh);
		DEBUG(4, "micom failed\n", "");
		delock(dev.D_line);
		return(CF_DIAL);
	} else
		DEBUG(4, "micom ok\n", "");

	CU_end = cd->CU_clos;
	strcat(devSel, dev.D_line);	/* for later unlock */
	return(dh);

}

miccls(fd)
register int fd;
{

	if (fd > 0) {
		close(fd);
		delock(devSel);
		}
	}
#endif MICOM

/***
 *	Acuopn - open an ACU and dial the number.  The condevs table
 *		will be searched until a dialing unit is found that is
 *		free.
 *
 *	return codes:	>0 - file number - o.k.
 *			FAIL - failed
 */

char devSel[20];	/* used for later unlock() */

Acuopn(flds)
register char *flds[];
{
    char phone[MAXPH+1];
    register struct condev *cd;
    register int fd;
    register FILE *dfp;
    struct Devices dev;
    char msg[100];
    int ret = CF_NODEV;

    exphone(flds[F_PHONE], phone);
    devSel[0] = '\0';
    DEBUG(4, "Dialing %s\n", phone);
    dfp = fopen(DEVFILE, "r");
    ASSERT(dfp != NULL, "Can't open", DEVFILE, 0);

    for(cd = condevs; cd->CU_meth != NULL; cd++) {
	if (prefix(cd->CU_meth, flds[F_LINE])) {
	    fseek(dfp, (off_t)0, 0);
	    while(rddev(dfp, &dev) != FAIL) {
		if (strcmp(flds[F_CLASS], dev.D_class) != SAME)
		    continue;
		if (snccmp(flds[F_LINE], dev.D_type) != SAME)
		    continue;
		if (dev.D_brand[0] == '\0')
		    logent("Acuopn","No 'brand' name on ACU");
		else if (snccmp(dev.D_brand, cd->CU_brand) != SAME)
		    continue;
		DEBUG(5,"correct brand %s\n", dev.D_brand);
		if (mlock(dev.D_line) == FAIL)
		    continue;

		DEBUG(4, "Using %s\n", cd->CU_brand);
		CU_end = cd->CU_clos;   /* point CU_end at close func */
		fd = (*(cd->CU_open))(phone, flds, &dev);
		if (fd > 0) {
		    DEBUG(5, "open succeeded\n", "");
		    fclose(dfp);
		    strcpy(devSel, dev.D_line);   /* save for later unlock() */
		    sprintf(msg, "%s, fd= %d",devSel, fd);
		    logent(msg, "using device");
		    return(fd);
		    }
		ret = CF_DIAL;
		CU_end = nulldev;
		delock(dev.D_line);
		}
	    }
	}
    fclose(dfp);
    return(ret);
    }

#ifdef DN11

/***
 *	dnopn(ph, flds, dev)	dial remote machine
 *	char *ph;
 *	char *flds[];
 *	struct Devices *dev;
 *
 *	return codes:
 *		file descriptor  -  succeeded
 *		FAIL  -  failed
 */

dnopn(ph, flds, dev)
char *ph;
char *flds[];
struct Devices *dev;
{
	char dcname[20], dnname[20], phone[MAXPH+2], c = 0;
#ifdef	SYSIII
	struct termio ttbuf;
#endif
	int dnf, dcf;
	int nw, lt, pid, status;
	unsigned timelim;

	sprintf(dnname, "/dev/%s", dev->D_calldev);
	errno = 0;
	
	if (setjmp(Sjbuf)) {
		logent(dnname, "CAN'T OPEN");
		DEBUG(4, "%s Open timed out\n", dnname);
		return(CF_NODEV);
	}
	signal(SIGALRM, alarmtr);
	getnextfd();
	alarm(10);
	dnf = open(dnname, 1);
	alarm(0);
	next_fd = -1;
	if (dnf < 0 && errno == EACCES) {
		logent(dnname, "CAN'T OPEN");
		logent("DEVICE", "NO");
		return(CF_NODEV);
		}
	/* rti!trt: avoid passing acu file descriptor to children */
	fioclex(dnf);

	sprintf(dcname, "/dev/%s", dev->D_line);
	sprintf(phone, "%s%s", ph, ACULAST);
	DEBUG(4, "dc - %s, ", dcname);
	DEBUG(4, "acu - %s\n", dnname);
	pid = 0;
	if (setjmp(Sjbuf)) {
		logent("DIALUP DN write", "TIMEOUT");
		if (pid)
			kill(pid, 9);
		delock(dev->D_line);
		if (dnf)
			close(dnf);
		return(FAIL);
	}
	signal(SIGALRM, alarmtr);
	timelim = 5 * strlen(phone);
	alarm(timelim < 30 ? 30 : timelim);
	if ((pid = fork()) == 0) {
		sleep(2);
		fclose(stdin);
		fclose(stdout);
#ifdef	TIOCFLUSH
		ioctl(dnf, TIOCFLUSH, STBNULL);
#endif
		nw = write(dnf, phone, lt = strlen(phone));
		if (nw != lt) {
			logent("DIALUP ACU write", "FAILED");
			exit(1);
		}
		DEBUG(4, "ACU write ok%s\n", "");
		exit(0);
	}
	/*  open line - will return on carrier */
	/* RT needs a sleep here because it returns immediately from open */

#if RT
	sleep(15);
#endif

	getnextfd();
	errno = 0;
	dcf = open(dcname, 2);
	next_fd = -1;
	if (dcf < 0 && errno == EACCES)
		logent(dcname, "CAN'T OPEN");
	DEBUG(4, "dcf is %d\n", dcf);
	if (dcf < 0) {
		logent("DIALUP LINE open", "FAILED");
		alarm(0);
		kill(pid, 9);
		close(dnf);
		delock(dev->D_line);
		return(FAIL);
	}
#ifdef ULTRIX
	/* ensure correct process group if run in background by cron */
	{
	int pgrp = getpgrp(0);
	ioctl(dcf, TIOCSPGRP, &pgrp);
	}
#endif
	/* brl-bmd.351 (Doug Kingston) says the next ioctl is unneeded . */
/*	ioctl(dcf, TIOCHPCL, STBNULL);*/
	while ((nw = wait(&lt)) != pid && nw != -1)
		;
#ifdef	SYSIII
	ioctl(dcf, TCGETA, &ttbuf);
	if(!(ttbuf.c_cflag & HUPCL)) {
		ttbuf.c_cflag |= HUPCL;
		ioctl(dcf, TCSETA, &ttbuf);
	}
#endif
	alarm(0);
	fflush(stdout);
	fixline(dcf, dev->D_speed);
	DEBUG(4, "Fork Stat %o\n", lt);
	if (lt != 0) {
		close(dcf);
		if (dnf)
			close(dnf);
		delock(dev->D_line);
		return(FAIL);
	}
	return(dcf);
}

/***
 *	dncls()		close dn type call unit
 *
 *	return codes:	None
 */
dncls(fd)
register int fd;
{
	if (fd > 0) {
		close(fd);
		sleep(5);
		delock(devSel);
		}
}
#endif DN11

#ifdef DF0




/***
 *	df0opn(ph, flds, dev)	dial remote machine
 *			with a df02 or df03.
 *	char *ph;
 *	char *flds[];
 *	struct Devices *dev;
 *
 *	return codes:
 *		file descriptor  -  succeeded
 *		FAIL  -  failed
 *
 *	Modified 9/28/81 by Bill Shannon (DEC)
 */

df0opn(ph, flds, dev)
char *ph;
char *flds[];
struct Devices *dev;
{

	char  dnname[20], phone[MAXPH+2], c = 0;
#ifdef	SYSIII
	struct termio ttbuf;
#endif
	int status;
	int nw, lt, pid, dnf;
	int ret, df02, df03;
	unsigned timelim;
	char msg[50];


	df02 = (strcmp(dev->D_brand, "DF02") == SAME);
	df03 = (strcmp(dev->D_brand, "DF03") == SAME);
	DEBUG(4, "df0 device is a %s\n", df02 ? "DF02" : "DF03");
	sprintf(dnname, "/dev/%s", dev->D_calldev);
	errno = 0;
#ifdef ONDELAY
	dnf = open(dnname, (df02||df03 ? O_RDWR : O_WRONLY)|O_NDELAY); 
#else
	dnf = open(dnname, (df02||df03 ? O_RDWR : O_WRONLY)); 
#endif
	if (dnf < 0 && errno == EACCES) {
		delock(dev->D_line);
		logent(dnname, "CAN'T OPEN");
		return(CF_TRYANOTHER);
	}
	else if (dnf < 0 && errno == EBUSY) {
		logent(dnname, "ALREADY OPEN");
		return(CF_TRYANOTHER);
		/* dont remove lock if someone else has
		 * this device (tip/cu). 
		 * tip/cu do not "refresh" lock files
		 * so it is possible to remove a valid
		 * lock file.  Fortunately tip uses
		 * exclusive access so any further opens
		 * should return EBUSY.
		 */
		}
	if (dnf<0) {
		sprintf(msg,"device=%s, errno = %d", dnname, errno);
		logent(msg, "open failed");
		return(FAIL);
	}
	else 
		Dcf=dnf; /* so clsacu will work */

#ifdef ULTRIX
	{
	int pgrp = getpgrp(0);
	int temp = 0;
	/* ensure correct process group if run in background by cron */
	ioctl(dnf, TIOCSPGRP, &pgrp);
#ifdef ONDELAY
	ret = ioctl(dnf, TIOCMODEM, &temp);  /* we are attatched to a modem */
	if (ret < 0)
		DEBUG(6, "ioctl(TIOCMODEM) - errno=%d\n", errno);
	ioctl(dnf, TIOCNCAR);  /* ignore carrier while dialing number */
#endif
	}
#endif
/* Changed from TIOCMSET to TIOCMBIS by rti!trt */

#ifdef TIOCMBIS
	if (df03) {
		/* set secondary transmit to mark (idle) state on dh's
		 * and dz32's (the regular DZ does not have ST).
		 * On devices with rate select (dmf/z) this translates to
		 * select the high speed rate for 1200 bps.
		 */
		int st=TIOCM_ST;
		fixline(dnf, 1200);
		if (dev->D_speed != 1200)
			ioctl(dnf, TIOCMBIC, &st);
		else
			ioctl(dnf, TIOCMBIS, &st);
	} else
#endif
#ifdef V7M11
			fixline(dnf, 300);
#else
			fixline(dnf, dev->D_speed);
#endif
	sprintf(phone, "\02%s", ph);
	DEBUG(4, "acu - %s\n", dnname);
	if (setjmp(Sjbuf)) {
		DEBUG(1, "DN write %s\n", "timeout");
		sprintf(msg, "DIALUP DN write - dnf=%d",dnf);
		logent(msg, "TIMEOUT");
		delock(dev->D_line);
		clsacu();
		return(FAIL);
	}
	signal(SIGALRM, alarmtr);
	timelim = 5 * strlen(phone);
	alarm(timelim < 30 ? 30 : timelim);
	/*  open line - will return on carrier */
	/* RT needs a sleep here because it returns immediately from open */

#if RT
	sleep(15);
#endif

#ifdef	TIOCFLUSH
	ioctl(dnf, TIOCFLUSH, 0);
#endif
	write(dnf, "\01", 1);
	sleep(1);
	nw = write(dnf, phone, lt = strlen(phone));
	if (nw != lt) {
		DEBUG(1, "DF0 write %s\n", "error");
		clsacu();
		delock(dev->D_line);
		logent("DIALUP DF0 write", "FAILED");
		return(CF_DIAL);
	}
	DEBUG(4, "DF0 write ok%s\n", "");
	errno=0;
	sprintf(msg,"FAILED acu=%s, char=%o, errno=%d",
		dnname, c, errno);
	if (read(dnf, &c, 1) < 0) {
		/* acu in strange state */
		clsacu();
		/* give this acu a rest, try another */
		logent(msg, "df02/df03 illegal return");
		delock(dev->D_line);
		return(CF_TRYANOTHER);
	}
#ifndef ONDELAY
	if (c != 'A') {
		alarm(0);
		clsacu();
		delock(dev->D_line);
		logent(msg, "FAILED-read on df02/3");
		DEBUG(4,"FAILED-%s\n", msg);
		return(CF_TRYANOTHER);
	}
#endif
#ifdef V7M11
	fixline(dnf, dev->D_speed);
#endif
#ifdef ONDELAY
	ioctl(dnf, TIOCCAR);  /* dont ignore carrier anymore */
	alarm(40);  /* find a better timeout value */
	if (setjmp(Sjbuf)) {
		DEBUG(1, "no carrier", "timeout");
		sprintf(msg, "no carrier - dnf=%d",dnf);
		logent(msg, "TIMEOUT");
		delock(dev->D_line);
		clsacu();
		return(FAIL);
	}
	ioctl(dnf, TIOCWONLINE); /* wait for carrier */
	alarm(0);
#endif
#ifdef	SYSIII
	ioctl(dnf, TCGETA, &ttbuf);
	if(!(ttbuf.c_cflag & HUPCL)) {
		ttbuf.c_cflag |= HUPCL;
		ioctl(dnf, TCSETA, &ttbuf);
	}
#endif
	alarm(0);
	fflush(stdout);
	fixline(dnf, dev->D_speed);
	return(dnf);
}


/*
 * df0cls()	close the DF02/DF03 call unit
 *
 *	return codes: none
 */

df0cls(fd)
register int fd;
{
	char msg[20];
	DEBUG(5, "close df02/df03, Dcf=%d\n", fd);
	if (fd > 0) {
		ioctl(fd, TIOCCDTR, STBNULL);
		write(fd, "\01", 1);
		if (close(fd) < 0) {
			sprintf(msg,"errno=%d, fd=%d",errno, fd);
			logent(msg, "could not close acu");
		}
		else {
			sprintf(msg,"fd=%d",fd);
			logent(msg,"closed df0 type acu");
		}
		sleep(5);
		delock(devSel);
	}
}
#endif DF0

#ifdef DF1
/***
 *	df1opn(telno, flds, dev) connect to df112/df224
 *	char *flds[], *dev[];
 *
 *	return codes:
 *		>0  -  file number  -  ok
 *		CF_DIAL,CF_DEVICE  -  failed
 */
/*
 * Assume touch tone for now
 */

df1opn(telno, flds, dev)
char *telno;
char *flds[];
struct Devices *dev;
{
	int	dh = -1;
	register int i;
	register int j;
	extern errno;
	char dcname[20], schar[2], msg[50], dvname[6];
	int twice, df112;
	char c;
	int	rw;

	df112 = (strcmp(dev->D_brand, "DF112") == SAME);
	if (df112) {
		strcpy(schar,"#");
		strcpy(dvname,"df112");
	} else {
		strcpy(schar,"!");
		strcpy(dvname,"df224");
	}
	DEBUG(4, "df1 device is a %s\n", dvname);
	sprintf(dcname, "/dev/%s", dev->D_line);
	DEBUG(4, "dc - %s\n", dcname);
	if (setjmp(Sjbuf)) {
		sprintf(msg,"timeout %s open %s\n", dvname, dcname);
		DEBUG(1, msg, "");
		sprintf(msg,"%s open", dvname);
		logent(msg, "TIMEOUT");
		if (Dcf >= 0) 
			clsacu();
		delock(dev->D_line);
		return(CF_DIAL);
	}
	signal(SIGALRM, alarmtr);
	getnextfd();
	/* translate telephone number for df112/df224 */
	for (j = i = 0; i < strlen(telno); ++i) {
		switch(telno[i]) {
		case '-':	/* df112 doesn't like dash */
		case ' ':	/* df112 doesn't like space */
			break;
		default:
			telno[j++] = telno[i];
			break;
		}
	}
	telno[j] = '\0';
#ifdef ONDELAY
	dh = open(dcname, O_RDWR|O_NDELAY); 
#else
	alarm(10);
	dh = open(dcname, 2); /* read/write */
	alarm(0);
#endif
	Dcf =dh;

	/* modem is open */
	next_fd = -1;
	if (dh >= 0) {
		fixline(dh, dev->D_speed);
		sleep(2);
/*
 * Must try the dial twice so the Brain-damaged df112/df224 dialer
 * can determine the proper speed to dial at. Ohms 11/6/84
 */
		for(twice = 2; twice; twice--) {
			ioctl(dh, TIOCFLUSH, &rw);
 			/*cntrl A = burst mode, P*/
			write(dh, "\001", 1);
			write(dh, "P", 1);
			write(dh, telno, strlen(telno));
			write(dh, schar, 1);
 			read(dh, &c, 1);	/* avoid carrige return */
 			read(dh, &c, 1);	/* and line feed */
 			read(dh, &c, 1);
 			sleep(2);   /* allow time for the rest of the modem */
 				   /* "Attached" message to be recieved */
			if(c == 'A') {	   /* if we got an A, ok */
				break;		/* success */
			}
		}
 		ioctl(dh, TIOCFLUSH, &rw);	/* get rid of modem garbage */
		if(c != 'A') {
			delock(dev->D_line);
			clsacu();
 			return (FAIL);	/* fail */
		}
	}
	if (dh < 0) {
		DEBUG(4, "%s failed\n", dvname);
		delock(dev->D_line);
	}
#ifdef ONDELAY
	ioctl(dh, TIOCCAR);  /* dont ignore carrier anymore */
	alarm(40);  /* find a better timeout value */
	if (setjmp(Sjbuf)) {
		DEBUG(1, "no carrier", "timeout");
		sprintf(msg, "no carrier %s - dh=%d",dvname, dh);
		logent(msg, "TIMEOUT");
		delock(dev->D_line);
		clsacu();
		return(FAIL);
	}
	ioctl(dh, TIOCWONLINE); /* wait for carrier */
	alarm(0);
#endif
	DEBUG(4, "%s ok\n", dvname);
	return(dh);
}

df1cls(fd)
int fd;
{
	char dcname[20];
	struct sgttyb hup, sav;
	char msg[50];

	if (fd > 0) {
		sprintf(dcname, "/dev/%s", devSel);
		DEBUG(4, "Hanging up fd = %d\n", fd);
		ioctl(fd, TIOCCDTR, STBNULL);
		sleep(2);
		sprintf(msg, "fd= %d", fd);
		logent(msg, "df112/df224: closing");
		errno = 0;
		if (close(fd)<0) {
			sprintf(msg, "errno =%d", errno);
			logent(msg, "df112/df224: could not close");
		}
		delock(devSel);
		}
	}

#endif DF1

#ifdef HAYES
/***
 *	hysopn(telno, flds, dev) connect to hayes smartmodem
 *	char *flds[], *dev[];
 *
 *	return codes:
 *		>0  -  file number  -  ok
 *		CF_DIAL,CF_DEVICE  -  failed
 */
/*
 * Define HAYSTONE if you have touch tone dialing.
 */
/*#define HAYSTONE	*/

hysopn(telno, flds, dev)
char *telno;
char *flds[];
struct Devices *dev;
{
	int	dh = -1;
	register int i;
	extern errno;
	char dcname[20];

	sprintf(dcname, "/dev/%s", dev->D_line);
	DEBUG(4, "dc - %s\n", dcname);
	if (setjmp(Sjbuf)) {
		DEBUG(1, "timeout hayes open %s\n", dcname);
		logent("hayes open", "TIMEOUT");
		if (Dcf >= 0) 
			clsacu();
		delock(dev->D_line);
		return(CF_DIAL);
	}
	signal(SIGALRM, alarmtr);
	getnextfd();
	/* translate telephone number for hayes */
	for (i = 0; i < strlen(telno); ++i) {
		switch(telno[i]) {
		case '-':	/* delay */
			telno[i] = ',';
			break;
		case '=':	/* await dial tone */
			telno[i] = ',';
			break;
		}
	}
#ifdef ONDELAY
	dh = open(dcname, O_RDWR|O_NDELAY); 
#else
	alarm(10);
	dh = open(dcname, 2); /* read/write */
	alarm(0);
#endif
	Dcf =dh;

	/* modem is open */
	next_fd = -1;
	if (dh >= 0) {
#ifdef ULTRIX
		/* ensure correct process group if run in background by cron */
		{
		int pgrp = getpgrp(0);
		int temp = 0;
		ioctl(dh, TIOCSPGRP, &pgrp);
#ifdef ONDELAY
		/* we are attatched to a modem so dont ignore modem signals */
		ioctl(dh, TIOCMODEM, &temp);
		ioctl(dh, TIOCNCAR); /* ignore soft carr while dialing number */
#endif
		}
#endif
		fixline(dh, dev->D_speed);
		sleep(2);
		write(dh, "+++", 3);
		sleep(2);
		write(dh, "ATH\r", 4);
		sleep(1);
		write(dh, "ATZ\r", 4);
		sleep(2);
		write(dh, "+++", 3);
		sleep(2);
#ifdef HAYSTONE
		write(dh, "ATDT", 4);
#else
		write(dh, "ATDP", 4);
#endif
		write(dh, telno, strlen(telno));
		write(dh, "\r", 1);

		if (expect("CONNECT", dh) != 0) {
			logent("HSM no carrier", "FAILED");
			strcpy(devSel, dev->D_line);
			hyscls(dh);
			return(CF_DIAL);
		}

	}
	if (dh < 0) {
		DEBUG(4, "hayes failed\n", "");
		delock(dev->D_line);
	}
#ifdef ONDELAY
	ioctl(dh, TIOCCAR);  /* dont ignore carrier anymore */
	alarm(40);  /* find a better timeout value */
	if (setjmp(Sjbuf)) {
		char msg[50];
		DEBUG(1, "no carrier", "timeout");
		sprintf(msg, "no carrier hayes - dh=%d",dh);
		logent(msg, "TIMEOUT");
		delock(dev->D_line);
		clsacu();
		return(FAIL);
	}
	ioctl(dh, TIOCWONLINE); /* wait for carrier */
	alarm(0);
#endif
	DEBUG(4, "hayes ok\n", "");
	return(dh);
}

hyscls(fd)
int fd;
{
	char dcname[20];
	struct sgttyb hup, sav;
	char msg[50];

	if (fd > 0) {
		sprintf(dcname, "/dev/%s", devSel);
		DEBUG(4, "Hanging up fd = %d\n", fd);
		ioctl(fd, TIOCCDTR, STBNULL);
		sleep(2);
/*
 * If you have a getty sleeping on this line, when it wakes up it sends
 * all kinds of garbage to the modem.  Unfortunatly, the modem likes to
 * execute the previous command when it sees the garbage.  The previous
 * command was to dial the phone, so let's make the last command reset
 * the modem.
 */
		sleep(2);
		write(fd, "+++", 3);
		sleep(2);
		write(fd, "ATZ\r", 4);
		sprintf(msg, "fd= %d", fd);
		logent(msg, "hayes: closing");
		errno = 0;
		if (close(fd)<0) {
			sprintf(msg, "errno =%d", errno);
			logent(msg, "hayesq: could not close");
		}
		delock(devSel);
		}
	}

#endif HAYES

#ifdef HAYESQ
/*
 * New dialout routine to work with Hayes' SMART MODEM
 * 13-JUL-82, Mike Mitchell
 * Modified 23-MAR-83 to work with Tom Truscott's (rti!trt)
 * version of UUCP	(ncsu!mcm)
 *
 * The modem should be set to NOT send any result codes to
 * the system (switch 3 up, 4 down). This end will figure out
 * what is wrong.
 *
 * I had lots of problems with the modem sending
 * result codes since I am using the same modem for both incomming and
 * outgoing calls.  I'd occasionally miss the result code (getty would
 * grab it), and the connect would fail.  Worse yet, the getty would
 * think the result code was a user name, and send garbage to it while
 * it was in the command state.  I turned off ALL result codes, and hope
 * for the best.  99% of the time the modem is in the correct state.
 * Occassionally it doesn't connect, or the phone was busy, etc., and
 * uucico sits there trying to log in.  It eventually times out, calling
 * clsacu() in the process, so it resets itself for the next attempt.
 */

/*
 * Define HAYSTONE if touch-tone dialing is to be used.  If it is not defined,
 * Pulse dialing is assumed.
 */
/*#define HAYSTONE*/

hysqopn(telno, flds, dev)
char *telno, *flds[];
struct Devices *dev;
{
	char dcname[20], phone[MAXPH+10], c = 0;
#ifdef	SYSIII
	struct termio ttbuf;
#endif
	int status, dnf;
	register int i;
	unsigned timelim;

	signal(SIGALRM, alarmtr);
	sprintf(dcname, "/dev/%s", dev->D_line);

	/* translate telephone number for hayes */
	for (i = 0; i < strlen(telno); ++i) {
		switch(telno[i]) {
		case '-':	/* delay */
			telno[i] = ',';
			break;
		case '=':	/* await dial tone */
			telno[i] = ',';
			break;
		}
	}
	getnextfd();
	if (setjmp(Sjbuf)) {
		delock(dev->D_line);
		logent("TIMEOUT", "HAYESQ");
		DEBUG(4, "Open timed out %s", dcname);
		if (Dcf > 0)
			clsacu();
		return(CF_NODEV);
		}
	alarm(10);

	if ((dnf = open(dcname, 2)) <= 0) {
		delock(dev->D_line);
		logent("DEVICE", "NO");
		DEBUG(4, "Can't open %s", dcname);
		return(CF_NODEV);
		}
#ifdef ULTRIX
	/* ensure correct process group if run in background by cron */
	{
	int pgrp = getpgrp(0);
	ioctl(dnf, TIOCSPGRP, &pgrp);
	}
#endif

	Dcf = dnf;
	alarm(0);
	next_fd = -1;
	fixline(dnf, dev->D_speed);
	DEBUG(4, "Hayes port - %s, ", dcname);
	sleep(2);
	write(dnf, "+++", 3);
	sleep(2);
	write(dnf, "ATH\r", 4);
	sleep(1);
	write(dnf, "ATZ\r", 4);
	sleep(2);
	write(dnf, "+++", 3);
	sleep(2);

#ifdef HAYSTONE
	sprintf(phone, "ATDT%s\r", telno);
#else
	sprintf(phone, "ATDP%s\r", telno);
#endif

	write(dnf, phone, strlen(phone));

/* calculate delay time for the other system to answer the phone.
 * Default is 15 seconds, add 2 seconds for each comma in the phone
 * number.
 */
	timelim = 150;
	while(*telno) {
		c = *telno++;
		if (c == ',')
			timelim += 20;
		else {
#ifdef HAYSTONE
			timelim += 2;	/* .2 seconds per tone */
			}
#else
			if (c == '0') timelim += 10;   /* .1 second per digit */
			else if (c > '0' && c <= '9')
				timelim += (c - '0');
			}
#endif
		}
	alarm(timelim/10);
	if (setjmp(Sjbuf) == 0) {
		read(dnf, &c, 1);
		alarm(0);
		}

	return(dnf);
	}

hysqcls(fd)
int fd;
{
	char dcname[20];
	struct sgttyb hup, sav;
	char msg[50];

	if (fd > 0) {
		sprintf(dcname, "/dev/%s", devSel);
		DEBUG(4, "Hanging up fd = %d\n", fd);
		ioctl(fd, TIOCCDTR, STBNULL);
		sleep(2);
/*
 * If you have a getty sleeping on this line, when it wakes up it sends
 * all kinds of garbage to the modem.  Unfortunatly, the modem likes to
 * execute the previous command when it sees the garbage.  The previous
 * command was to dial the phone, so let's make the last command reset
 * the modem.
 */
		sleep(2);
		write(fd, "+++", 3);
		sleep(2);
		write(fd, "ATZ\r", 4);
		sprintf(msg, "fd= %d", fd);
		logent(msg, "hayesq: closing");
		errno = 0;
		if (close(fd)<0) {
			sprintf(msg, "errno =%d", errno);
			logent(msg, "hayesq: could not close");
		}
		delock(devSel);
		}
	}

#endif HAYESQ

#ifdef	VENTEL
ventopn(telno, flds, dev)
char *flds[], *telno;
struct Devices *dev;
{
	int	dh;
	int	i, ok = -1;
	char dcname[20];

	sprintf(dcname, "/dev/%s", dev->D_line);
	if (setjmp(Sjbuf)) {
		DEBUG(1, "timeout ventel open\n", "");
		logent("ventel open", "TIMEOUT");
		if (dh >= 0)
			close(dh);
		delock(dev->D_line);
		return(CF_NODEV);
	}
	signal(SIGALRM, alarmtr);
	getnextfd();
#ifdef ONDELAY
	dh = open(dcname, O_RDWR|O_NDELAY); 
#else
	alarm(10);
	dh = open(dcname, 2);
	alarm(0);
#endif
	next_fd = -1;
	if (dh < 0) {
		DEBUG(4,"%s\n", errno == 4 ? "no carrier" : "can't open modem");
		delock(dev->D_line);
		return(errno == 4 ? CF_DIAL : CF_NODEV);
	}
#ifdef ULTRIX
	/* ensure correct process group if run in background by cron */
	{
	int pgrp = getpgrp(0);
	int temp = 0;
	ioctl(dh, TIOCSPGRP, &pgrp);
#ifdef ONDELAY
	/* we are attatched to a modem so dont ignore modem signals */
	ioctl(dh, TIOCMODEM, &temp);
	ioctl(dh, TIOCNCAR); /* ignore soft carr while dialing number */
#endif
	}
#endif

	/* modem is open */
	fixline(dh, dev->D_speed);

	/* translate - to % and = to & for VenTel */
	DEBUG(4, "calling %s -> ", telno);
	for (i = 0; i < strlen(telno); ++i) {
		switch(telno[i]) {
		case '-':	/* delay */
			telno[i] = '%';
			break;
		case '=':	/* await dial tone */
			telno[i] = '&';
			break;
		case '<':
			telno[i] = '%';
			break;
		}
	}
	DEBUG(4, "%s\n", telno);
	sleep(1);
	for(i = 0; i < 5; ++i) {	/* make up to 5 tries */
 		slowrite(dh, "\r\r");/* awake, thou lowly VenTel! */

		DEBUG(4, "wanted %s ", "$");
		ok = expect("$", dh);
		DEBUG(4, "got %s\n", ok ? "?" : "that");
		if (ok != 0)
			continue;
 		slowrite(dh, "K");	/* "K" (enter number) command */
		DEBUG(4, "wanted %s ", "DIAL: ");
		ok = expect("DIAL: ", dh);
		DEBUG(4, "got %s\n", ok ? "?" : "that");
		if (ok == 0)
			break;
	}

	if (ok == 0) {
 		slowrite(dh, telno); /* send telno, send \r */
 		slowrite(dh, "\r");
		DEBUG(4, "wanted %s ", "ONLINE");
		ok = expect("ONLINE!", dh);
		DEBUG(4, "got %s\n", ok ? "?" : "that");
	}
	if (ok != 0) {
		if (dh > 2)
			close(dh);
		DEBUG(4, "venDial failed\n", "");
		return(CF_DIAL);
	} else
		DEBUG(4, "venDial ok\n", "");
#ifdef ONDELAY
	ioctl(dh, TIOCCAR);  /* dont ignore carrier anymore */
	alarm(40);  /* find a better timeout value */
	if (setjmp(Sjbuf)) {
		char msg[50];
		DEBUG(1, "no carrier", "timeout");
		sprintf(msg, "no carrier ventel - dh=%d",dh);
		logent(msg, "TIMEOUT");
		delock(dev->D_line);
		clsacu();
		return(FAIL);
	}
	ioctl(dh, TIOCWONLINE); /* wait for carrier */
	alarm(0);
#endif
	return(dh);
}


/*
 * uucpdelay:  delay execution for numerator/denominator seconds.
 */

#ifdef INTERVALTIMER
#define uucpdelay(num,denom) intervaldelay(1000000*num/denom)
#include <sys/time.h>
catch alarm sig
SIGALRM
struct itimerval itimerval;
itimerval.itimer_reload =
itimerval.rtime.itimer_interval =
itimerval.rtime.itimer_value =
settimer(ITIMER_REAL, &itimerval);
pause();
alarm comes in
turn off timer.
#endif INTERVALTIMER

#ifdef FASTTIMER
#define uucpdelay(num,denom) nap(60*num/denom)
/*	Sleep in increments of 60ths of second.	*/
nap (time)
	register int time;
{
	static int fd;

	if (fd == 0)
		fd = open (FASTTIMER, 0);

	read (fd, 0, time);
}
#endif FASTTIMER

#ifdef FTIME
#define uucpdelay(num,denom) ftimedelay(1000*num/denom)
#include <sys/timeb.h>
ftimedelay(n)
{
	static struct timeb loctime;
	ftime(&loctime);
	{register i = loctime.millitm;
	   while (abs((int)(loctime.millitm - i))<n) ftime(&loctime);
	}
}
#endif FTIME

#ifdef BUSYLOOP
#define uucpdelay(num,denom) busyloop(CPUSPEED*num/denom)
#define CPUSPEED 1000000	/* VAX 780 is 1MIPS */
#define	DELAY(n)	{ register long N = (n); while (--N > 0); }
busyloop(n)
	{
	DELAY(n);
	}
#endif BUSYLOOP

slowrite(fd, str)
register char *str;
{
	DEBUG(6, "slowrite ", "");
	while (*str) {
		DEBUG(6, "%c", *str);
		uucpdelay(1,10);	/* delay 1/10 second */
		write(fd, str, 1);
		str++;
		}
	DEBUG(6, "\n", "");
}


ventcls(fd)
int fd;
{

	if (fd > 0) {
		close(fd);
		sleep(5);
		delock(devSel);
		}
}
#endif VENTEL

#ifdef VADIC

/*
 *	vadopn: establish dial-out connection through a Racal-Vadic 3450.
 *	Returns descriptor open to tty for reading and writing.
 *	Negative values (-1...-7) denote errors in connmsg.
 *	Be sure to disconnect tty when done, via HUPCL or stty 0.
 */

vadopn(telno, flds, dev)
char *telno;
char *flds[];
struct Devices *dev;
{
	int	dh = -1;
	int	i, ok, er = 0, delay;
	extern errno;
	char dcname[20];

	sprintf(dcname, "/dev/%s", dev->D_line);
	if (setjmp(Sjbuf)) {
		DEBUG(1, "timeout vadic open\n", "");
		logent("vadic open", "TIMEOUT");
		if (dh >= 0)
			close(dh);
		delock(dev->D_line);
		return(CF_NODEV);
	}
	signal(SIGALRM, alarmtr);
	getnextfd();
#ifdef ONDELAY
	dh = open(dcname, O_RDWR|O_NDELAY); 
#else
	alarm(10);
	dh = open(dcname, 2);
	alarm(0);
#endif

	/* modem is open */
	next_fd = -1;
	if (dh < 0) {
		delock(dev->D_line);
		return(CF_NODEV);
		}
#ifdef ULTRIX
	{
	int pgrp = getpgrp(0);
	int temp = 0;
	/* ensure correct process group if run in background by cron */
	ioctl(dh, TIOCSPGRP, &pgrp);
#ifdef ONDELAY
	/* we are attatched to a modem so dont ignore modem signals */
	ioctl(dh, TIOCMODEM, &temp);
	ioctl(dh, TIOCNCAR); /* ignore soft carr while dialing number */
#endif
	}
#endif
	fixline(dh, dev->D_speed);

/* translate - to K for Vadic */
	DEBUG(4, "calling %s -> ", telno);
	delay = 0;
	for (i = 0; i < strlen(telno); ++i) {
		switch(telno[i]) {
		case '=':	/* await dial tone */
		case '-':	/* delay */
		case '<':
			telno[i] = 'K';
			delay += 5;
			break;
		}
	}
	DEBUG(4, "%s\n", telno);
	for(i = 0; i < 5; ++i) {	/* make 5 tries */
		/* wake up Vadic */
		sendthem("\005\\d", dh);
		DEBUG(4, "wanted %s ", "*");
		ok = expect("*", dh);
		DEBUG(4, "got %s\n", ok ? "?" : "that");
		if (ok != 0)
			continue;

		sendthem("D\\d", dh);	/* "D" (enter number) command */
		DEBUG(4, "wanted %s ", "NUMBER?\\r\\n");
		ok = expect("NUMBER?\r\n", dh);
		DEBUG(4, "got %s\n", ok ? "?" : "that");
		if (ok != 0)
			continue;

	/* send telno, send \r */
		sendthem(telno, dh);
		ok = expect(telno, dh);
		if (ok == 0)
			ok = expect("\r\n", dh);
		DEBUG(4, "got %s\n", ok ? "?" : "that");
		if (ok != 0)
			continue;

		sendthem("", dh); /* confirm number */
		DEBUG(4, "wanted %s ", "DIALING: ");
		ok = expect("DIALING: ", dh);
		DEBUG(4, "got %s\n", ok ? "?" : "that");
		if (ok == 0)
			break;
	}

	if (ok == 0) {
		sleep(10 + delay);	/* give vadic some time */
		DEBUG(4, "wanted ON LINE\\r\\n ", 0);
		ok = expect("ON LINE\r\n", dh);
		DEBUG(4, "got %s\n", ok ? "?" : "that");
	}

	if (ok != 0) {
		sendthem("I\\d", dh);	/* back to idle */
		if (dh > 2)
			close(dh);
		DEBUG(4, "vadDial failed\n", "");
		delock(dev->D_line);
		return(CF_DIAL);
	}
	DEBUG(4, "vadic ok\n", "");
#ifdef ONDELAY
	ioctl(dh, TIOCCAR);  /* dont ignore carrier anymore */
	alarm(40);  /* find a better timeout value */
	if (setjmp(Sjbuf)) {
		char msg[50];
		DEBUG(1, "no carrier", "timeout");
		sprintf(msg, "no carrier vadic - dh=%d",dh);
		logent(msg, "TIMEOUT");
		delock(dev->D_line);
		clsacu();
		return(FAIL);
	}
	ioctl(dh, TIOCWONLINE); /* wait for carrier */
	alarm(0);
#endif
	return(dh);
}

vadcls(fd) {

	if (fd > 0) {
		close(fd);
		sleep(5);
		delock(devSel);
		}
	}

#endif VADIC

#ifdef	RVMACS
/*
 * Racal-Vadic 'RV820' MACS system with 831 adaptor.
 * A typical 300 baud L-devices entry is
 *	ACU /dev/tty10 /dev/tty11,48 300 rvmacs
 * where tty10 is the communication line (D_Line),
 * tty11 is the dialer line (D_calldev),
 * the '4' is the dialer address + modem type (viz. dialer 0, Bell 103),
 * and the '8' is the communication port (they are 1-indexed).
 * BUGS:
 * Only tested with one dialer, one modem
 * uses common speed for dialer and communication line.
 * UNTESTED
 */

#define	STX	02	/* Access Adaptor */
#define	ETX	03	/* Transfer to Dialer */
#define	SI	017	/* Buffer Empty (end of phone number) */
#define	SOH	01	/* Abort */

rvmacsopn(ph, flds, dev)
char *ph, *flds[];
struct Devices *dev;
{
	register int va, i, child;
	register char *p;
	char c, acu[20], com[20];

	child = -1;
	if ((p = index(dev->D_calldev, ',')) == NULL) {
		DEBUG(2, "No dialer/modem specification\n", 0);
		goto failret;
	}
	*p++ = '\0';
	if (setjmp(Sjbuf)) {
		logent("rvmacsopn", "TIMEOUT");
		i = CF_DIAL;
		goto ret;
	}
	DEBUG(4, "STARTING CALL\n", 0);
	sprintf(acu, "/dev/%s", dev->D_calldev);
	getnextfd();
	signal(SIGALRM, alarmtr);
	alarm(30);
	if ((va = open(acu, 2)) < 0) {
		logent(acu, "CAN'T OPEN");
		i = CF_NODEV;
		goto ret;
	}
#ifdef ULTRIX
	/* ensure correct process group if run in background by cron */
	{
	int pgrp = getpgrp(0);
	ioctl(va, TIOCSPGRP, &pgrp);
	}
#endif
	fixline(va, dev->D_speed);

	p_chwrite(va, STX);	/* access adaptor */
	i = *p++ - '0';
	if (i < 0 || i > 7) {
		logent(p-1, "Bad dialer address/modem type\n");
		goto failret;
	}
	p_chwrite(va, i);		/* Send Dialer Address Digit */
	i = *p - '0';
	if (i <= 0 || i > 14) {
		logent(p-1, "Bad modem address\n");
		goto failret;
	}
	p_chwrite(va, i-1);	/* Send Modem Address Digit */
	write(va, ph, strlen(ph));	/* Send Phone Number */
	p_chwrite(va, SI);	/* Send Buffer Empty */
	p_chwrite(va, ETX);	/* Initiate Call */
	sprintf(com, "/dev/%s", dev->D_line);

	/* create child to open comm line */
	if ((child = fork()) == 0) {
		signal(SIGINT, SIG_DFL);
		open(com, 0);
		sleep(5);
		exit(1);
	}

	if (read(va, &c, 1) != 1) {
		logent("ACU READ", "FAILED");
		goto failret;
	}
	switch(c) {
	case 'A':
		/* Fine! */
		break;
	case 'B':
		DEBUG(2, "CALL ABORTED\n", 0);
		goto failret;
	case 'D':
		DEBUG(2, "Dialer format error\n", 0);
		goto failret;
	case 'E':
		DEBUG(2, "Dialer parity error\n", 0);
		goto failret;
	case 'F':
		DEBUG(2, "Phone number too long\n", 0);
		goto failret;
	case 'G':
		DEBUG(2, "Busy signal\n", 0);
		goto failret;
	default:
		DEBUG(2, "Unknown MACS return code '%c'\n", i);
		goto failret;
	}
	/*
	 * open line - will return on carrier
	 */
	if ((i = open(com, 2)) < 0) {
		if (errno == EIO)
			logent("carrier", "LOST");
		else
			logent("dialup open", "FAILED");
		goto failret;
	}
#ifdef ULTRIX
	/* ensure correct process group if run in background by cron */
	{
	int pgrp = getpgrp(0);
	ioctl(i, TIOCSPGRP, &pgrp);
	}
#endif
	fixline(i, dev->D_speed);
	goto ret;
failret:
	i = CF_DIAL;
ret:
	alarm(0);
	if (child != -1)
		kill(child, SIGKILL);
	close(va);
	return(i);
}

rvmacscls(fd)
register int fd;
{
	DEBUG(2, "MACS close %d\n", fd);
	p_chwrite(fd, SOH);
/*	ioctl(fd, TIOCCDTR, NULL);*/
	close(fd);
}
#endif
