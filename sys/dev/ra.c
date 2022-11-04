
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)ra.c	3.1	3/26/87
 */
/*
 * ULTRIX-11 UDA50-RA60/RA80/RA81 disk driver
 *	     RQDX -RD31/RD32/RD51/RD52/RD53/RD54/RX50/RX33
 *	     RUX1 -RX50
 *	     KLESI-RC25
 *
 * Fred Canter
 *
 * Supports up to 4 MSCP (UQPORT) controllers,
 * each with up to 4 physical units, 8 logical units.
 * CAVEATS:
 *
 * 1.	Each controller must be of a unique type, i.e.,
 *	can't have two UDA50s or two KLESIs, etc.
 *
 *	FIXED!
 * 2.	The error log "block device activity" report will
 *	only indicate that the MSCP driver was active. It
 *	will not be able to tell how many or which of the
 *	possible 4 controllers were active.
 *
 *	FIXED!
 * 3.	There are sure to be problems assocated with booting
 *	from a disk on other than the first controller, but
 *	I don't know for sure what they are yet.
 *
 *	OPTIONAL FEATURES
 *
 *	DQSORT		Order the I/O request queue for RX50/RD51-54 disks.
 *	RDMIO		Allow multiple winchester I/O requests to be sent to
 *			the RQDX1/2/3 prior to sending a floppy request.
 *
 */

#define	PANIC	1	/* panic on fatal controller error */
#define	NOPANIC	0

/*
 * Queue sorting turned off in V2.0,
 * not worth the extra code! -- Fred
 */
/*
#define	DQSORT	0
*/
#define	RDMIO	1

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/conf.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/seg.h>
#include <sys/errlog.h>
#include <sys/devmaj.h>
#include <sys/ra_info.h>	/* ra_info.h includes mscp.h */

/*
 * Instrumentation (iostat) structures
 */
struct	ios	ra_ios[];
#define	DK_N	8

char	ra_index[];	/* non-semetrical array index (see conf/dds.c) */
int	ra_csr[];	/* (c.c) Hardware register I/O page address */
int	nuda;		/* c.c number of controllers configured */
char	nra[];		/* (c.c) Number of drives configured (per controller) */
int	ra_ivec[];	/* (c.c) MSCP controller interrupt vector addressess */
char	*ra_dct[];	/* MSCP cntlr type, i.e., UDA50, RQDX1, RUX1, KLESI */


struct	ra_drv	ra_drv[];	/* (conf/uda.c) drive types, status, unit size */

char	ra_ctid[];	/* controller type ID + U-code rev */

#ifdef	UDADBUG
int udaerror = 0;	/* set to cause hex dump of error log packets */
#endif	UDADBUG

				/* all the following located in conf/dds.c */
struct uda_softc uda_softc[];	/* software control structure */
struct	uda	uda[];		/* UQSSP communciations area */
char	ra_rs[];		/* rsp/cmd ring size (# of packets) */
char	ra_rsl2[];		/* rsp/cmd ring size (log2) */
struct	mscp	ra_rp[];	/* response ring (actual packets) */
struct	mscp	ra_cp[];	/* command ring (actual packets) */

/*
 * The maintenance area size for each controller is
 * set during controller initialization, as follows:
 *
 * UDA50	1000 blocks
 * RQDX1	32 blocks
 * KLESI	102 blocks
 */
daddr_t	ra_mas[];	/* maint area size UDA50=1000, RQDX1=32, KLESI=102 */

struct	rasize	*ra_sizes[];

/*
 * Sizes tables now in /usr/sys/conf/dksizes.c
 */
extern	struct	rasize	ud_sizes[];
extern	struct	rasize	rc_sizes[];
extern	struct	rasize	rq_sizes[];

struct	mscp *udgetcp();

/*
 * /block I/O device error log buffer.
 */
struct	ra_ebuf	ra_ebuf[];
int	ra_elref[];	/* used with command reference number to */
			/* associate datagrams with end messages */

struct	buf ratab[];	/* UDA controller queue */
struct	buf rawtab[];	/* I/O wait queue */
struct	buf rautab[];	/* Drive queue, one per unit */
struct	buf rrabuf[];	/* RAW I/O buffer header, one per drive */

#ifdef	DQSORT
#define	b_cylin	b_resid
#define	RXNBPC	10
#define	RDNBPC	72
int	rd_dqs = 1;	/* Allows dynamic control of DQSORT */
#endif	DQSORT

#ifdef	RDMIO
int	rd_mio = 16;	/* Allows dynamic control of RDMIO */
int	rd_iocnt;	/* # of consecutive RD51-54 I/O requests processed */
/*int	rd_mioc = 16;	/* MAX # of consecutive RD51-54 I/O requests */
#endif	RDMIO

/*
 * Open a UDA.  Initialize the device and
 * set the unit online.
 */
