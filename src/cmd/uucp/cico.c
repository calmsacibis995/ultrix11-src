
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char sccsid[] = "@(#)cico.c	3.0 (decvax!larry) 4/22/86";
#endif

/*******
 *	cico - this program is used  to place a call to a
 *	remote machine, login, and copy files between the two machines.
 *	The following options are available:
 *	  -r# : if #==0 operate in slave mode,  if #==1 operate in master mode
 *	  -X# : debugging output from packet level code (pk0.c pk1.c),  0<X<10
 *	  -x# : debugging output from all other routines, 0<x<10
 *	  -sNAME :  start transfer to specified system NAME, if NAME is null
 *		    then start a general poll to all systems that have work.
 *
 *	decvax!larry -  add callcheck() routine to check if remote machine
 *			is using its valid login id.
 * 	decvax!larry -  cleanup code - unnecessary close()'s.
 *		     -  uustat messages added.
 *		     -  cleanup() modified to call clsacu()
 *		     -  intrEXIT() cleans up LCK files.
 *
 */


#include "uucp.h"
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#ifdef	SYSIII
#include <termio.h>
#else
#include <sgtty.h>
#endif
#include "uust.h"
#include "uusub.h"


jmp_buf Sjbuf;
	/*  call fail text  */
char *Stattext[] = {
	"",
	"",    /* conn should not return -1 */
	"WRONG TIME",
	"SYSTEM LOCKED",
	"NO DEVICE",
	"DIAL FAILED",
	"LOGIN FAILED",
	"BAD SEQUENCE",
	"LOGIN/SYSTEM FAILED",
	"BAD SYSTEM"
	};

int Role = 0;

	/*  call fail codes  */
int Stattype[] = {0, 0, 0, 0, 
	SS_NODEVICE, 0, SS_FAIL, SS_BADSEQ,  SS_BADLOGIN, 0
	};


extern int errno;
int Errorrate = 0;
#ifdef	SYSIII
struct termio Savettyb;
#else
struct sgttyb Savettyb;
#endif


