/*
 * SCCSID: @(#)ra.c	3.4	7/11/87
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 Stand-alone driver for MSCP disks
 *
 *	UDA50 - RA60/RA80/RA81
 *	KDA50 - RA60/RA80/RA81
 *	KLESI - RC25
 *	RQDX1 - RD31/RD32/RX33/RX50/RD51/RD52/RD53/RD54
 *	RQDX2 - "
 *	RQDX3 - "
 *	RUX1  - RX50
 *
 * Supports three MSCP controllers, but only one of each type.
 *
 * Fred Canter
 *
 * WISHLIST:
 *
 * 1.	Add error checking and messages for init seq.
 * 2.	Improve error messsages in I/O section.
 */

#include <sys/param.h>
#include <sys/inode.h>
#include <sys/mscp.h>
#include "saio.h"
#include "ra_saio.h"

/*
 * UQPORT registers and structures
 */

struct device {
	int	raaip;		/* initialization and polling */
	int	raasa;		/* status and address */
};

struct	device	*ra_csr[3];

#define	UDA_ERR		0100000	/* error bit */
#define	UD_STEP4	0040000	/* step 4 has started */
#define	UD_STEP3	0020000	/* step 3 has started */
#define	UD_STEP2	0010000	/* step 2 has started */
#define	UD_STEP1	0004000	/* step 1 has started */
#define	UD_SMASK	0074000	/* mask for checking multiple step bits set */
#define	UDA_NV		0002000	/* no host settable interrupt vector */
#define	UDA_QB		0001000	/* controller supports Q22 bus */
#define	UDA_DI		0000400	/* controller implements diagnostics */
#define	UDA_IE		0000200	/* interrupt enable */
#define	UDA_PI		0000001	/* host requests adapter purge interrupts */
#define	UDA_GO		0000001	/* start operation, after init */
#define	UD_DEL1		200	/* SA reg checking loop count (see uqpbits) */

/*
 * Parameters for the communications area
 */

#define	NRSPL2	0		/* log2 number of response packets */
#define	NCMDL2	0		/* log2 number of command packets */
#define	NRSP	(1<<NRSPL2)
#define	NCMD	(1<<NCMDL2)

/*
 * UQPORT Communications Area
 */

struct udaca {
	short	ca_xxx1;	/* unused */
	char	ca_xxx2;	/* unused */
	char	ca_bdp;		/* BDP to purge */
	short	ca_cmdint;	/* command queue transition interrupt flag */
	short	ca_rspint;	/* response queue transition interrupt flag */
	struct {
		unsigned int rl;
		unsigned int rh;
	} ca_rspdsc[NRSP];	/* response descriptors */
	struct {
		unsigned int cl;
		unsigned int ch;
	} ca_cmddsc[NCMD];	/* command descriptors */
};

#define	ca_ringbase	ca_rspdsc[0].rl

#define	UDA_OWN	0100000	/* UDA owns this descriptor */
#define	UDA_INT	0040000	/* allow interrupt on ring transition */

/*
 * Controller states
 */
#define	S_IDLE	0		/* hasn't been initialized */
#define	S_STEP1	1		/* doing step 1 init */
#define	S_STEP2	2		/* doing step 2 init */
#define	S_STEP3	3		/* doing step 3 init */
#define	S_SCHAR	4		/* doing "set controller characteristics" */
#define	S_RUN	5		/* running */

struct uda {
	struct udaca	uda_ca;		/* communications area */
	struct mscp	uda_rsp[NRSP];	/* response packets */
	struct mscp	uda_cmd[NCMD];	/* command packets */
} uda[3];

struct mscp *udcmd();

/*
 * Most of the following defines and variables are
 * shared with the RABADS program.
 */

#define	RP_WRT	1	/* RABADS command is WRITE */
#define	RP_RD	2	/* RABADS command is READ */
#define	RP_REP	3	/* RABADS command is REPLACE */
#define RP_AC	4	/* RABADS command is ACCESS */

int	ra_badc;		/* RABADS command flag */
int	ra_badm;		/* RABADS command modifier */
union {
	daddr_t	ra_rbnl;	/* RABADS RBN for REPLACE command */
	short	ra_rbnw[2];
} ra_rbn;
int	ra_openf[3];		/* Controller has been initialized flag */
int	ra_stat[3];		/* Info saved for error messages */
int	ra_ecode[3];
int	ra_eflags[3];
int	ra_ctid[3];		/* controller type ID + micro-code rev level */
char	*ra_dct[3];		/* controller type for error messages */

