
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)clock.c	3.0	4/21/86
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/callo.h>
#include <sys/seg.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/reg.h>

#define	SCHMAG	8/10
int	hz;		/* line frequency, see c.c */
struct callo *callhead, *callfree;

/*
 * clock is called straight from
 * the real time clock interrupt.
 *
 * Functions:
 *	reprime clock
 *	copy *switches to display
 *	implement callouts
 *	maintain user/system times
 *	maintain date
 *	profile
 *	lightning bolt wakeup (every second)
 *	alarm clock signals
 *	jab the scheduler
 */

clock(dev, sp, r1, ov, nps, r0, pc, ps)
dev_t dev;
caddr_t pc;
{
	int a;
	mapinfo	map;
	extern caddr_t waitloc;
	extern char *panicstr;

	/*
	 * restart clock
	 */

	lks->r[0] = 0115;

	/*
	 * ensure normal mapping of kernel data
	 */
	savemap(map);

	/*
	 * display register
	 */

	display();
	/*
	 * callouts
	 * never after panics
	 * if none, just continue
	 * else update first non-zero time
	 */
	if (panicstr == (char *)0) {
		register struct callo *p1, *p2;

		if ((p1 = callhead) == NULL)
			goto out;
		do {
			if (p1->c_time > 0) {
				p1->c_time--;
				break;
			}
		} while (p1 = p1->c_next);

		/*
		 * if ps is high, just return
		 */
		if (BASEPRI(ps))
			goto out;

		/*
		 * callout
		 */

		spl5();	/* berkeley 2.9 doesn't have this */
		p1 = callhead;
		if (p1->c_time <= 0) {
			do {
				(*p1->c_func)(p1->c_arg);
				p2 = p1;
				p1 = p1->c_next;
			} while (p1 && p1->c_time <= 0);
			p2->c_next = callfree;
			callfree = callhead;
			callhead = p1;
		}
	}

	/*
	 * lightning bolt time-out
	 * and time of day
	 */
out:
	a = 0;		/* user */
	if (USERMODE(ps)) {
		u.u_utime++;
		if(u.u_prof.pr_scale)
			addupc(pc, &u.u_prof, 1);
		if(u.u_procp->p_nice > NZERO)
			a++;		/* nice */
	} else {
		a = 2;		/* system */
		if (pc == waitloc)
			a++;	/* idle */
		u.u_stime++;
	}
	cp_time[a]++;	/* tally cpu time */
/*
 * Disk I/O instrumentation
 */

	{
		register struct ios *dp;
		register int i, j;
		int k;

		k = 0;
		for (i = DK_NC-1; i >= 0; --i) {
			if (dp = dk_iop[i]) {
				for (j = dk_nd[i]; j > 0; dp++, --j)
					if(dp->dk_busy) {
						dp->dk_time++;
						k++;
					}
			}
		}
		if(k && (a == 3))	/* tally idle with I/O active */
			cp_time[4]++;
	}
	{
		register struct proc *pp;
		pp = u.u_procp;
		/*
		 * el_c_pid is the process ID of the
		 * error log copy process (elc),
		 * DO NOT NICE this process !
		 */
		if((++pp->p_cpu == 0) || (pp->p_pid == el_c_pid))
			pp->p_cpu--;
	}
	if (++lbolt >= hz && !(BASEPRI(ps)))
	{
		register struct proc *pp;
		register int s;

		lbolt -= hz;
		++time;
		spl1();
#ifdef	UCB_LOAD
		if ((time&03) == 0)	/* every 4 seconds */
			meter();
#endif	UCB_LOAD
		runrun++;
		wakeup((caddr_t)&lbolt);
		for(pp = &proc[0]; pp <= maxproc; pp++)
		    if (pp->p_stat && pp->p_stat<SZOMB) {
			if (pp->p_time != 127)
				pp->p_time++;
			if(pp->p_clktim && --pp->p_clktim == 0)
#ifdef	SELECT
				/*
				 * If process has clock counting down, and it
				 * expires, set it running (if this is a
				 * tsleep()), or give it an SIGALRM (if the user
				 * process is using alarm signals.
				 */
				if (pp->p_flag & STIMO) {
					s = spl6();
					switch (pp->p_stat) {
					case SSLEEP:
						setrun(pp);
						break;
					case SSTOP:
						unsleep(pp);
						break;
					}
					pp->p_flag &= ~STIMO;
					splx(s);
				} else
#endif	SELECT
					psignal(pp, SIGALRM);
			s = (pp->p_cpu & 0377)*SCHMAG + pp->p_nice - NZERO;
			if(s < 0)
				s = 0;
			if(s > 255)
				s = 255;
			pp->p_cpu = s;
			if(pp->p_pri >= PUSER)
				setpri(pp);
		    }
		if(runin!=0) {
			runin = 0;
			wakeup((caddr_t)&runin);
		}
	}
	restormap(map);
}

/*
 * timeout is called to arrange that
 * fun(arg) is called in tim/HZ seconds.
 * An entry is sorted into the callout
 * structure. The time in each structure
 * entry is the number of HZ's more
 * than the previous entry.
 * In this way, decrementing the
 * first entry has the effect of
 * updating all entries.
 *
 * The panic is there because there is nothing
 * intelligent to be done if an entry won't fit.
 */