raopen(dev, flag)
	dev_t dev;
	int flag;	/* -1 = called from main(), don't set u.u_error */
{
	register struct uda_softc *sc;
	register struct mscp *mp;
	register struct ra_regs *raaddr;
	int ctrl, unit, s, i, ind;

/*
 * Set up pointers to cmd/rsp ring descriptors
 * and ring packets, on first open.
 */
	ctrl = ractrl(dev);
	if(flag == -1) {
	    for(s=0, i=0; i<ctrl; i++)
		s += ra_rs[i];
	    uda[ctrl].uda_rsp = &ra_rp[s];
	    uda[ctrl].uda_cmd = &ra_cp[s];
	}
	unit = raunit(dev);
	ind = ra_index[ctrl] + unit;
	s = spl5();
	if(unit >= nra[ctrl])
		goto bad;
	sc = &uda_softc[ctrl];
	if (sc->sc_state != S_RUN) {
		if (sc->sc_state == S_IDLE) {
			i = 0;	/* try to start cntlr init 3 times */
			while(udinit(ctrl)) {
				if(++i >= 3)
					goto bad;	/* fatal cntlr error */
			}
		}
		/* wait for initialization to complete */
		sleep((caddr_t)&uda_softc[ctrl], PSWP+1);
		if (sc->sc_state != S_RUN)
			goto bad;
	}
/*
 * Get the status of the unit and save it,
 * by attempting to force unit online.
 */
	if(ra_drv[ind].ra_online==0) {
		while((mp = udgetcp(ctrl)) == NULL)
			sleep((caddr_t)&uda[ctrl].uda_ca.ca_cmdint, PSWP+1);
		raaddr = ra_csr[ctrl];
		sc->sc_credits--;
		if(flag < 0)
			mp->m_opcode = M_O_GTUNT;
		else
			mp->m_opcode = M_O_ONLIN;
		mp->m_unit = unit;
		mp->m_cmdref = &ra_drv[ind].ra_dt;
		mp->m_elref = 0;	/* MBZ, see uderror() */
		*((int *)mp->m_dscptr + 1) |= UDA_OWN|UDA_INT;
		i = raaddr->raaip;
		sleep((caddr_t)mp->m_cmdref, PSWP+1);
	}
	/* Status saved from response packet by interrupt routine udrsp() */
	if(ra_drv[ind].ra_online == 0) {	/* NED or off-line */
	bad:
		if(flag >= 0)
			u.u_error = ENXIO;
	}
	splx(s);
	/*
	 * Initialize disk I/O instrumentation structures
	 */
	i = DK_N + ctrl;
	if(dk_iop[i] == 0) {
		s = spl6();
		dk_iop[i] = &ra_ios[ra_index[ctrl]];
		dk_nd[i] = nra[ctrl];
		splx(s);
	}
}

/*
 * Initialize a UDA,
 * initialize data structures, and start hardware
 * initialization sequence.
 */
udinit(ctrl)
{
	register struct uda_softc *sc;
	register struct ra_regs *raaddr;
	unsigned cnt;	/* don't change to register */
	int rtn;

	sc = &uda_softc[ctrl];
	raaddr = ra_csr[ctrl];
	/*
	 * Start the hardware initialization sequence.
	 *	If cntlr error bit set, report it and continue.
	 *	Wait a minimum of 100 micro-seconds after the IP
	 *	write before checking the S1 bit in the SA register.
	 *	(this prevents seeing left over S1 bits)
	 *	If init fails, report error and return failure.
	 */
	rtn = 0;
	if(raaddr->raasa & UDA_ERR) {	/* fatal cntlr error */
	bad:
		udfatal(NOPANIC, ctrl, sc->sc_state, raaddr->raasa);
		if(rtn)
			return(1);
	}
	rtn = 1;
	raaddr->raaip = 0;		/* start initialization */
	for(cnt=0; cnt<1000; cnt++) ;	/* wait guaranteed min 100 micro-sec */
	cnt = 0177777;			/* cntlr should enter S1 by 100us */
	while ((raaddr->raasa & UD_STEP1) == 0) {
		if(--cnt == 0)
			goto bad;
	}
	raaddr->raasa =
	 UDA_ERR|(ra_rsl2[ctrl]<<11)|(ra_rsl2[ctrl]<<8)|UDA_IE|(ra_ivec[ctrl]/4);
	/*
	 * Initialization continues in interrupt routine.
	 */
	sc->sc_state = S_STEP1;
	sc->sc_credits = 0;
	return(0);
}