/*
 * Drive info obtained from on-line and get
 * unit status commands issued to all possible
 * drives when the controller is initialized.
 */
struct	ra_drv	ra_drv[3][4];	/* see ra_saio.h, size must be 3 * 4 */

/*
 * Open a UDA.  Initialize the device and
 * set the unit online.
 */

raopen(io)
register struct iob *io;
{
	register struct device *raaddr;
	register int ctrl;
	int i;
	char *p;

	ctrl = devsw[io->i_ino.i_dev].dv_cn;
	raaddr = (struct device *)devsw[io->i_ino.i_dev].dv_csr;
	ra_csr[ctrl] = raaddr;	/* save CSR for udcmd() */
	while(1) {
	    if(ra_openf[ctrl] && ((raaddr->raasa&UDA_ERR) == 0))
		break;	/* cntlr already inited & no cntlr error in SA reg */
	    p = (caddr_t)&uda[ctrl];	/* zero UQSSP communications area */
	    for(i=0; i<sizeof(struct uda); i++)
		*p++ = 0;
	    if(raaddr->raasa&UDA_ERR)
		uqperror("OPEN ERROR", raaddr);	/* print cntlr error */
	    if(uqpinit(ctrl, raaddr) == 0) {
		ra_openf[ctrl] = 1;		/* cntlr init succeeded */
		break;
	    } else
		return(-1);			/* cntrl init failed */
	}
	if(rainit(ctrl, io->i_unit&7) == 0) {
		printf("\n%s unit %d OFFLINE: status=%o", ra_dct[ctrl],
			io->i_unit&7, ra_stat[ctrl]);
		return(-1);
	}
	return(0);
}

/*
 * WARNING, CAUTION, NOTE, etc.....
 *	The code in this routine which attempts to prevent
 *	the RQDX3 false entry into step 1 (gate array bug)
 *	from hanging the installation could cause an endless
 *	loop of its own. If the SA register shows an error or
 *	any of the step bits set, we try to re-init the cntlr.
 *	Of course, the controller init routine calls udcmd,
 *	and so on.....
 *	This would only happen on nested errors, which, hopefully
 *	are unlikely events.
 *	This is gross, but better than always hanging!
 * Fred Canter -- 4/4/87
 * Fred Canter -- 7/11/87
 *	The above is bogus! Once the RQDX3 hangs, there is no
 *	way in the world for the software to unjam it! SO,
 *	we just feed the RQDX3 a vector and that makes it happy.
 */

struct mscp *
udcmd(ctrl, op)
register int ctrl;
int op;
{
	struct mscp *mp;
	int i, j;
	register struct device *raaddr;

	for(j=0; j<3; j++) {		/* try command 3 times max */
	    raaddr = ra_csr[ctrl];
	    uda[ctrl].uda_cmd[0].m_opcode = op;
	    uda[ctrl].uda_rsp[0].m_header.uda_msglen = 
	        sizeof(struct mscp) - sizeof(struct mscp_header);
	    uda[ctrl].uda_cmd[0].m_header.uda_msglen = 
	        sizeof(struct mscp) - sizeof(struct mscp_header);
	    uda[ctrl].uda_ca.ca_rspdsc[0].rh &= ~UDA_INT;
	    uda[ctrl].uda_ca.ca_cmddsc[0].ch &= ~UDA_INT;
	    uda[ctrl].uda_ca.ca_rspdsc[0].rh |= UDA_OWN;
	    uda[ctrl].uda_ca.ca_cmddsc[0].ch |= UDA_OWN;
	    i = raaddr->raaip;
	    while(uda[ctrl].uda_ca.ca_cmddsc[0].ch & UDA_OWN) {
		for(i=0; i<UD_DEL1; i++) ;	/* SA access delay */
		if(raaddr->raasa & (UDA_ERR | UD_SMASK))
		    goto udc_fail;
	    }
	    while(uda[ctrl].uda_ca.ca_rspdsc[0].rh & UDA_OWN) {
		for(i=0; i<UD_DEL1; i++) ;	/* SA access delay */
		if(raaddr->raasa & (UDA_ERR | UD_SMASK))
		    goto udc_fail;
	    }
	    mp = &uda[ctrl].uda_rsp[0];
	    ra_stat[ctrl] = mp->m_status;
	    ra_ecode[ctrl] = mp->m_opcode&0377;
	    ra_eflags[ctrl] = mp->m_flags&0377;
	    uda[ctrl].uda_ca.ca_cmdint = 0;
	    uda[ctrl].uda_ca.ca_rspint = 0;
	    if ((mp->m_opcode & 0377) != (op|M_O_END) ||
	        (mp->m_status&M_S_MASK) != M_S_SUCC)
	    	return(0);
	    return(mp);
udc_fail:					/* cntlr error or false init */

	    uda[ctrl].uda_ca.ca_rspdsc[0].rh &= ~UDA_OWN;
	    uda[ctrl].uda_ca.ca_cmddsc[0].ch &= ~UDA_OWN;
	    if(uqpinit(ctrl, raaddr) == UDA_ERR)	/* re-init cntlr */
		return(0);
	    else
	        continue;				/* retry command */
	}
}

