/*
 * SCCSID: @(#)rl.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * RL01/2 standalone disk driver
 *
 * 7/80 - Created by Armando P. Stettner to support both
 *	RL01 and RL02 drives on the same controller.
 *
 * 5/81 - Modified by Fred Canter to improve drive type
 *	identification.
 *
 * Fred Canter
 */

#include	<sys/param.h>
#include	<sys/inode.h>
#include	"saio.h"

#define	BLKRL1	10240		/* Number of UNIX blocks for an RL01 drive */
#define BLKRL2	20480		/* Number of UNIX blocks for an RL02 drive */
#define RLCYLSZ 10240		/* bytes per cylinder */
#define RLSECSZ 256		/* bytes per sector */

#define RESET 013
#define	RL02TYP	0200	/* drive type bit */
#define STAT 03
#define GETSTAT 04
#define WCOM 012
#define RCOM 014
#define SEEK 06
#define SEEKHI 5
#define SEEKLO 1
#define RDHDR 010
#define IENABLE 0100
#define CRDY 0200
#define OPI 02000
#define CRCERR 04000
#define TIMOUT 010000
#define NXM 020000
#define DE  040000

struct device
{
	int	rlcs;	/* RL control & status register */
	int	rlba;	/* RL bus addess register */
	int	rlda;	/* RL disk address register */
	int	rlmp;	/* RL multipurpose register */
};

#define	NRL	4	/* maximum number of RL drives, DO NOT CHANGE ! */

int	rl_openf;
struct 
{
	int	cn[4];		/* location of heads for each drive */
	int	type[4];	/* RL disk drive type indicator */
				/* -1 = non existent drive */
				/* BLKRL1 = RL01 */
				/* BLKRL2 = RL02 */
	char	openf;		/* RL open flag, 1 = open not completed */
	int	com;		/* read or write command word */
	int	chn;		/* cylinder and head number */
	unsigned int	bleft;	/* bytes left to be transferred */
	unsigned int	bpart;	/* number of bytes transferred */
	int	sn;		/* sector number */
	union {
		int	w[2];
		long	l;
	} addr;			/* address of memory for transfer */

/*
 * Initialize the RL structure as follows:
 *
 * cn[] = all heads, location unknown
 * type[] = all drives, non existent
 * openf = RL open not completed yet
 */

}	rl = {-1,-1,-1,-1, -1,-1,-1,-1, 1} ;