rastrategy(bp)
	register struct buf *bp;
{
	register struct buf *dp;
	register struct rasize *rasizes;
	int	ctrl, unit, ind;
	daddr_t sz, maxsz;
	int s, fs;
#ifdef	DQSORT
	int	nbpc;
#endif	DQSORT

	mapalloc(bp);
	ctrl = ractrl(bp->b_dev);
	rasizes = ra_sizes[ctrl];
	unit = raunit(bp->b_dev);
	ind = ra_index[ctrl] + unit;
	fs = bp->b_dev & 7;
	if((unit >= nra[ctrl]) ||
	   (ra_drv[ind].ra_dt == 0) ||
	   (ra_drv[ind].d_un.ra_dsize == 0))
		goto bad;
	sz = (bp->b_bcount+511) >> 9;
	if ((maxsz = rasizes[fs].nblocks) < 0) {
		maxsz = ra_drv[ind].d_un.ra_dsize - rasizes[fs].blkoff;
		if(rasizes[fs].nblocks == -2)
			maxsz -= ra_mas[ctrl];
	}
	if (bp->b_blkno < 0 || bp->b_blkno+sz > maxsz ||
	    rasizes[fs].blkoff >= ra_drv[ind].d_un.ra_dsize) {
	bad:
		bp->b_error = ENXIO;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return;
	}
	s = spl5();
	/*
	 * Link the buffer onto the drive queue.
	 * If the drive is an RX50 or RD51-54, and queue sorting
	 * is enabled, then order the queue.
	 */
	dp = &rautab[ind];
#ifdef	DQSORT
	if(rd_dqs == 0)
		goto ra_enter;	/* Queue sorting disabled */
	if((ra_drv[ind].ra_dt == RX33) || (ra_drv[ind].ra_dt == RX50))
		nbpc = RXNBPC;
	else if(ra_drv[ind].ra_dt == RD51)
		nbpc = RDNBPC;
	else if((ra_drv[ind].ra_dt == RD31) ||
		(ra_drv[ind].ra_dt == RD32) ||
		(ra_drv[ind].ra_dt == RD52) ||
		(ra_drv[ind].ra_dt == RD53) ||
		(ra_drv[ind].ra_dt == RD54))
			nbpc = RDNBPC * 2;
	else
		nbpc = 0;
	if(nbpc) {
		bp->b_cylin = bp->b_blkno/nbpc;
		disksort(dp, bp);
	} else {
ra_enter:
#endif	DQSORT
		if(dp->b_actf == NULL) {
		/*
		 * Insure that the links for the unit table
		 * are initialized, very bad things happen
		 * if they are not initialized !
		 */
			dp->b_actf = bp;
			dp->b_actl = bp;
			bp->av_forw = NULL;
		} else
			dp->b_actl->av_forw = bp;
		dp->b_actl = bp;
		bp->av_forw = NULL;
#ifdef	DQSORT
	}
#endif	DQSORT
	/*
	 * Link the drive onto the controller queue,
	 * if not active, i.e., not already on queue.
	 * The ratab links are initialized by binit().
	 */
	if(dp->b_active == NULL) {
		dp->b_forw = NULL;
		if(ratab[ctrl].b_actf == NULL)
			ratab[ctrl].b_actf = dp;
		else
			ratab[ctrl].b_actl->b_forw = dp;
		ratab[ctrl].b_actl = dp;
		dp->b_active = 1;
	}
	if(ratab[ctrl].b_active == NULL)
		rastart(ctrl);
	splx(s);
}

rastart(ctrl)
{
	register struct buf *bp, *dp;
	register struct mscp *mp;
	struct uda_softc *sc;
	struct ra_regs *raaddr;
	struct rasize *rasizes;
	int i;
	int unit, ind;
	union {
		long	longw;
		struct {
			int	lo;
			int	hi;
		} t_str;
	} t_un;

	sc = &uda_softc[ctrl];
loop:
	if((dp = ratab[ctrl].b_actf) == NULL) {
		ratab[ctrl].b_active = NULL;
		return;
	}
	if((bp = dp->b_actf) == NULL) {
		/*
		 * No more requests for this drive, remove it
		 * from the controller queue and look at the next drive.
		 * We know we're at the head of the controller queue.
		 */
		dp->b_active = NULL;
		ratab[ctrl].b_actf = dp->b_forw;
		goto loop;
	}
	ratab[ctrl].b_active++;
	ra_mcact |= (1 << ctrl);	/* tell error log cntlr is active */
	raaddr = ra_csr[ctrl];
	if ((raaddr->raasa&UDA_ERR) || sc->sc_state != S_RUN) {
/*		udinit(ctrl);	*/
		/* SHOULD REQUEUE OUTSTANDING REQUESTS, LIKE UDRESET */
/*		return;	*/
		udfatal(PANIC, ctrl, sc->sc_state, raaddr->raasa);
	}
	/*
	 * If no credits, can't issue any commands
	 * until some outstanding commands complete.
	 */
	if (sc->sc_credits < 2)
		return (0);
	if ((mp = udgetcp(ctrl)) == NULL)
		return (0);
 	sc->sc_credits--;	/* committed to issuing a command */
	unit = raunit(bp->b_dev);
	ind = ra_index[ctrl] + unit;
	if(ra_drv[ind].ra_online == 0) {
		mp->m_opcode = M_O_ONLIN;
		mp->m_unit = unit;
		mp->m_cmdref = 0;	/* no sleep */
		mp->m_elref = 0;	/* MBZ, see uderror() */
		dp->b_active = 2;
		ratab[ctrl].b_actf = dp->b_forw;	/* remove from controller queue */
		*((int *)mp->m_dscptr + 1) |= UDA_OWN|UDA_INT;
		i = raaddr->raaip;
		goto loop;
	}
	mp->m_cmdref = bp;	/* pointer to get back */
	mp->m_elref = ra_elref[ctrl]++;	/* error log ref # */
	mp->m_opcode = bp->b_flags&B_READ ? M_O_READ : M_O_WRITE;
	mp->m_unit = unit;
	rasizes = ra_sizes[ctrl];
	t_un.longw = bp->b_blkno + rasizes[bp->b_dev&7].blkoff;
	mp->m_lbn_l = t_un.t_str.hi;
	mp->m_lbn_h = t_un.t_str.lo;
	mp->m_bytecnt = bp->b_bcount;
	mp->m_zzz2 = 0;
	mp->m_buf_l = bp->b_un.b_addr;
	mp->m_buf_h = bp->b_xmem;
	*((int *)mp->m_dscptr + 1) |= UDA_OWN|UDA_INT;
	i = raaddr->raaip;		/* initiate polling */
	/* iostat stuff */
	ra_ios[ind].dk_busy++;		/* drive active */
	ra_ios[ind].dk_numb++;		/* count # of xfer's */
	ra_ios[ind].dk_wds += (bp->b_bcount >> 6);	/* count words xfer'd */
	/*
	 * Move drive to the end of the controller queue.
	 * If RDMIO is defined, allow up to rd_mio RD I/O requests before
	 * moving the RD drive to the end of the controller queue.
	 * Cut down on RX interference with RD I/O throughput.
	 */
	if(dp->b_forw != NULL) {
#ifdef	RDMIO
		if(rd_mio &&
		    ((ra_drv[ind].ra_dt == RD31) ||
		     (ra_drv[ind].ra_dt == RD32) ||
		    ((ra_drv[ind].ra_dt>=RD51) && (ra_drv[ind].ra_dt<=RD54))) &&
		    (++rd_iocnt <= rd_mio))
			goto ra_iowq;
		rd_iocnt = 0;
#endif	RDMIO
		ratab[ctrl].b_actf = dp->b_forw;
		ratab[ctrl].b_actl->b_forw = dp;
		ratab[ctrl].b_actl = dp;
		dp->b_forw = NULL;
	}
#ifdef	RDMIO
	else
		rd_iocnt = 0;
#endif	RDMIO
ra_iowq:
	/*
	 * Move the buffer to the I/O wait queue.
	 */
	dp->b_actf = bp->av_forw;
	dp = &rawtab;
	if(dp->av_forw == 0) {
	/*
	 * Initialize the I/O wait queue links,
	 * very bad things happen if not !
	 */
		dp->av_forw = dp;
		dp->av_back = dp;
	}
	bp->av_forw = dp;
	bp->av_back = dp->av_back;
	dp->av_back->av_forw = bp;
	dp->av_back = bp;
	goto loop;
}

