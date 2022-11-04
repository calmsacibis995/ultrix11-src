
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)msg.c	3.2	10/8/87
 */

/*
 * Inter-Process Communication Message Facility.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <signal.h>
#include <sys/user.h>
#include <sys/seg.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <errno.h>
#include <sys/map.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/systm.h>

extern struct map	msgmap[];	/* msg allocation map */
extern struct msqid_ds	msgque[];	/* msg queue headers */
extern struct msg	msgh[];		/* message headers */
extern struct msginfo	msginfo;	/* message parameters */
struct msg		*msgfp;	/* ptr to head of free header list */
unsigned		msgbase;	/* base address of message buffer */
long			msg;
extern time_t		time;		/* system idea of date */

struct msqid_ds		*ipcget(),
			*msgconv();

/* Convert bytes to msg segments. */
#define	btoq(X)	((X + msginfo.msgssz - 1) / msginfo.msgssz)

/* Choose appropriate message copy routine. */
#define MOVE	msgpimove

/*
 * msgconv - Convert a user supplied message queue id into a ptr to a
 *	msqid_ds structure.
 */

static struct msqid_ds *
msgconv(id)
register int	id;
{
	register struct msqid_ds	*qp;	/* ptr to associated q slot */

	qp = &msgque[id % msginfo.msgmni];
	if((qp->msg_perm.mode & IPC_ALLOC) == 0 ||
		id / msginfo.msgmni != qp->msg_perm.seq) {
		u.u_error = EINVAL;
		return(NULL);
	}
	return(qp);
}

/*
 * msgctl - Msgctl system call.
 */

static
msgctl()
{
	register struct a {
		int		msgid,
				cmd;
		struct msqid_ds	*buf;
	}		*uap = (struct a *)u.u_ap;
	struct msqid_ds			ds;	/* queue work area */
	register struct msqid_ds	*qp;	/* ptr to associated q */

	if((qp = msgconv(uap->msgid)) == NULL)
		return;
	u.u_rval1 = 0;
	switch(uap->cmd) {
	case IPC_RMID:
		if(u.u_uid != qp->msg_perm.uid && u.u_uid != qp->msg_perm.cuid
			&& !suser())
			return;
		while(qp->msg_first)
			msgfree(qp, NULL, qp->msg_first);
		qp->msg_cbytes = 0;
		if(uap->msgid + msginfo.msgmni < 0)
			qp->msg_perm.seq = 0;
		else
			qp->msg_perm.seq++;
		if(qp->msg_perm.mode & MSG_RWAIT)
			wakeup(&qp->msg_qnum);
		if(qp->msg_perm.mode & MSG_WWAIT)
			wakeup(qp);
		qp->msg_perm.mode = 0;
		return;
	case IPC_SET:
		if(u.u_uid != qp->msg_perm.uid && u.u_uid != qp->msg_perm.cuid
			 && !suser())
			return;
		if(copyin(uap->buf, &ds, sizeof(ds))) {
			u.u_error = EFAULT;
			return;
		}
		if(ds.msg_qbytes > qp->msg_qbytes && !suser())
			return;
		qp->msg_perm.uid = ds.msg_perm.uid;
		qp->msg_perm.gid = ds.msg_perm.gid;
		qp->msg_perm.mode = (qp->msg_perm.mode & ~0777) |
			(ds.msg_perm.mode & 0777);
		qp->msg_qbytes = ds.msg_qbytes;
		qp->msg_ctime = time;
		return;
	case IPC_STAT:
		if(ipcaccess(&qp->msg_perm, MSG_R))
			return;
		if(copyout(qp, uap->buf, sizeof(*qp))) {
			u.u_error = EFAULT;
			return;
		}
		return;
	default:
		u.u_error = EINVAL;
		return;
	}
}

/*
 * msgfree - Free up space and message header, relink pointers on q,
 * and wakeup anyone waiting for resources.
 */

static
msgfree(qp, pmp, mp)
register struct msqid_ds	*qp;	/* ptr to q of mesg being freed */
register struct msg		*mp,	/* ptr to msg being freed */
				*pmp;	/* ptr to mp's predecessor */
{
	/* Unlink message from the q. */
	if(pmp == NULL)
		qp->msg_first = mp->msg_next;
	else
		pmp->msg_next = mp->msg_next;
	if(mp->msg_next == NULL)
		qp->msg_last = pmp;
	qp->msg_qnum--;
	if(qp->msg_perm.mode & MSG_WWAIT) {
		qp->msg_perm.mode &= ~MSG_WWAIT;
		wakeup(qp);
	}

	/* Free up message text. */
	if(mp->msg_ts)
		mfree(msgmap, btoq(mp->msg_ts), mp->msg_spot + 1);

	/* Free up header */
	mp->msg_next = msgfp;
	if(msgfp == NULL)
		wakeup(&msgfp);
	msgfp = mp;
}

/*
 * msgget - Msgget system call.
 */