rastrategy(io, func)
	register struct iob *io;
{
	register struct mscp *mp;
	register int ctrl;
	int unit, op;
	char *p;
	union {
		long	longw;
		struct {
			int	lo;
			int	hi;
		} t_str;
	} t_un;

	ctrl = devsw[io->i_ino.i_dev].dv_cn;
	unit = io->i_unit & 7;
	if(unit >= 4)
		return(-1);
	p = 0;
	if((devsw[io->i_ino.i_dev].dv_flags&DV_RX) &&
	   (ra_drv[ctrl][unit].ra_dt != RX33) &&
	   (ra_drv[ctrl][unit].ra_dt != RX50))
		p = "RX";
	else if((devsw[io->i_ino.i_dev].dv_flags&DV_RC) &&
		(ra_drv[ctrl][unit].ra_dt != RC25))
			p = "RC";
	else if((devsw[io->i_ino.i_dev].dv_flags&DV_RD) &&
		(ra_drv[ctrl][unit].ra_dt != RD31) &&
		(ra_drv[ctrl][unit].ra_dt != RD32) &&
		(ra_drv[ctrl][unit].ra_dt != RD51) &&
		(ra_drv[ctrl][unit].ra_dt != RD52) &&
		(ra_drv[ctrl][unit].ra_dt != RD53) &&
		(ra_drv[ctrl][unit].ra_dt != RD54))
			p = "RD";
	else if((devsw[io->i_ino.i_dev].dv_flags&DV_RA) &&
		(ra_drv[ctrl][unit].ra_dt != RA60) &&
		(ra_drv[ctrl][unit].ra_dt != RA80) &&
		(ra_drv[ctrl][unit].ra_dt != RA81))
			p = "RA";
	if(p) {
		printf("\n%s unit %d: not %s disk!\n",
			ra_dct[ctrl], unit, p);
		ra_openf[ctrl] = 0;
		return(-1);
	}
	/*
	 * Make sure the controller is alive and well
	 * before initiating the I/O request.
	 * Try to re-init cntlr if UDA_ERR set in SA register.
	 */
	if((int)((struct device *)ra_csr[ctrl]->raasa) & UDA_ERR) {
		uqperror("RUN ERROR", ra_csr[ctrl]);
		if(uqpinit(ctrl, ra_csr[ctrl]) == UDA_ERR)
			return(-1);
	}
	zcmdpkt(ctrl);
	mp = &uda[ctrl].uda_cmd[0];
	t_un.longw = io->i_bn;
	mp->m_lbn_l = t_un.t_str.hi;
	mp->m_lbn_h = t_un.t_str.lo;
	mp->m_unit = io->i_unit&7;
	if(ra_badc) {	/* allows compare and force error modifiers */
		mp->m_modifier = ra_badm;
		ra_badm = 0;
	}
	if(ra_badc == RP_REP) {		/* REPLACE command */
		mp->m_bytecnt = ra_rbn.ra_rbnw[1]; 	/* RBN lo word */
		mp->m_zzz2 = ra_rbn.ra_rbnw[0]; 	/* RBN hi word */
		mp->m_buf_l = 0;
		mp->m_buf_h = 0;
	} else {
		mp->m_bytecnt = io->i_cc;
		mp->m_zzz2 = 0;
		mp->m_buf_l = io->i_ma;
		mp->m_buf_h = segflag;
	}
	if(ra_badc == RP_REP)
		op = M_O_REPLC;
	else if(ra_badc == RP_AC)
		op = M_O_ACCES;
	else if(func == READ)
		op = M_O_READ;
	else
		op = M_O_WRITE;
	if((mp = udcmd(ctrl, op)) == 0) {
	    if(ra_badc == 0) {
		printf("\n%s unit %d disk error: ", ra_dct[ctrl], io->i_unit&7);
		printf("endcode=%o flags=%o status=%o\n",
			ra_ecode[ctrl], ra_eflags[ctrl], ra_stat[ctrl]);
		printf("(FATAL ERROR)\n");
	    } else
		ra_badc = 0;
	return(-1);
	}
	ra_badc = 0;
	return(io->i_cc);
}

