
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#ifndef lint
static char sccsid[] = "@(#)tipout.c	3.0	4/22/86";
#endif

#include "tip.h"
/*
 * tip
 *
 * lower fork of tip -- handles passive side
 *  reading from the remote host
 */

static	jmp_buf sigbuf;

/*
 * TIPOUT wait state routine --
 *   sent by TIPIN when it wants to posses the remote host
 */
intIOT()
{

#ifdef	V7M-11
	signal(SIGIOT, intIOT);
#endif	V7M-11
	write(repdes[1],&ccc,1);
	read(fildes[0], &ccc,1);
	longjmp(sigbuf, 1);
}

/*
 * Scripting command interpreter --
 *  accepts script file name over the pipe and acts accordingly
 */
intEMT()
{
	char c, line[256];
	register char *pline = line;
	char reply;

#ifdef	V7M-11
	signal(SIGEMT, intEMT);
#endif	V7M-11
	read(fildes[0], &c, 1);
	while (c != '\n') {
		*pline++ = c;
		read(fildes[0], &c, 1);
	}
	*pline = '\0';
	if (boolean(value(SCRIPT)) && fscript != NULL)
		fclose(fscript);
	if (pline == line) {
		boolean(value(SCRIPT)) = FALSE;
		reply = 'y';
	} else {
		if ((fscript = fopen(line, "a")) == NULL)
			reply = 'n';
		else {
			reply = 'y';
			boolean(value(SCRIPT)) = TRUE;
		}
	}
	write(repdes[1], &reply, 1);
	longjmp(sigbuf, 1);
}

intTERM()
{

	if (boolean(value(SCRIPT)) && fscript != NULL)
		fclose(fscript);
	exit(0);
}

intSYS()
{
#ifdef	V7M-11
	signal(SIGSYS, intSYS);
#endif	V7M-11
	boolean(value(BEAUTIFY)) = !boolean(value(BEAUTIFY));
	longjmp(sigbuf, 1);
}

/*
 * ****TIPOUT   TIPOUT****
 */
#ifdef	V7M-11
int dosig;		/* signal that needs to be processed */
#endif	V7M-11
tipout()
{
	char buf[BUFSIZ];
	register char *cp;
	register int cnt;
	int omask;

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGEMT, intEMT);		/* attention from TIPIN */
	signal(SIGTERM, intTERM);	/* time to go signal */
	signal(SIGIOT, intIOT);		/* scripting going on signal */
	signal(SIGHUP, intTERM);	/* for dial-ups */
	signal(SIGSYS, intSYS);		/* beautify toggle */
	setjmp(sigbuf);
	for (omask = 0;; sigsetmask(omask)) {
#ifdef	V7M-11
		if (dosig) {
			register int i = dosig;
			dosig = 0;
			kill(getpid(), i);
		}
#endif	V7M-11
		cnt = read(FD, buf, BUFSIZ);
		if (cnt <= 0)
			continue;
#define	mask(s)	(1 << ((s) - 1))
#define	ALLSIGS	mask(SIGEMT)|mask(SIGTERM)|mask(SIGIOT)|mask(SIGSYS)
		omask = sigblock(ALLSIGS);
		for (cp = buf; cp < buf + cnt; cp++)
			*cp &= 0177;
		write(1, buf, cnt);
		if (boolean(value(SCRIPT)) && fscript != NULL) {
			if (!boolean(value(BEAUTIFY))) {
				fwrite(buf, 1, cnt, fscript);
				continue;
			}
			for (cp = buf; cp < buf + cnt; cp++)
				if ((*cp >= ' ' && *cp <= '~') ||
				    any(*cp, value(EXCEPTIONS)))
					putc(*cp, fscript);
		}
	}
}

#ifdef	V7M-11
int sigmsk;		/* signals currently ignoring */
int (*sigblk[NSIG])();	/* signal value before ignoring */
int gotsig;		/* signals gotten while ignoring them */
sigsetmask(nmask)
register int nmask;
{
	register int	i;
	register int	retval = sigmsk;

	for (i = 0; i < NSIG; i++) {
		if (nmask & 1<<i) {
			if (sigblk[i] == NULL)
				sigblk[i] = signal(i+1, SIG_IGN);
		} else if (sigmsk & 1<<i) {
			signal(i+1, sigblk[i]);
			sigblk[i] = NULL;
		}
	}
	sigmsk = nmask;
	nmask = gotsig & ~sigmsk;
	dosig = 0;
	if (nmask) {
		for (i = 0; i < NSIG; i++) {
			if (nmask & (1<<i))
				break;
		}
		gotsig &= ~(1<<i);
		dosig = i+1;
	}
	return (retval);
}

sigblock(sigs)
int sigs;
{
	register int	i;
	register int	retval = sigmsk;
	int	sig_mark();

	for (i = 0; i < NSIG; i++) {
		if (sigs & 1<<i) {
			if (sigblk[i] == NULL)
				sigblk[i] = signal(i+1, sig_mark);
		}
	}
	sigmsk |= sigs;
	return (retval);
}

sig_mark(sig)
int sig;
{
	signal(sig, sig_mark);
	gotsig |= 1<<(sig -1);
}
#endif	V7M-11
