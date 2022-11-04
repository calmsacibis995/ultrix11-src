
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/


static char *Sccsid = "@(#)aux.c	3.0	(ULTRIX-11)	4/22/86";

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
insque(e, prev)
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
remque(e)
register struct vaxque *e;
{
	e->vq_prev->vq_next = e->vq_next;
	e->vq_next->vq_prev = e->vq_prev;
}