static
msgget()
{
	register struct a {
		key_t	key;
		int	msgflg;
	}	*uap = (struct a *)u.u_ap;
	register struct msqid_ds	*qp;	/* ptr to associated q */
	int				s;	/* ipcget status return */

	if((qp = ipcget(uap->key, uap->msgflg, msgque, msginfo.msgmni, sizeof(*qp), &s))
		== NULL)
		return;

	if(s) {
		/* This is a new queue.  Finish initialization. */
		qp->msg_first = qp->msg_last = NULL;
		qp->msg_qnum = 0;
		qp->msg_qbytes = msginfo.msgmnb;
		qp->msg_lspid = qp->msg_lrpid = 0;
		qp->msg_stime = qp->msg_rtime = 0;
		qp->msg_ctime = time;
	}
	u.u_rval1 = qp->msg_perm.seq * msginfo.msgmni + (qp - msgque);
}

/*
 * msginit - Called by main(main.c) to initialize message queues.
 */

msginit()
{
	register int		i;	/* loop control */
	register struct msg	*mp;	/* ptr to msg begin linked */
	register int		bs;	/* message buffer size */
	long limit;			/* Maximum message buffer size */

	limit = (long)64*1024 - 63;
	/* Allocate physical memory for message buffer. */
	if((long)msginfo.msgseg * msginfo.msgssz >= limit) {
		printf("\nMessage buffer greater than 64K, Unallocated\n");
		msginfo.msgseg = 0;
		bs = 0;
	}
	else 
	    if((msgbase = malloc(coremap,
	        bs=(int)btoc((long)msginfo.msgseg * msginfo.msgssz))) == 0) {
	        printf("Can't allocate message buffer.\n");
	        msginfo.msgseg = 0;
	    }
	msg = ctob((long)(unsigned)msgbase);
	MAPINIT(msgmap, msginfo.msgmap);
	mfree(msgmap, msginfo.msgseg, 1);
	for(i = 0, mp = msgfp = msgh;++i < msginfo.msgtql;mp++)
		mp->msg_next = mp + 1;
	return(bs);
}

/*
 * msgpimove - PDP 11 pimove interface for possibly large copies.
 */

static
msgpimove(base, count, mode)
long			base;	/* base (spot * msginfo.msgssz) in msgmap */
				/* in bytes */
register unsigned	count;	/* byte count */
int			mode;	/* transfer mode */
{
	register unsigned	tcount;	/* current transfer count */

	while(u.u_error == 0 && count) {
		tcount = count > 8064 ? 8064 : count;
		pimove(base, tcount, mode);
		base += tcount;
		u.u_base += tcount;
		count -= tcount;
	}
}

/*
 * msgrcv - Msgrcv system call.
 */

static
msgrcv()
{
	register struct a {
		int		msqid;
		struct msgbuf	*msgp;
		int		msgsz;
		long		msgtyp;
		int		msgflg;
	}	*uap = (struct a *)u.u_ap;
	register struct msg		*mp,	/* ptr to msg on q */
					*pmp,	/* ptr to mp's predecessor */
					*smp,	/* ptr to best msg on q */
					*spmp;	/* ptr to smp's predecessor */
	register struct msqid_ds	*qp;	/* ptr to associated q */
	int				sz;	/* transfer byte count */

	if((qp = msgconv(uap->msqid)) == NULL)
		return;
	if(ipcaccess(&qp->msg_perm, MSG_R))
		return;
	if(uap->msgsz < 0) {
		u.u_error = EINVAL;
		return;
	}
	smp = spmp = NULL;
findmsg:
	pmp = NULL;
	mp = qp->msg_first;
	if(uap->msgtyp == 0) {
		smp = mp;
	} else {
		for(;mp;pmp = mp, mp = mp->msg_next) {
			if(uap->msgtyp > 0) {
				if(uap->msgtyp != mp->msg_type)
					continue;
				smp = mp;
				spmp = pmp;
				break;
			}
			if(mp->msg_type <= -uap->msgtyp) {
				if(smp && smp->msg_type <= mp->msg_type)
					continue;
				smp = mp;
				spmp = pmp;
			}
		}
	}
	if(smp) {
		if(uap->msgsz < smp->msg_ts)
			if(!(uap->msgflg & MSG_NOERROR)) {
				u.u_error = E2BIG;
				return;
			} else
				sz = uap->msgsz;
		else
			sz = smp->msg_ts;
		copyout(&smp->msg_type, uap->msgp, sizeof(smp->msg_type));
		if(u.u_error)
			return;
		if(sz) {
			u.u_base = (caddr_t)uap->msgp + sizeof(smp->msg_type);
			u.u_segflg = 0;
			MOVE(msg + msginfo.msgssz * smp->msg_spot,
				sz, B_READ);
			if(u.u_error)
				return;
		}
		u.u_rval1 = sz;
		qp->msg_cbytes -= smp->msg_ts;
		qp->msg_lrpid = u.u_procp->p_pid;
		qp->msg_rtime = time;
		curpri = PMSG;
		msgfree(qp, spmp, smp);
		return;
	}
	if(uap->msgflg & IPC_NOWAIT) {
		u.u_error = ENOMSG;
		return;
	}
	qp->msg_perm.mode |= MSG_RWAIT;
	sleep(&qp->msg_qnum, PMSG);
	if(msgconv(uap->msqid) == NULL) {
		u.u_error = EIDRM;
		return;
	}
	goto findmsg;
}