/*
 * UDA interrupt routine.
 */
raintr(dev)
{
	register struct ra_regs *raaddr;
	register struct uda_softc *sc;
	register struct uda *ud;
	struct mscp *mp;
	int i, j, ctrl;
	char	*ubm_off;	/* address offset (if UB map used) */

	if(ubmaps)
		ubm_off = (char *)&cfree;/* cfree = UB virtual addr 0 */
	else
		ubm_off = 0;
	ctrl = dev & 3;
	ud = &uda[ctrl];
	raaddr = ra_csr[ctrl];
	sc = &uda_softc[ctrl];
	switch (sc->sc_state) {
	case S_IDLE:
		logsi(raaddr);
		return;

	case S_STEP1:
#define	STEP1MASK	0174377
		i = UD_STEP2 | UDA_IE | (ra_rsl2[ctrl]<<3) | ra_rsl2[ctrl];
		if ((raaddr->raasa&STEP1MASK) != i) {
			sc->sc_state = S_IDLE;
			wakeup((caddr_t)&uda_softc[ctrl]);
			return;
		}
		raaddr->raasa = ((char *)&uda[ctrl].uda_ca.ca_ringbase-ubm_off);
		sc->sc_state = S_STEP2;
		return;

	case S_STEP2:
#define	STEP2MASK	0174377
#define	STEP2GOOD	(UD_STEP3|UDA_IE|(ra_ivec[ctrl]/4))
		if ((raaddr->raasa&STEP2MASK) != STEP2GOOD) {
			sc->sc_state = S_IDLE;
			wakeup((caddr_t)&uda_softc[ctrl]);
			return;
		}
		raaddr->raasa = 0;    /* ringbase will always be in low 64kb */
		sc->sc_state = S_STEP3;
		return;

	case S_STEP3:
#define	STEP3MASK	0174000
#define	STEP3GOOD	UD_STEP4
		if ((raaddr->raasa&STEP3MASK) != STEP3GOOD) {
			sc->sc_state = S_IDLE;
			wakeup((caddr_t)&uda_softc[ctrl]);
			return;
		}
		i = raaddr->raasa & 03777;	/* UQSSP ID + micro-code rev */
		if(((i >> 4) & 0177) == R_RQDX3)
			ra_ctid[ctrl] = (char)((RQDX3 << 4) | (i & 017));
		else
			ra_ctid[ctrl] = (char)(i & 0377);
		i = (ra_ctid[ctrl] >> 4) & 017;
		ra_sizes[ctrl] = &ud_sizes;
		ra_mas[ctrl] = (long)RA_MAS;
		if(i == RQDX1) {	/* set cont. specific info */
			ra_dct[ctrl] = "RQDX1";	/* cont. type for error msg's */
			ra_mas[ctrl] = (long)RD_MAS;	/* maint. area size */
			ra_sizes[ctrl] = &rq_sizes;
		} else if(i == RQDX3) {
			ra_dct[ctrl] = "RQDX3";
			ra_mas[ctrl] = (long)RD_MAS;
			ra_sizes[ctrl] = &rq_sizes;
		} else if(i == RUX1) {
			ra_dct[ctrl] = "RUX1";
			ra_mas[ctrl] = (long)RD_MAS;
			ra_sizes[ctrl] = &rq_sizes;
		} else if(i == KLESI) {
			ra_dct[ctrl] = "KLESI";
			ra_mas[ctrl] = (long)RC_MAS;
			ra_sizes[ctrl] = &rc_sizes;
		} else if(i == KDA50) {
			ra_dct[ctrl] = "KDA50";
/* NO KDA25 support -- Fred	*/
/*		} else if(i == R_KDA25) {	/* Use fake ID of 4 */
/*			ra_dct[ctrl] = "KDA25";		*/
/*			i = ra_ctid[ctrl] & 017;	*/
/*			i |= (KDA25 << 4);		*/
/*			ra_ctid[ctrl] = (char )i;	*/
		} else {	/* UDA50 is default */
			ra_dct[ctrl] = "UDA50";
		}
		/*
		 * Tell the controller to start normal operations.
		 * Also set the NPR burst size to the default,
		 * which is controller dependent.
		 */
		raaddr->raasa = UDA_GO;
		sc->sc_state = S_SCHAR;

		/*
		 * Initialize the data structures.
		 */
		for (i = 0; i < ra_rs[ctrl]; i++) {
			ud->uda_ca.ca_dscptr[i].rh = UDA_OWN|UDA_INT;
			mp = ud->uda_rsp;
			mp += i;
			ud->uda_ca.ca_dscptr[i].rl = &mp->m_cmdref;
			(char *)ud->uda_ca.ca_dscptr[i].rl -= ubm_off;
			ud->uda_rsp[i].m_dscptr = &ud->uda_ca.ca_dscptr[i].rl;
			ud->uda_rsp[i].m_header.uda_msglen = 
			    sizeof(struct mscp) - sizeof(struct mscp_header);
		}
		/* cmd dscptrs start where ever rsp dscptrs end */
		for(j=0; j<ra_rs[ctrl]; j++, i++) {
			ud->uda_ca.ca_dscptr[i].rh = UDA_INT;
			mp = ud->uda_cmd;
			mp += j;
			ud->uda_ca.ca_dscptr[i].rl = &mp->m_cmdref;
			(char *)ud->uda_ca.ca_dscptr[i].rl -= ubm_off;
			ud->uda_cmd[j].m_dscptr = &ud->uda_ca.ca_dscptr[i].rl;
			ud->uda_cmd[j].m_header.uda_msglen = 
			    sizeof(struct mscp) - sizeof(struct mscp_header);
			/* FYI - disk virtual circuit ID is 0 */
		}
		sc->sc_lastcmd = 0;
		sc->sc_lastrsp = 0;
		if ((mp = udgetcp(ctrl)) == NULL) {
			sc->sc_state = S_IDLE;
			wakeup((caddr_t)&uda_softc[ctrl]);
			return;
		}
		mp->m_opcode = M_O_STCON;
		mp->m_cntflgs = M_C_ATTN|M_C_MISC|M_C_THIS;
		*((int *)mp->m_dscptr + 1) |= UDA_OWN|UDA_INT;
		i = raaddr->raaip;	/* initiate polling */
		return;

	case S_SCHAR:
	case S_RUN:
		break;

	default:
		logsi(raaddr);	/* will print SI followed by address */
		printf("\n%s state=%d\n", ra_dct[ctrl], sc->sc_state);
		return;
	}

	if (raaddr->raasa&UDA_ERR) {
/*
		printf("%s: fatal error (%o)\n", ra_dct[ctrl], raaddr->raasa);
		raaddr->raaip = 0;
		wakeup((caddr_t)&uda_softc[ctrl]);
 */
		udfatal(PANIC, ctrl, sc->sc_state, raaddr->raasa);
	}
	/*
	 * Check for response ring transition.
	 */
	if (ud->uda_ca.ca_rspint) {
		ud->uda_ca.ca_rspint = 0;
		for (i = sc->sc_lastrsp;; i++) {
			i %= ra_rs[ctrl];
			if (ud->uda_ca.ca_dscptr[i].rh&UDA_OWN)
				break;
			udrsp(ctrl, ud, sc, i);
			ud->uda_ca.ca_dscptr[i].rh |= UDA_OWN;
		}
		sc->sc_lastrsp = i;
	}
	/*
	 * Check for command ring transition.
	 */
	if (ud->uda_ca.ca_cmdint) {
		ud->uda_ca.ca_cmdint = 0;
		wakeup((caddr_t)&uda[ctrl].uda_ca.ca_cmdint);
	}
	rastart(ctrl);
}