main(argc, argv)
char *argv[];
{
	int ret, seq;
	int onesys = 0;
	char wkpre[NAMESIZE], file[NAMESIZE];
	char msg[BUFSIZ], *p, *q;
	extern onintr(), timeout();
	extern intrEXIT();
	extern char *pskip();
	char rflags[30];
	char *ttyn;
	int orig_uid = getuid();
	register int i;
	int ldisc = 0;
	int force = 0;

	strcpy(Progname, "uucico");
	uucpname(Myname);


	signal(SIGILL, intrEXIT);
	signal(SIGTRAP, intrEXIT);
	signal(SIGIOT, intrEXIT);
	signal(SIGEMT, intrEXIT);
	signal(SIGFPE, intrEXIT);
	signal(SIGBUS, intrEXIT);
	signal(SIGSEGV, intrEXIT);
	signal(SIGSYS, intrEXIT);
	signal(SIGPIPE, onintr);
	signal(SIGINT, onintr);
	signal(SIGHUP, onintr);
	signal(SIGQUIT, onintr);
	signal(SIGTERM, onintr);
	ret = guinfo(getuid(), User, msg);
	ASSERT(ret == 0, "BAD UID ", "", ret);
	strcpy(Loginuser, User);

	/* Try to run as uucp -- rti!trt */
	setgid(getegid());
	setuid(geteuid());

	rflags[0] = '\0';
	umask(WFMASK);
	strcpy(Rmtname, Myname);
	Ifn = Ofn = -1;
	while(argc>1 && argv[1][0] == '-'){
		switch(argv[1][1]){
		case 'd':
			Spool = &argv[1][2];
			break;
#ifdef PROTODEBUG
		case 'E':
			Errorrate = atoi(&argv[1][2]);
			if (Errorrate <= 0)
				Errorrate = 100;
			break;
		case 'g':
			Pkdrvon = 1;
			break;
		case 'G':
			Pkdrvon = 1;
			strcat(rflags, " -g ");
			break;
#endif
		case 'r':
			Role = atoi(&argv[1][2]);
			break;
		case 's':
			sprintf(Rmtname, "%.7s", &argv[1][2]);
			if (Rmtname[0] != '\0')
				onesys = 1;
			break;
		case 'X':
			Pkdebug = atoi(&argv[1][2]);
			if (Pkdebug <= 0)
				Pkdebug = 1;
			strcat(rflags, argv[1]);
			break;
		case 'x':
			chkdebug(orig_uid);
			Debug = atoi(&argv[1][2]);
			if (Debug <= 0)
				Debug = 1;
			strcat(rflags, argv[1]);
			break;
		case 'f':
			force++;
			break;
		default:
			printf("unknown flag %s\n", argv[1]);
			break;
		}
		--argc;  argv++;
	}


	if (Role == SLAVE) {
		/* initial handshake */
		onesys = 1;
#ifdef	SYSIII
		ret = ioctl(0, TCGETA, &Savettyb);
		Savettyb.c_cflag = (Savettyb.c_cflag & ~CS8) | CS7;
		Savettyb.c_oflag |= OPOST;
		Savettyb.c_lflag |= (ISIG|ICANON|ECHO);
#else
		ret = ioctl(0, TIOCGETP, &Savettyb);
		Savettyb.sg_flags |= ECHO;
		Savettyb.sg_flags &= ~RAW;
#endif
		Ifn = 0;
		Ofn = 1;
		fixmode(Ifn);
#ifdef NEWLDISC
		ldisc = HCLDISC;
		DEBUG(4,"Switching to new line discipline\n","");
		if ((ret = ioctl(Ifn,TIOCSETD,&ldisc)) < 0) 
			ASSERT_NOFAIL(ret != -1 ,"CAN NOT SWITCH_I LDISC","errno", errno);
		if ((ret = ioctl(Ofn,TIOCSETD,&ldisc)) < 0) 
			ASSERT_NOFAIL(ret != -1 ,"CAN NOT SWITCH_O LDISC","errno", errno);
#endif
		omsg('S', "here", Ofn);
		signal(SIGALRM, timeout);
		alarm(MAXMSGTIME);
		if (setjmp(Sjbuf)) {
			/* timed out */
#ifdef	SYSIII
			ret = ioctl(0, TCSETA, &Savettyb);
#else
			ret = ioctl(0, TIOCSETP, &Savettyb);
#endif
	DEBUG(4,"TIMEOUT\n","");
/* should not exit without closing acus ?
			exit(0);
*/
			cleanup(0);
		}
		for (;;) {
			ret = imsg(msg, Ifn);
			if (ret != 0) {
		DEBUG(4,"Bad return from imsg, ret=%d\n", ret);
				alarm(0);
#ifdef	SYSIII
				ret = ioctl(0, TCSETA, &Savettyb);
#else
				ret = ioctl(0, TIOCSETP, &Savettyb);
#endif
/*
				exit(0);
*/
				cleanup(0);
			}
			if (msg[0] == 'S')
				break;
		}
		alarm(0);
		q = &msg[1];
		p = pskip(q);
		sprintf(Rmtname, "%.7s", q);
		DEBUG(4, "sys-%s\n", Rmtname);
		if (mlock(Rmtname)) {
			omsg('R', "LCK", Ofn);
			cleanup(0);
		}
		sprintf(Spool, "%s/sys/%s", SPOOL, Rmtname);
		ret = callcheck(Loginuser, Rmtname);
		if (ret==1) {
			signal(SIGINT, SIG_IGN);
			signal(SIGHUP, SIG_IGN);
			omsg('R', "CB", Ofn);
			logent("CALLBACK", "REQUIRED");
			/* point Spool to per system spool directory */
			mkspname(Rmtname);
			/*  set up for call back  */
			subchdir(Spool);
			strcpy(Wrkdir, Spool);
			systat(Rmtname, SS_CALLBACK, "CALL BACK");
			gename(CMDPRE, Rmtname, 'C', file);
			close(creat(subfile(file), 0666));
			xuucico(Rmtname);
			cleanup(0);
		}
		else if (ret==2) {
			logent("LOGIN VS MACHINE", "FAILED");
			systat(Rmtname, SS_BADLOGIN, "LOGIN/MACHINE");
			omsg('R', "LOGIN", Ofn);
			cleanup(0);
		     }
		mkspname(Rmtname);  /* determine Spool directory */
		subchdir(Spool);
		fclose(stderr);
		fopen(RMTDEBUG, "w");
		strcpy(Wrkdir, Spool);
		seq = 0;
		while (*p == '-') {
			q = pskip(p);
			switch(*(++p)) {
			case 'g':
				Pkdrvon = 1;
				break;
			case 'x':
				Debug = atoi(++p);
				if (Debug <= 0)
					Debug = 1;
				break;
			case 'X':
				Pkdebug = atoi(++p);
				if (Pkdebug <= 0)
					Pkdebug = 1;
				break;
			case 'Q':
				seq = atoi(++p);
				break;
			default:
				break;
			}
			p = q;
		}
		if (callok(Rmtname) == SS_BADSEQ) {
			logent("BADSEQ", "PREVIOUS");
			omsg('R', "BADSEQ", Ofn);
			cleanup(0);
		}
		if ((ret = gnxseq(Rmtname)) == seq) {
			omsg('R', "OK", Ofn);
			cmtseq();
		}
		else {
			systat(Rmtname, SS_HANDSHAKE, "HANDSHAKE FAILED");
			logent("BAD SEQ", "HANDSHAKE FAILED");
			ulkseq();
			omsg('R', "BADSEQ", Ofn);
			cleanup(0);
		}
		ttyn = ttyname(Ifn);
		if (ttyn != NULL)
			chmod(ttyn, 0600);
	}
loop:
	if (!onesys) {
		ret = gnsys(Rmtname, CMDPRE);
		if (ret == FAIL)
			cleanup(100);
		if (ret == 0)
			cleanup(0);
	}
	else if (!force && Role == MASTER && callok(Rmtname) != 0) {
		logent("SYSTEM STATUS", "CAN NOT CALL");
		cleanup(0);
	}

	sprintf(wkpre, "%c.%.7s", CMDPRE, Rmtname);
	mkspname(Rmtname); 
	subchdir(Spool);
	strcpy(Wrkdir, Spool);

	if (Role == MASTER) {
		/*  master part */
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		if (!iswrk(file, "chk", Spool, wkpre) && !onesys) {
			logent(Rmtname, "NO WORK");
			goto next;
		}
		if (Ifn != -1 && Role == MASTER) {
			write(Ofn, EOTMSG, strlen(EOTMSG));
			clsacu();
/*
	clsacu should close Dnf which is == Ofn == Ifn.
			close(Ofn);
			close(Ifn);
*/
			Ifn = Ofn = -1;
			rmlock(CNULL);
			sleep(3);
		}
		sprintf(msg, "call to %s ", Rmtname);
		if (mlock(Rmtname) != 0) {
			logent(msg, "LOCKED");
			systat(Rmtname, SS_OK, "STILL TALKING");
			goto next;
		}
		Ofn = Ifn = conn(Rmtname);
		if (Ofn < 0) {
			logent(msg, "FAILED");
			UB_SST(-Ofn);
			systat(Rmtname, Stattype[-Ofn],
				Stattext[-Ofn]);
			clsacu();	
			goto next;
		}
		else {
			logent(msg, "SUCCEEDED");
			systat(Rmtname, SS_CONN_OK, "CONNECT SUCCEEDED");
			UB_SST(ub_ok);
		}
	
		if (setjmp(Sjbuf)) {
			systat(Rmtname, SS_BADCONNECT, "CONNECT TIMEOUT");
			DEBUG(4," timeout-Ofn=%d\n",Ofn);
			goto next;
		}
		signal(SIGALRM, timeout);
		alarm(2 * MAXMSGTIME);
		for (;;) {
			ret = imsg(msg, Ifn);
			if (ret != 0) {
				alarm(0);
				systat(Rmtname, SS_OK, "NO START CHAR");
				goto next;
			}
			if (msg[0] == 'S')
				break;
		}
		alarm(MAXMSGTIME);
		seq = gnxseq(Rmtname);
		sprintf(msg, "%.7s -Q%d %s", Myname, seq, rflags);
#ifdef NEWLDISC
		ldisc = HCLDISC;
		DEBUG(4,"Switching to new line discipline\n","");
/*  Ifn == Ofn  therefore only have to switch one
		if ((ret = ioctl(Ifn,TIOCSETD,&ldisc)) < 0) 
			ASSERT_NOFAIL(ret != -1 ,"CAN NOT SWITCH1 LDISC","errno", errno);
*/
		if ((ret = ioctl(Ofn,TIOCSETD,&ldisc)) < 0) 
			ASSERT_NOFAIL(ret != -1 ,"CAN NOT SWITCH2 LDISC","errno", errno);
#endif
		omsg('S', msg, Ofn);
		for (;;) {
			ret = imsg(msg, Ifn);
			DEBUG(4, "msg-%s\n", msg);
			if (ret != 0) {
				alarm(0);
				ulkseq();
				systat(Rmtname, SS_OK, " NO RECEIVE ACK");
				goto next;
			}
			if (msg[0] == 'R')
				break;
		}
		alarm(0);
		if (msg[1] == 'B') {
			/* bad sequence */
			logent("BAD SEQ", "HANDSHAKE FAILED");
			systat(Rmtname, SS_BADSEQ, "BAD SEQUENCE");
			ulkseq();
			goto next;
		}
		if (strcmp(&msg[1], "OK") != SAME)  {
			logent(&msg[1], "HANDSHAKE FAILED");
			systat(Rmtname, SS_OK, "HANDSHAKE FAILED");
			ulkseq();
			goto next;
		}
		cmtseq();
	}
	DEBUG(1, " Rmtname %s, ", Rmtname);
	DEBUG(1, "Role %s,  ", Role ? "MASTER" : "SLAVE");
	DEBUG(1, "Ifn - %d, ", Ifn);
	DEBUG(1, "Loginuser - %s\n", Loginuser);

	alarm(MAXMSGTIME);
	if (setjmp(Sjbuf))
		goto Failure;
	ret = startup(Role);
	alarm(0);
	if (ret != SUCCESS) {
Failure:
		logent("startup", "FAILED");
		systat(Rmtname, SS_FAIL, "STARTUP FAILED");
		goto next;
	}
	else {
		logent("startup", "OK");
		systat(Rmtname, SS_INPROGRESS, "TALKING");
		ret = cntrl(Role, wkpre);
		DEBUG(1, "cntrl - %d\n", ret);
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGALRM, timeout);
		if (ret == 0) {
			systat(Rmtname, SS_OK, "CONVERSATION FINI");
#ifdef ULTRIX
			US_SST(us_s_ok);
#endif
			logent("conversation complete", "OK");
		}
		else {
			logent("conversation complete", "FAILED");
			systat(Rmtname, SS_OK, "CONVERSATION FAILED");
		}
		alarm(MAXMSGTIME);
		omsg('O', "OOOOO", Ofn);
		DEBUG(4, "send OO %d,", ret);
		if (!setjmp(Sjbuf)) {
			for (;;) {
				omsg('O', "OOOOO", Ofn);
				ret = imsg(msg, Ifn);
				if (ret != 0)
					break;
				if (msg[0] == 'O')
					break;
			}
		}
		alarm(0);
	}