/*
 * msgsnd - Msgsnd system call.
 */

static
msgsnd()
{
	register struct a {
		int		msqid;
		struct msgbuf	*msgp;
		int		msgsz;
		int		msgflg;
	}	*uap = (struct a *)u.u_ap;
	register struct msqid_ds	*qp;	/* ptr to associated q */
	register struct msg		*mp;	/* ptr to allocated msg hdr */
	register int			cnt,	/* byte count */
					spot;	/* msg pool allocation spot */
	long				type;	/* msg type */

	if((qp = msgconv(uap->msqid)) == NULL)
		return;
	if(ipcaccess(&qp->msg_perm, MSG_W))
		return;
	if((cnt = uap->msgsz) < 0 || cnt > msginfo.msgmax) {
		u.u_error = EINVAL;
		return;
	}
	copyin(uap->msgp, &type, sizeof(type));
	if(u.u_error)
		return;
	if(type < 1) {
		u.u_error = EINVAL;
		return;
	}
getres:
	/* Be sure that q has not been removed. */
	if(msgconv(uap->msqid) == NULL) {
		u.u_error = EIDRM;
		return;
	}

	/* Allocate space on q, message header, & buffer space. */
	if(cnt + qp->msg_cbytes > qp->msg_qbytes) {
		if(uap->msgflg & IPC_NOWAIT) {
			u.u_error = EAGAIN;
			return;
		}
		qp->msg_perm.mode |= MSG_WWAIT;
		sleep(qp, PMSG );
		/* if(sleep(qp, PMSG | PCATCH)) {
			u.u_error = EINTR;
			qp->msg_perm.mode &= ~MSG_WWAIT;
			wakeup(qp);
			return;
		} */
		goto getres;
	}
	if(msgfp == NULL) {
		if(uap->msgflg & IPC_NOWAIT) {
			u.u_error = EAGAIN;
			return;
		}
		sleep(&msgfp, PMSG );
		/*if(sleep(&msgfp, PMSG | PCATCH)) {
			u.u_error = EINTR;
			return;
		} */
		goto getres;
	}
	if(cnt && (spot = malloc(msgmap, btoq(cnt))) == NULL) {
		if(uap->msgflg & IPC_NOWAIT) {
			u.u_error = EAGAIN;
			return;
		}
		/* mapwant(msgmap)++; */
		msgmap[0].m_addr++;
		sleep(msgmap, PMSG );
		/*if(sleep(msgmap, PMSG | PCATCH)) {
			u.u_error = EINTR;
			return;
		} */
		goto getres;
	}

	/* Everything is available, copy in text and put msg on q. */
	if(cnt) {
		u.u_base = (caddr_t)uap->msgp + sizeof(type);
		u.u_segflg = 0;
		MOVE(msg + msginfo.msgssz * --spot, cnt, B_WRITE);
		if(u.u_error) {
			mfree(msgmap, btoq(cnt), spot + 1);
			return;
		}
	}
	qp->msg_qnum++;
	qp->msg_cbytes += cnt;
	qp->msg_lspid = u.u_procp->p_pid;
	qp->msg_stime = time;
	mp = msgfp;
	msgfp = mp->msg_next;
	mp->msg_next = NULL;
	mp->msg_type = type;
	mp->msg_ts = cnt;
	mp->msg_spot = cnt ? spot : -1;
	if(qp->msg_last == NULL)
		qp->msg_first = qp->msg_last = mp;
	else {
		qp->msg_last->msg_next = mp;
		qp->msg_last = mp;
	}
	if(qp->msg_perm.mode & MSG_RWAIT) {
		qp->msg_perm.mode &= ~MSG_RWAIT;
		curpri = PMSG;
		wakeup(&qp->msg_qnum);
	}
	u.u_rval1 = 0;
}

/*
 * msgsys - System entry point for msgctl, msgget, msgrcv, and msgsnd
 *		system calls.
 */

msgsys()
{
	int		msgctl(),
			msgget(),
			msgrcv(),
			msgsnd();
	static int	(*calls[])() = { msgget, msgctl, msgrcv, msgsnd };
	register struct a {
		unsigned	id;	/* function code id */
		int		*ap;	/* arg pointer for recvmsg */
	}		*uap = (struct a *)u.u_ap;

	if(uap->id > 3) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ap = &u.u_arg[1];
	(*calls[uap->id])();
}