/*
 * Process a response packet
 */
udrsp(ctrl, ud, sc, i)
	register struct uda *ud;
	register struct uda_softc *sc;
	int i;
{
	register struct mscp *mp;
	struct buf *dp, *bp;
	int st;
	int unit, opcode, ind;
	int msglen;

	mp = (struct mscp *)ud->uda_rsp + i;
	msglen = mp->m_header.uda_msglen;
	mp->m_header.uda_msglen = sizeof(struct mscp)-sizeof(struct mscp_header);
	sc->sc_credits += mp->m_header.uda_credits & 017;
	if(sc->sc_tcmax < sc->sc_credits)	/* save xfer credit limit */
		sc->sc_tcmax = sc->sc_credits;
	if ((mp->m_header.uda_credits & 0360) > 020)
		return;
	/*
	 * If it's an error log message (datagram),
	 * pass it on for more extensive processing.
	 */
	if ((mp->m_header.uda_credits & 0360) == 020) {
		uderror(ctrl, (struct mslg *)mp, msglen);
		return;
	}
	if ((unit = mp->m_unit) >= nra[ctrl])
		return;
	ind = ra_index[ctrl] + unit;
	st = mp->m_status&M_S_MASK;
	opcode = mp->m_opcode&0377;
	switch (opcode) {
	case M_O_STCON|M_O_END:
		if (st == M_S_SUCC)
			sc->sc_state = S_RUN;
		else
			sc->sc_state = S_IDLE;
		ra_mcact &= ~(1 << ctrl);
		ratab[ctrl].b_active = 0;
		wakeup((caddr_t)&uda_softc[ctrl]);
		break;

	case M_O_ONLIN|M_O_END:
/*
 * If the command reference mumber (mp->m_cmdref) is zero,
 * the online command came from rastart().
 * If it is non zero, the online command came form raopen().
 */
		/*
		 * Link the drive onto the controller queue.
		 * ONLY if ONLINE command came from rastart().
		 * Confuses the queue, don't know exactly why !
		 */
		dp = &rautab[ind];
		if(mp->m_cmdref == 0) {
			dp->b_forw = NULL;
			if(ratab[ctrl].b_actf == NULL)
				ratab[ctrl].b_actf = dp;
			else
				ratab[ctrl].b_actl->b_forw = dp;
			ratab[ctrl].b_actl = dp;
		}
		if ((st == M_S_SUCC) || (st == M_S_OFFLN)) {
			ra_drv[ind].ra_dt = *((int *)&mp->m_mediaid)&0177;
			if(st == M_S_SUCC) {
				ra_drv[ind].ra_online = 1;
				/*
				 * Set xfer rate for iostat
				 * pure guesswork !!!!
				 */
				if((ra_drv[ind].ra_dt >= RD51) &&
				   (ra_drv[ind].ra_dt <= RD54))
					ra_ios[ind].dk_tr = 18;
				else if(ra_drv[ind].ra_dt == RD31)
					ra_ios[ind].dk_tr = 18;/* XXX */
				else if(ra_drv[ind].ra_dt == RD32)
					ra_ios[ind].dk_tr = 18;/* XXX */
				else if(ra_drv[ind].ra_dt == RX50)
					ra_ios[ind].dk_tr = 17;
				else if(ra_drv[ind].ra_dt == RX33)
					ra_ios[ind].dk_tr = 17;/* XXX */
				else if(ra_drv[ind].ra_dt == RC25)
					ra_ios[ind].dk_tr = 19;
				else	/* DEFAULT: ra60, ra80, ra81 */
					ra_ios[ind].dk_tr = 16;
			} else
				ra_drv[ind].ra_online = 0;
			if(mp->m_unitid != 0) {
			    ra_drv[ind].d_un.d_str.ra_dslo = mp->m_ushigh; /* get size */
			    ra_drv[ind].d_un.d_str.ra_dshi = mp->m_uslow;
			}
		} else {
			ra_drv[ind].ra_dt = 0;		/* mark drive NED */
			ra_drv[ind].ra_online = 0;	/* mark drive offline */
		}
		if(ra_drv[ind].ra_online == 0) {
			printf("\n%s unit %d OFFLINE: status=%o\n",
				ra_dct[ctrl], unit, mp->m_status);
			while(bp = dp->b_actf) {
				dp->b_actf = bp->av_forw;
				bp->b_error = ENXIO;
				bp->b_flags |= B_ERROR;
				iodone(bp);
			}
		}
		if(mp->m_cmdref)
			wakeup((caddr_t)mp->m_cmdref);
		else
			dp->b_active = 1;
		break;
/*
 * Duplicate unit number attn message.
 */
	case M_O_DUPUN:
		printf("\n%s: duplicate unit # = %d!\n", ra_dct[ctrl], unit);
		break;

/*
 * The AVAILABLE ATTENTION messages occurs when the
 * unit becomes available after spinup,
 * marking the unit offline will force an online command
 * prior to using the unit.
 */
	case M_O_AVATN:
		ra_drv[ind].ra_online = 0;	/* mark unit offline */
		break;

	case M_O_END:
/*
 * An endcode without an opcode (0200) is an invalid command.
 * The mscp specification states that this would be a protocol
 * type error, such as illegal opcodes. The mscp spec. also
 * states that parameter error type of invalid commands should
 * return the normal end message for the command. This does not appear
 * to be the case. An invalid logical block number returned an endcode
 * of 0200 instead of the 0241 (read) that was expected.
 * This may be due to using the old UDA50 modules instead of the
 * new (enhanced) UDA50.
 * WRONG !, the new UDA50 modules have the same problem.
 *
 * As per a phone conversation with BG of CX, the mscp spec is incorrect
 * and will be changed. BG and I agree that the way the mscp spec states
 * that invalid commands are reported would be the correct way to do it,
 * BUT the powers that be say other wise.
 *
 * For this reason, invalid command errors will
 * be handled as follows:
 *
 * 1.	If the endcode is for a read or write command,
 *	the command will complete normally but the invalid
 *	command code in the status will cause an error to be logged
 *	and the buffer to be flaged with an error.
 *
 * 2.	If the endcode is 0200 and the command reference number does
 *	not match the buffer pointer in ratab, an error message will
 *	be printed, this may be folowed by a system hang !!
 *	I JUST DON'T KNOW !!!!!!
 *
 * 3.	If the endcode is 0200 and the command reference number
 *	matches the buffer pointer in ratab, it will be assumed
 * 	that the invalid command is the current read/write command.
 *	Action taken will be the same as in case one above.
 */
	if((struct buf *)mp->m_cmdref != ratab[ctrl].b_actf) {
		printf("\n%s: inv cmd err, ", ra_dct[ctrl]);
		printf("endcd=%o, stat=%o\n", opcode, mp->m_status);
		break;
	}
	case M_O_READ|M_O_END:
	case M_O_WRITE|M_O_END:
		bp = (struct buf *)mp->m_cmdref;
		/*
		 * Unlink the buffer form the I/O wait queue.
		 */
		bp->av_back->av_forw = bp->av_forw;
		bp->av_forw->av_back = bp->av_back;
		dp = &rautab[ind];
		/* iostat stuff */
		ra_ios[ind].dk_busy = 0;
		if (st == M_S_OFFLN || st == M_S_AVLBL) {
			ra_drv[ind].ra_online = 0;	/* mark unit offline */
			/*
			 * Link the buffer onto the front of the drive queue.
			 */
			if((bp->av_forw = dp->b_actf) == NULL)
				dp->b_actl = bp;
			dp->b_actf = bp;
			/*
			 * Link the drive onto the controller queue.
			 */
			if(dp->b_active == NULL) {
				dp->b_forw = NULL;
				if(ratab[ctrl].b_actf == NULL)
					ratab[ctrl].b_actf = dp;
				else
					ratab[ctrl].b_actl->b_forw = dp;
				ratab[ctrl].b_actl = dp;
				dp->b_active = 1;
			}
			return;
		}
/*
 * Fatal I/O error,
 * format a block device error log record and
 * log it if possible, print error message if not.
 */
		if (st != M_S_SUCC)
			bp->b_flags |= B_ERROR;
		if((bp->b_flags&B_ERROR) || (mp->m_flags&M_E_ERLOG)) {
			fmtbde(bp, &ra_ebuf[ctrl], (int *)mp,
				((sizeof(struct mscp_header)/2)+(msglen/2)), 0);
			ra_ebuf[ctrl].ra_bdh.bd_csr = ra_csr[ctrl];
			/* hi byte of XMEM is drive type for ELP ! */
			ra_ebuf[ctrl].ra_bdh.bd_xmem |= (ra_drv[ind].ra_dt << 8);
			ra_ebuf[ctrl].ra_reg[0] = msglen;    /* real message length */
			if(st == M_S_SUCC)
				ra_ebuf[ctrl].ra_bdh.bd_errcnt = 2;	/* soft error */
			else
				ra_ebuf[ctrl].ra_bdh.bd_errcnt = 0;	/* hard error */
			if((mp->m_status & M_S_MASK) == M_S_WRTPR)
			  printf("%s unit %d Write Locked\n", ra_dct[ctrl], unit);
			else {
			  if(!logerr(E_BD, &ra_ebuf[ctrl],
				(sizeof(struct elrhdr) + sizeof(struct el_bdh) +
				msglen + sizeof(struct mscp_header))))
	deverror(bp, ((mp->m_opcode&0377) | (mp->m_flags<<8)), mp->m_status);
			}
		}
		bp->b_resid = bp->b_bcount - mp->m_bytecnt;
		iodone(bp);
		break;

/*
 * The driver assues that any get unit status command
 * came from raopen() and just saves the drive type.
 * The `unitid' being non-zero says media type ID valid.
 */
	case M_O_GTUNT|M_O_END:
		if(mp->m_unitid != 0)
			ra_drv[ind].ra_dt = *((int *)&mp->m_mediaid)&0177;
		if(mp->m_cmdref)
			wakeup((caddr_t)mp->m_cmdref);
		break;

	default:
		printf("\n%s: bad/unexp packet, len=%d opcode=%o\n",
			ra_dct[ctrl], msglen, opcode);
	}
	/*
	 * If no commands outstanding, say controller
	 * is no longer active.
	 */
	if(sc->sc_credits == sc->sc_tcmax)
		ra_mcact &= ~(1 << ctrl);
}