next:
	/* just to make sure everything is closed */
/*
	logcls();
	for(i=3; i<20; i++)
		close(i);
*/
	if (!onesys) {
		goto loop;
	}
	cleanup(0);
}


#ifndef	SYSIII
struct sgttyb Hupvec;
#endif

/***
 *	cleanup(code)	cleanup and exit with "code" status
 *	int code;
 */

cleanup(code)
int code;
{
	int ret;
	char *ttyn;
	char msg[75];

	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	rmlock(CNULL);
	logcls();
	if (Role == SLAVE) {
#ifdef	SYSIII
		Savettyb.c_cflag |= HUPCL;
		ret = ioctl(0, TCSETA, &Savettyb);
#else
		/* rti!trt: use more robust hang up sequence */
		ret = ioctl(0, TIOCHPCL, STBNULL);
		ret = ioctl(0, TIOCGETP, &Hupvec);
		Hupvec.sg_ispeed = B0;
		Hupvec.sg_ospeed = B0;
		ret = ioctl(0, TIOCSETP, &Hupvec);
		sleep(2);
		ret = ioctl(0, TIOCSETP, &Savettyb);
#endif
		DEBUG(4, "ret ioctl - %d\n", ret);
		ttyn = ttyname(Ifn);
		if (ttyn != NULL)
			chmod(ttyn, 0600);
	}
		if (Role == MASTER) {
			DEBUG(5,"about to close acu\n", "");
			if (Ofn != -1) 
				write(Ofn, EOTMSG, strlen(EOTMSG));
			/* acu may be open but not connected */
			clsacu();
		} else 
			if (Ofn != -1) {
				close(Ifn);
				close(Ofn);
			}
	
	DEBUG(1, "exit code %d\n", code);
	if (code == 0)
		xuuxqt();
	else {
		sprintf(msg, "ABORTED (%d)", code);
		systat(Rmtname, SS_OK, msg);
	}
	exit(code);
}

