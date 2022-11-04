
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: "@(#)sigset.c	3.0	4/22/86"
 */
#include <signal.h>
#include <errno.h>
/*
 * signal system call interface package.
 */

#define BYTESPERVEC	4			/* size of machine language vector */
int	errno;
extern	char	mvectors[NSIG][BYTESPERVEC];	/* machine language vector */
int touched[NSIG];
						/* really in I space */
static	int	(*cactions[NSIG])();		/* saved callers signal actions */
static	char	setflg[NSIG];			/* flags for using sigset */
int	(*sigsys())();

int (*
signal(signum, action))()
register int signum;
register int (*action)();
{
	register int (*retval)();

	if (signum <= 0 || signum > NSIG) {
		errno = EINVAL;
		return BADSIG;
	}
	retval = cactions[signum];
	cactions[signum] = action;
	if (action != SIG_IGN && action != SIG_DFL && action != SIG_HOLD)
		if (SIGISDEFER(action))
			action = DEFERSIG(mvectors[signum]);
		else
			action = (int (*)())(int)mvectors[signum];
	action = sigsys(signum, action);
	if (action == SIG_IGN || action == SIG_DFL || action == SIG_HOLD)
		retval = action;
	setflg[signum] = 0;	/* indicate which kind of protocol */
	return retval;
}
/*
 * pretty much like the old signal call - set the action, return the
 *	previous action and remember the action locally
 */
int (*
sigset(signum, action))()
register int signum;
register int (*action)();
{
	register int (*retval)();

	if (signum <= 0 || signum > NSIG) {
		errno = EINVAL;
		return BADSIG;
	}
	retval = cactions[signum];
	cactions[signum] = action;
	if (action != SIG_IGN && action != SIG_DFL && action != SIG_HOLD)
		action = DEFERSIG(mvectors[signum]);
	action = sigsys(signum, action);
	if (action == SIG_IGN || action == SIG_DFL || action == SIG_HOLD)
		retval = action;
	setflg[signum] = 1;
	return retval;
}

/* temporarily hold a signal until further notice - sigpause or sigrelse */

sighold(signum)
register int signum;
{
	if (signum <= 0 || signum > NSIG)
		abort();
	/*
	 *	Bug fix to allow one to call sighold() then sigrelse()
	 *	to hold the default action rather than being forced
	 *	to call sigset (x, SIG_HOLD) then sigrelse.
	 */
	setflg[signum] = 1;
	sigsys(signum, SIG_HOLD);
}

/* atomically release the signal and pause (if none pending) */
/* if no other signal has occurred, the signal will be held upon return */
sigpause(signum)
register signum;
{
	if (signum <= 0 || signum > NSIG || setflg[signum] == 0)
		abort();
	sigsys(signum | SIGDOPAUSE, DEFERSIG(mvectors[signum]));
	/* sigsys(signum , DEFERSIG(mvectors[signum]));
	pause(); */
}

/* re-enable signals after sighold or possibly after sigpause */

sigrelse(signum)
register signum;
{
	if (signum <= 0 || signum > NSIG || setflg[signum] == 0)
		abort();
	sigsys(signum, DEFERSIG(mvectors[signum]));
}

/* ignore these signals */

sigignore(signum)
{
	sigsys(signum, SIG_IGN);
}
/* called by the machine language assist at interrupt time */
/* return value is new signal action (if any) */
int (*
_sigcatch(signum))()
register signum;
{
touched[signum]++;
	(*cactions[signum])(signum);		/* call the C routine */

	if (setflg[signum])
		return DEFERSIG(mvectors[signum]);	/* return new action to set */
	else
		return SIG_DFL;
}

/* machine language assist is at interrupt time, where it vectors in,
 * determining the caught signal and passing it to _sigcatch.
 * its main purpose in life is to ensure that ALL registers are saved.
 * and figuring out which signal is being caught
 */