/*
 * Initialize a drive,
 * do GET UNIT STATUS and ONLINE commands
 * and save the results.
 */
rainit(ctrl, unit)
register int ctrl, unit;
{
	register struct mscp *mp;

	if(unit >= 4) {
		ra_stat[ctrl] = 03;	/* fake unit unknown status code */
		return(0);
	}
	zcmdpkt(ctrl);
	uda[ctrl].uda_cmd[0].m_unit = unit;
	mp = &uda[ctrl].uda_rsp[0];
	ra_drv[ctrl][unit].ra_online = 0; 	/* mark unit off-line */
	ra_drv[ctrl][unit].ra_dt = 0;		/* mark unit non-existent */
	udcmd(ctrl, M_O_GTUNT);
	if(mp->m_unitid[3] != 0) {	/* unit exists */
		ra_drv[ctrl][unit].ra_dt = *((int *)&mp->m_mediaid) & 0177;
		ra_drv[ctrl][unit].ra_rbns = mp->m_rbns;
		ra_drv[ctrl][unit].ra_ncopy = mp->m_rctcpys;
		ra_drv[ctrl][unit].ra_rctsz = mp->m_rctsize;
		ra_drv[ctrl][unit].ra_trksz = mp->m_track;
	} else
		return(0);		/* non-existent unit */
	/* zcmdpkt() not needed here, if added must reload unit number field */
	if(udcmd(ctrl, M_O_ONLIN) != 0)		/* ON-LINE command */
		ra_drv[ctrl][unit].ra_online = 1;	/* unit is on-line */
/*
 * Save unit size, even though
 * in may not be valid.
 */
	ra_drv[ctrl][unit].d_un.d_str.ra_dslo = mp->m_ushigh;
	ra_drv[ctrl][unit].d_un.d_str.ra_dshi = mp->m_uslow;
	return(ra_drv[ctrl][unit].ra_online);
}

/*
 * Zero the entire command packet before loading
 * the command. This makes sure all MBZ and reserved
 * fields are zero. The new "architecture consistent"
 * MSCP controller micro-code rejects commands if these
 * fields are not zero.
 */

zcmdpkt(ctrl)
{
	register int * p;
	register int i;

	p = &uda[ctrl].uda_cmd[0];
	for(i=0; i<sizeof(struct mscp)/2; i++)
		*p++ = 0;
}

/*
 * UQSSP controller initialization routine.
 *
 * INIT the port and return zero if the init succeeds
 * or UDA_ERR if the init fails.
 * Try the init three times, then punt!
 *
 * ctrl	- MSCP cntrl number
 * addr - address of IP register
 */