/***
 *	onintr(inter)	interrupt - remove locks and exit
 */

onintr(inter)
int inter;
{
	char str[30];
	signal(inter, SIG_IGN);
	sprintf(str, "SIGNAL %d", inter);
	logent(str, "CAUGHT");
	cleanup(inter);
}

/* changed to single version of intrEXIT.  Is this okay? rti!trt */
intrEXIT(signo)
int signo;
{
	char sig[30];
	sprintf(sig, " signal: %d", signo);
	logent(sig, "intrEXIT");
	signal(signo, SIG_DFL);
	setuid(getuid());
	cleanup(signo);
/*
	abort();
*/
}

/***
 *	fixmode(tty)	fix kill/echo/raw on line
 *
 *	return codes:  none
 */

fixmode(tty)
int tty;
{
#ifdef	SYSIII
	struct termio ttbuf;
#else
	struct sgttyb ttbuf;
#endif
	int ret;

#ifdef	SYSIII
	ioctl(tty, TCGETA, &ttbuf);
	ttbuf.c_iflag = ttbuf.c_oflag = ttbuf.c_lflag = (ushort)0;
	ttbuf.c_cflag &= (CBAUD);
	ttbuf.c_cflag |= (CS8|CREAD);
	ttbuf.c_cc[VMIN] = 6;
	ttbuf.c_cc[VTIME] = 1;
	ret = ioctl(tty, TCSETA, &ttbuf);
#else
	ioctl(tty, TIOCGETP, &ttbuf);
	ttbuf.sg_flags = (ANYP | RAW);
	ret = ioctl(tty, TIOCSETP, &ttbuf);
#endif
	ASSERT(ret >= 0, "STTY FAILED", "", ret);
#ifndef	SYSIII
	ioctl(tty, TIOCEXCL, STBNULL);
#endif
	return;
}


/***
 *	timeout()	catch SIGALRM routine
 */

timeout()
{
	DEBUG(1, "timeout - %s\n", Rmtname);
	logent(Rmtname, "TIMEOUT");
	longjmp(Sjbuf, 1);
}

static char *
pskip(p)
register char *p;
{
	while( *p && *p != ' ' )
		++p;
	if( *p ) *p++ = 0;
	return(p);
}