timeout(fun, arg, tim)
int (*fun)();
caddr_t arg;
{
	register struct callo *p1, *p2;
	register int t;
	int s;
	mapinfo map;	/* save map so callout[] can be after mb_end */

	savemap(map);
	t = tim;
	s = spl7();
	if (callfree == NULL) {
		restormap(map);
		panic("Timeout table overflow");
	}
	for (p2 = NULL, p1 = callhead; p1; p2 = p1, p1 = p1->c_next) {
		if (p1->c_time > t) {
			p1->c_time -= t;
			break;
		}
		t -= p1->c_time;
	}
	p1 = callfree;
	callfree = p1->c_next;
	if (p2) {
		p1->c_next = p2->c_next;
		p2->c_next = p1;
	} else {
		p1->c_next = callhead;
		callhead = p1;
	}
	p1->c_time = t;
	p1->c_func = fun;
	p1->c_arg = arg;
	splx(s);
	restormap(map);
}

calloinit()
{
	register struct callo *p;
	callhead = (struct callo *)NULL;
	callfree = (struct callo *)NULL;
	for (p = &callout[0]; p < &callout[ncall]; p++) {
		p->c_next = callfree;
		callfree = p;
	}
}

#ifdef	UCB_LOAD

/*
 * Compute Tenex style load average.  This code based on the 2.9 code,
 * which is adapted from similar code by Bill Joy on the Vax system.
 * There are two major changes: 1) We avoid floating point since not all
 * pdp-11's have it.  "floating point" numbers here are stored in a 16
 * bit short, with 8 bits on each side of the decimal point.  Some partial
 * products will have 16 bits to the right.  See the comments below on how
 * the equation was devived.  2) We do the proc counts every 4 seconds
 * instead of every 5, and we do the averaging every 16 seconds.
 *
 * We count every 4 seconds, because it's easier to see if 4 seconds
 * have gone by (do a bit test) than if 5 seconds have gone by (have
 * to call lrem).  Since we only have 8 bits to hold the e^x values,
 * and they are so close to 1, by averaging every 4 values and computing
 * the load average every 16 seconds, we can use e^x values a bit
 * farther from 1, giving us more precision in the load average (especially
 * in the 15 minute load average).
 */

	/*
	 * The Vax algorithm is:
	 *
	 * /*
	 *  * Constants for averages over 1, 5, and 15 minutes
	 *  * when sampling at 5 second intervals.
	 *  * /
	 * double	cexp[3] = {
	 *	0.9200444146293232,	/* exp(-1/12) * /
	 *	0.9834714538216174,	/* exp(-1/60) * /
	 *	0.9944598480048967,	/* exp(-1/180 * /
	 * };
	 *
	 * /*
	 *  * Compute a tenex style load average of a quantity on
	 *  * 1, 5, and 15 minute intervals.
	 *  * /
	 * loadav(avg, n)
	 *	register double *avg;
	 *	int n;
	 * {
	 *	register int i;
	 *
	 *	for (i = 0; i < 3; i++)
	 *		avg[i] = cexp[i] * avg[i] + n * (1.0 - cexp[i]);
	 * }
	 */

short avenrun[3];	/* internal load average in psuedo-floating point */

long cexp[3] = {
	0304,	/* 256*exp(-4/15) */
	0363,	/* 256*exp(-4/75) */
	0373	/* 256*exp(-4/225) */
};

static			/* so we don't have to go through the thunk */
meter()
{
	static nrun = 0, cycle = 0;
	register n = 0;

	{
		register struct proc *p;

		for (p = &proc[1]; p <= maxproc; p++) {
			if (p->p_stat) {
				switch(p->p_stat) {

				case SSLEEP:
				case SSTOP:
					if (p->p_pri <= PZERO)
						n++;
					break;
				case SRUN:
				case SIDL:
					n++;
					break;
				}
			}
		}
	}
	if ((++cycle)&3) {
		nrun += n;
	} else {
		register short *avg = avenrun;
		register int i;

		n += nrun;
		nrun = 0;

		/*
		 * rn = regular number
		 * pfp = psudeo-floating point number.
		 * Given:	1) rn * 256 = pfp
		 *		2) rn * rn = rn
		 * pfp = rn * 256
		 *     = rn * rn * 256
		 *     = rn * (256 / 256) * rn * 256
		 *     = (rn * 256) / 256 * (rn * 256)
		 *     = pfp / 256 * pfp
		 *     = (pfp * pfp)/256
		 * Therefore: when multiplying two pfp's, the
		 * result must be divided by 256 to yeild a pfp.
		 * pfp + pfp = rn * 256 + rn * 256
		 *	     = (rn + rn) * 256
		 *	     = rn * 256
		 *	     = pfp
		 * Therefore: two pfp's add up to another pfp.
		 * 
		 * w = cexp[i] (in pfp)
		 * v = *avg (in pfp)
		 * n' = new value to weight in (not in pfp)
		 * 
		 * w'*v' + n'*(1-w')
		 *	convert n',w',v' & 1 to pfp, taking into
		 *	account where we multiply two pfp's.
		 * (w*v)/256 + (n'*256*(256 - w))/256
		 * (w*v + n'*256*256 - n'*256*w)/256
		 * ((w*v - n'*256*w) + n'*256*256)/256
		 * (w*(v - n'*256) + n'*256*256)/256
		 * 	n' is sum of 4 values, divide by 4 to get the average
		 * (w*(v - n'/4*256) + n'/4*256*256)/256
		 * (w*(v - n'*64) + n'*64*256)/256
		 * (w*(v - n'<<6) + n'<<14)>>8
		 */
		for (i = 0; i < 3; avg++, i++) {
		    *avg = (cexp[i] * ((*avg)-(n<<6)) + (((long)n)<<14)) >> 8;
		}
	}
}
#endif