rlstrategy(io, func)
register struct iob *io ;
int func ;
{
	register struct device *rladdr;
	int nblocks ;	/* number of UNIX blocks for the drive in question */
	int drive ;
	int dn;
	int dif ;
	int head ;
	int ctr ;
	int errcnt;

	rladdr = devsw[io->i_ino.i_dev].dv_csr;
	drive = io->i_unit ;	/* get drive number */
	errcnt = 0;
	if(rl_openf == 0){
		for(dn = 0; dn < 4; dn++)
			rl.cn[dn] = -1;
		rl_openf++;
	}
retry:

/*
 * If the RL open has not been completed,
 * then get the status of all possible drives
 * as follows:
 *
 * 1.	Execute a get status with reset up to 8 times
 *	to get a valid status from the drive.
 *
 * 2.	If an OPI error is detected then the drive
 *	is non-existent (NED).
 *
 * 3.	If a vaild status cannot be obtained after 8 attempts,
 *	then print the "can't get status" message and
 *	mark the drive non-existent.
 *
 * 4.	If a valid status is obtained, then use the drive
 *	type to set rl.type to the number of blocks for that
 *	type drive.
 *	10240 for RL01 or 20480 for RL02
 *
 * The above sequence only occurs on the first access
 * of the RL disk driver. The drive status obtained is
 * retained until the system in rebooted.
 * Packs may be mounted and dismounted,
 * HOWEVER the disk unit number select plugs may
 * NOT be changed without rebooting the system.
 *
 ****************************************************************
 *								*
 * For some unknown reason the RL02 does not return a valid	*
 * status the first time a GET STATUS request is issued for	*
 * the drive, in fact it can take up to three or more		*
 * GET STATUS requests to obtain a valid drive status.		*
 * This is why the GET STATUS is repeated eight times		*
 * in step one above.						*
 *								*
 ****************************************************************
 */

	if(rl.openf)
		{
		rl.openf = 0;
		for (dn=0; dn<NRL; dn++)
			{
			for(ctr=0; ctr<8; ctr++)
				{
				rladdr->rlda = RESET;
				rladdr->rlcs = (dn << 8) | GETSTAT;
				while ((rladdr->rlcs & CRDY) == 0) ;
				if(rladdr->rlcs & OPI)
					break;	/* NED */
				if((rladdr->rlmp & 0157400) == 0)
					break;	/* valid status */
				}
			if(rladdr->rlcs & OPI)
				continue;	/* NED */
			if(ctr >= 8)
				{
			printf("\nCan't get status of RL unit %d\n", dn);
				continue;
				}
			if(rladdr->rlmp & RL02TYP)
				rl.type[dn] = BLKRL2;	/* RL02 */
			else
				rl.type[dn] = BLKRL1;	/* RL01 */
			}
		}
/*
 * If rl.cn[drive] = -1, then the position of the
 * heads is not known.
 * Do a read header to find where the heads are.
 */
	if(rl.cn[drive] < 0)
	{
		rladdr->rlcs = (drive << 8) | RDHDR;
 		while ((rladdr->rlcs&CRDY) == 0) ;
		rl.cn[drive] = (rladdr->rlmp >> 6) & 01777;
	}
	nblocks = rl.type[drive] ;	/* how many blocks on this drive */
	if(io->i_bn >= nblocks)
		return(-1);
	rl.chn = io->i_bn/20;
	rl.sn = (io->i_bn%20) << 1;
	rl.bleft = io->i_cc;
	rl.addr.w[0] = segflag & 3;
	rl.addr.w[1] = (int)io->i_ma ;
	rl.com = (drive << 8) ;
	if (func == READ)
		rl.com |= RCOM;
	else
		rl.com |= WCOM;
reading:
	/*
	 * One has to seek an RL head, relativily.
	 */
	dif =(rl.cn[drive] >> 1) - (rl.chn >>1);
	head = (rl.chn & 1) << 4;
	if (dif < 0)
		rladdr->rlda = (-dif <<7) | SEEKHI | head;
	else
		rladdr->rlda = (dif << 7) | SEEKLO | head;
	rladdr->rlcs = (drive << 8) | SEEK;
	rl.cn[drive] = rl.chn; 	/* keep current, our notion of where the heads are */
	if (rl.bleft < (rl.bpart = RLCYLSZ - (rl.sn * RLSECSZ)))
		rl.bpart = rl.bleft;
	while ((rladdr->rlcs&CRDY) == 0) ;
	rladdr->rlda = (rl.chn << 6) | rl.sn;
	rladdr->rlba = rl.addr.w[1];
	rladdr->rlmp = -(rl.bpart >> 1);
	rladdr->rlcs = rl.com | rl.addr.w[0] << 4;
	while ((rladdr->rlcs & CRDY) == 0)	/* wait for completion */
		;
	if (rladdr->rlcs < 0) {	/* check error bit */
		if (rladdr->rlcs & 040000) {	/* Drive error */
			/*
			 * get status from drive
			 */
			rladdr->rlda = STAT;
			rladdr->rlcs = (drive << 8) | GETSTAT;
			while ((rladdr->rlcs & CRDY) == 0)	/* wait for controller */
				;
		}
		if(errcnt == 0) {
			printf("\nRL unit %d disk error: cyl=%d head=%d sect=%d",
				drive, rl.chn>>1, rl.chn&1, rl.sn);
			printf("\nrlcs=%o rlmp=%o", rladdr->rlcs, rladdr->rlmp);
		}
		if(++errcnt >= 10) {
			printf("\n(FATAL ERROR)\n");
			return(-1);
		}
		rladdr->rlda = RESET;
		rladdr->rlcs = (drive << 8) | GETSTAT;
		while((rladdr->rlcs & CRDY) == 0) ;
		goto retry;
	}
	/*
	 * Determine if there is more to read to satisfy this request.
	 * This is to compensate for the lack of spiraling reads.
	 */
	if ((rl.bleft -= rl.bpart) > 0)
		{
		rl.addr.l += rl.bpart ;
		rl.sn = 0 ;
		rl.chn++ ;
		goto reading ;	/* read some more */
		}
	if(errcnt)
		printf("\n(RECOVERED by retry)\n");
	return(io->i_cc);
}