uqpinit(ctrl, addr)
register struct device *addr;
int	ctrl;
{
	register int i, j;

	for(j=0; j<3; j++) {	/* Give init 3 chances to succeed */
	    addr->raaip = 0;	/* start initialization */
	    /*
	     * Wait for zero in the SA register or until
	     * a loop count of 1000 expires.
	     * Prevents seeing left over bits caused by
	     * async update of SA register by the port.
	     */
	    i = 0;
	    while(addr->raasa != 0) {
		if(++i > 1000)
		    break;
	    }
	    if(uqpbits(addr, UD_DEL1, UD_STEP1))	/* wait for step 1 */
		continue;				/* error of some sort */
	    /*
	     * We give the cntlr a vector, but don't
	     * allow interrupts. This prevents the RQDX3
	     * false entry into init step one.
	     */
	    addr->raasa = (UDA_ERR | (0154/4));
	    if(uqpbits(addr, UD_DEL1, UD_STEP2))	/* wait for step 2 */
		continue;				/* blew it again! */
	    addr->raasa = (short)&uda[ctrl].uda_ca.ca_ringbase;
	    if(uqpbits(addr, UD_DEL1, UD_STEP3))	/* wait for step 3 */
		continue;				/* SHAZBOT, wrong again */
	    addr->raasa = segflag;
	    if(uqpbits(addr, UD_DEL1, UD_STEP4))	/* wait for step 4 */
		continue;				/* OH HECK DARN, bad news */
	    ra_ctid[ctrl] = addr->raasa & 03777;	/* save controller ID */
	    switch((ra_ctid[ctrl]>>4) & 0177) {
	    case UDA50:
		ra_dct[ctrl] = "UDA50";
		break;
	    case KLESI:
		ra_dct[ctrl] = "KLESI";
		break;
	    case RUX1:
		ra_dct[ctrl] = "RUX1";
		break;
	    case UDA50A:
		ra_dct[ctrl] = "UDA50A";
		break;
	    case RQDX1:
		ra_dct[ctrl] = "RQDX1";
		/*
		 * Save RQDX type ID for mkfs.
		 */
		if(segflag == 1)
			RQ_CTID->r[0] = RQDX1;
		break;
	    case KDA50:
		ra_dct[ctrl] = "KDA50";
		break;
	    case R_RQDX3:
		ra_dct[ctrl] = "RQDX3";
		i = ra_ctid[ctrl] & 017;
		i |= (RQDX3 << 4);	/* fake RQDX3 ID of 14 */
		ra_ctid[ctrl] = i;
		/*
		 * Save RQDX type ID for mkfs.
		 */
		if(segflag == 1)
			RQ_CTID->r[0] = RQDX3;
		break;
	    default:
		ra_dct[ctrl] = "MSCP";
		break;
	    }
	    addr->raasa = UDA_GO;
	    uda[ctrl].uda_ca.ca_rspdsc[0].rl = &uda[ctrl].uda_rsp[0].m_cmdref;
	    uda[ctrl].uda_ca.ca_rspdsc[0].rh = segflag;
	    uda[ctrl].uda_ca.ca_cmddsc[0].cl = &uda[ctrl].uda_cmd[0].m_cmdref;
	    uda[ctrl].uda_ca.ca_cmddsc[0].ch = segflag;
	    zcmdpkt(ctrl);
	    uda[ctrl].uda_cmd[0].m_cntflgs = 0;
	    if(udcmd(ctrl, M_O_STCON) == 0) {
		uqperror("STCON FAILED", addr);
		continue;	/* try init again */
	    }
	    /*
	     * Init all possible drives.
	     */
	    for(i=0; i<4; i++)
		rainit(ctrl, i);
	    return(0);
	}
	uqperror("INIT FAILED", addr);
	return(UDA_ERR);
}

/*
 * Routine to spin on UQSSP SA register STEP bits.
 *
 * RETURN when/if desired STEP bit sets.
 *
 * RETURN error if:
 *	STEP1 not set after waiting much longer than 100 micro-seconds.
 *	Multiple STEP bits detected.
 *	UDA_ERR bit sets.
 *
 * addr  - address of IP register.
 * delay - loop count (only look at SA every "delay" counts),
 *	   give the cntlr micro-code a little rest between RA reads.
 * step  - UD_STEP# bit to wait for setting.
 */

uqpbits(addr, delay, step)
register struct device *addr;
int	delay;
int	step;
{
	register int i;

	if(step == UD_STEP1) {
	    for(i=0; i<32767; i++) ;
	    if((addr->raasa&UD_SMASK) != UD_STEP1)
		return(UDA_ERR);
	    else
		return(0);
	} else {
	    while(1) {
		for(i=0; i<delay; i++) ;
		if((addr->raasa & step) == 0)
		    continue;
		if((addr->raasa & UD_SMASK) != step)
		    return(UDA_ERR);
		else
		    return(0);
	    }
	}
}

/*
 * MSCP controller error print routine.
 */

uqperror(str, addr)
char	*str;
register struct device *addr;
{
	printf("\nMSCP cntlr at %o: %s (SA=%o)\n", addr, str, addr->raasa);
}
