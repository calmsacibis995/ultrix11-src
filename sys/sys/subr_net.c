
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)subr_net.c	3.0	4/21/86
 * Various routines used by the network code.
 */
#include <sys/param.h>
#include <sys/map.h>
#include <net/netisr.h>
#include <netinet/in_systm.h>
#include <sys/mbuf.h>

int	netoff = 0;

/*
 * Initialize network code. Called from main();
 */
netinit()
{

	if (netoff)
		return;

	MAPSAVE();
	mbinit();
	netattach();	/* in c.c, calls hardware attach routines */
	domaininit();
#ifdef	INET
	loattach();
	ifinit();
#endif	INET
	MAPREST();
}

/*
 * Entered via software interrupt vector at spl1. Check netisr bit array
 * for tasks requesting service.
 */
netintr()
{
	int onetisr;
	mapinfo map;
	extern	char *panicstr;

	if (panicstr) {
		/*
		 * Don't process network interrupts if we're in panic mode.
		 */
		netisr = 0;
		return;
	}

	savemap(map);
	while (spl7(), (onetisr = netisr)) {
		netisr = 0;
		splnet();
		if (onetisr & (1 << NETISR_RAW))
			rawintr();
		if (onetisr & (1 << NETISR_IP))
			ipintr();
#ifdef	LAT
		if (onetisr & (1 << NETISR_LAT))
			latintr();
#endif	LAT
	}
	restormap(map);
}

/*
 * Compare bytes, same as VAX cmpc3.
 * This could be optimized for speed by
 * doing word compares when appropiate.
 */
bcmp(s1, s2, n)
register char *s1, *s2;
register n;
{
	do
		if (*s1++ != *s2++)
			break;
	while (--n);
	return(n);
}

/*
 * Queue format expected by VAX queue instructions.
 */

struct vaxque {
	struct vaxque *vq_next;
	struct vaxque *vq_prev;
};

/*
 * Insert an entry onto queue.
 */
_insque(e, prev)
register struct vaxque *e, *prev;
{
	e->vq_prev = prev;
	e->vq_next = prev->vq_next;
	prev->vq_next->vq_prev = e;
	prev->vq_next = e;
}

/*
 * Remove an entry from queue.
 */
_remque(e)
register struct vaxque *e;
{
	e->vq_prev->vq_next = e->vq_next;
	e->vq_next->vq_prev = e->vq_prev;
}