/*
 * Process an error log message
 *
 * Format an error log record of the type, uda datagram,
 * and log it if possible.
 * Also print a cryptic error message, this may change later !
 */
uderror(ctrl, mp, msglen)
	register struct mslg *mp;
{
	register int i;
	register int *p;
	struct buf *bp;
	int	unit;

/*
 * SOFTWARE WORK AROUND FOR RQDX1 MICROCODE BUG
 *
 * If an ECC error occurs during the RX50 on-line command,
 * the RQDX1 microcode creams the cmdref # and unit # of the
 * error log packet. This causes a memory management violation 
 * due to the bad unit number.
 *
 * magic numbers	cmdref = 137, zzz1 = 14116, unit = 775
 *
 * Fred Canter 7/23/83
 *
 *	FIXED IN V8.0 OF RQDX1 MICRO-CODE 10/22/83
 */
	unit = mp->me_unit;
	if((unit < 0) ||
	    (unit >= nra[ctrl]) ||	/* THIS CHECK MUST STAY */
	    (mp->me_cmdref & 1)) {
/*
 * save some space, remove entirely when
 * sure no more V7 micro-code in the field.
 * Fred Canter 6/13/84
		printf("\n%s: bad error log packet IGNORED %o %o %o\n",
		    ra_dct[ctrl], mp->me_cmdref, mp->me_elref, unit);
 */
		return;
	}
	ra_ebuf[ctrl].ra_hdr.e_time = time;
/*
 * The following test will not work if error log
 * reference numbers are added to non data transfer commands.
 */
	bp = (struct buf *)mp->me_cmdref;
	if(bp && mp->me_elref) {	/* only for xfer commands */
		ra_ebuf[ctrl].ra_bdh.bd_dev = bp->b_dev;
	} else
		ra_ebuf[ctrl].ra_bdh.bd_dev = ((RA_BMAJ<<8)|(ctrl<<6)|(unit<<3));
	ra_ebuf[ctrl].ra_bdh.bd_flags = 0100000;	/* say RA datagram */
	/* hi byte of XMEM is drive type for ELP ! */
	ra_ebuf[ctrl].ra_bdh.bd_xmem = (ra_drv[ra_index[ctrl]+unit].ra_dt << 8);
	ra_ebuf[ctrl].ra_bdh.bd_act = el_bdact;
	if((mp->me_flags & (M_LF_SUCC | M_LF_CONT)) == 0)
		ra_ebuf[ctrl].ra_bdh.bd_errcnt = 0;	/* hard error */
	else
		ra_ebuf[ctrl].ra_bdh.bd_errcnt = 2;	/* soft error */
	ra_ebuf[ctrl].ra_bdh.bd_nreg = ((msglen/2) + (sizeof(struct mscp_header)/2));
	p = (int *)mp;
	for(i=0; i<((msglen/2)+(sizeof(struct mscp_header)/2)); i++)
		ra_ebuf[ctrl].ra_reg[i] = *p++;
	ra_ebuf[ctrl].ra_reg[0] = msglen;	/* real message length */
	if(logerr(E_BD, &ra_ebuf[ctrl], (sizeof(struct elrhdr) +
		sizeof(struct el_bdh) + msglen + sizeof(struct mscp_header))))
			return;	/* no msg printed if error logged */
	printf("\n%s: %s err, ", ra_dct[ctrl],
		mp->me_flags&(M_LF_SUCC | M_LF_CONT) ? "soft" : "hard");
	switch (mp->me_format&0377) {
	case M_F_CNTERR:
		printf("cont err, event=0%o\n", mp->me_event);
		break;

	case M_F_BUSADDR:
		printf("host mem access err, event=0%o, addr=%o 0%o\n",
			mp->me_event, mp->me_busaddr[1], mp->me_busaddr[0]);
		break;

	case M_F_DISKTRN:
		printf("xfer err, unit=%d, event=0%o\n",
			unit, mp->me_event);
		break;

	case M_F_SDI:
		printf("SDI err, unit=%d, event=0%o\n", unit,
			mp->me_event);
		break;

	case M_F_SMLDSK:
		printf("small disk err, unit=%d, event=0%o, cyl=%d\n",
			unit, mp->me_event, mp->me_sdecyl);
		break;

	case M_F_REPLACE:
		printf("BBR attempt, unit=%d, event=0%o\n",
			unit, mp->me_event);
		break;

	default:
		printf("unknown err, unit=%d, format=0%o, event=0%o\n",
			unit, mp->me_format&0377, mp->me_event);
	}

/*
 * The udaerror flag is normally set to zero,
 * setting it to one with ADB will cause the entire
 * error log packet to be printed as octal words,
 * for some poor sole to decode.
 */
#ifdef	UDADBUG
	if (udaerror) {

		p = (int *)mp;
		for(i=0; i<((msglen + sizeof(struct mscp_header))/2); i++) {
			printf("%o ", *p++);
			if((i&07) == 07)
				printf("\n");
		}
		printf("\n");
	}
#endif	UDADBUG
}


/*
 * Find an unused command packet
 * Only an immediate command can take the
 * last command descriptor.
 */
struct mscp *
udgetcp(ctrl)
{
	register struct mscp *mp;
	register struct udaca *cp;
	register struct uda_softc *sc;
	int i, j;

	cp = &uda[ctrl].uda_ca;
	sc = &uda_softc[ctrl];
	/* cmd dscptrs start where ever rsp dscptrs end */
	i = sc->sc_lastcmd;
	j = i + ra_rs[ctrl];	/* offset into cmd descriptors */
	if ((cp->ca_dscptr[j].rh & (UDA_OWN|UDA_INT)) == UDA_INT) {
		cp->ca_dscptr[j].rh &= ~UDA_INT;
		mp = (struct mscp *)uda[ctrl].uda_cmd + i;
		mp->m_unit = mp->m_modifier = 0;
		mp->m_opcode = mp->m_flags = 0;
		mp->m_bytecnt = mp->m_zzz2 = 0;
		mp->m_buf_l = mp->m_buf_h = 0;
		mp->m_elgfll = mp->m_elgflh = 0;
		mp->m_copyspd = 0;
		sc->sc_lastcmd = (i + 1) % ra_rs[ctrl];
		return(mp);
	}
	return(NULL);
}

raread(dev)
	dev_t dev;
{
	physio(rastrategy, &rrabuf[ra_index[ractrl(dev)]+raunit(dev)], dev, B_READ);
}

rawrite(dev)
	dev_t dev;
{
	physio(rastrategy, &rrabuf[ra_index[ractrl(dev)]+raunit(dev)], dev, B_WRITE);
}

/*
 * So much for what we should do, here is what we will do !
 * PANIC, because there is no room for the code that would
 * be required to clean up the mess that could be created
 * by this type of error due to the large number of requests
 * that could be queued in the UDA50. Although this is painfull,
 * it less so than if we attempted to continue !
 */

udfatal(rap, ctrl, st, sa)
{
	printf("\nMSCP cntlr %d error: ", ctrl);
	printf("CSR=%o SA=%o state=%d\n", ra_csr[ctrl], sa, st);
	if(rap)
		panic("MSCP fatal");
}

raclose(dev,flag)
dev_t dev;
{

	bflclose(dev);
}
