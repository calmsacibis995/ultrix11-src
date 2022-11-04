
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)cda_q.c	3.0	4/21/86";
/*
 * "q" option for cda
 *
 * prints the block I/O buffer header queues
 *
 * Bill Burns 1984
 *
 *	This code is organized as follows:
 *
 *	1. qcmd() - is the main routine.
 *
 *	2. bioinfo() - obtains block device information from
 *		namelist and corefile
 *
 *	3. prfree() - prints the free list queue
 *
 *	4. prdev() - prints a device controller queue (xxtab)
 *
 *	5. prudev() - prints a device drive queue (xxutab)
 *
 *	6. prraw() - prints the rawtab queue
 *
 *	7. 3 thru 6 above use the following subroutines:
 *
 *		bufnum() - which returns an indicator as to where
 *			a buffer pointer points
 *		cchk() - which checks a buffer queue for consistency
 *		prbuf() - which formats the buffer header printout
 *
 */
#include <sys/param.h>	/* Don't matter which one */
#include <sys/errlog.h>
#include <stdio.h>
#include <a.out.h>

#include <sys/buf.h>
#include <sys/devmaj.h>
#include <sys/conf.h>
#include <sys/mount.h>
#include <sys/hp_info.h>
#include <sys/ra_info.h>
#include "cda.h"


extern int mem;
extern int flags;
extern int nlfil;
extern int nuda;
extern struct nlist nl[];
extern int nra;
extern int ntk;
extern char xnra[];

int	nblkdev;	/* # of block dev entries in bdevsw[] (from kernel) */
struct buf frbuf;	/* freelist buffer header */
struct buf devbuf[MBLKDEV]; 	/* buffer headers for block devices */
struct bdevsw bsw[MBLKDEV]; 	/* space for bdevsw table */
struct buf swbuf1;	/* swap buffer header 1 */
struct buf swbuf2;	/* swap buffer header 2 */
			/* device tables for multi-cntlr drivers (4 max) */
struct buf ra_devt[4];
struct buf tk_devt[4];
struct buf ts_devt[4];
struct buf rawtab;	/* space for rawtab */
struct buf tkwtab;	/* space for tkwtab */

char left[7] [40];		/* temp storage area for printout routines */
char right[7] [40];

struct nlist bqnl[] = {		/* namelist for -q option */
	{ "_nbuf" },		/*  0 */
	{ "_bfreeli" },		/*  1 */
	{ "_buf" },		/*  2 */
	{ "_bdevsw" },		/*  3 */
	{ "_nmount" },		/*  4 */
	{ "_mount" },		/*  5 */
	{ "_nblkdev" },		/*  6 */
	{ "" },
};

struct nlist misc[] = {		/* misc buffer headers */
	{ "_swbuf1" },		/*  0 */
	{ "_swbuf2" },		/*  1 */
	{ "_rawtab" },		/*  2 */
	{ "_tkwtab" },		/*  3 */
};

#define	NUTAB	6
struct nlist dutab[] = {		/* drive (utab) queues */
	{ "_rautab" },		/*  0 */
	{ "_rlutab" },		/*  1 */
	{ "_hputab" },		/*  2 */
	{ "_hmutab" },		/*  3 */
	{ "_hjutab" },		/*  4 */
	{ "_hkutab" },		/*  5 */
};
int nbh[NUTAB];		/* number of buffer headers for individual */
			/* drive queues (utab): loaded by bioinfo() */

#define	NRAWBUF	13
struct nlist rbuf[] = {			/* raw buffer headers */
	{ "_rrkbuf" },		/*  0 */
	{ "_rrpbuf" },		/*  1 */
	{ "_rrabuf" },		/*  2 */
	{ "_rrlbuf" },		/*  3 */
	{ "_rhxbuf" },		/*  4 */
	{ "_rtmbuf" },		/*  5 */
	{ "_rtkbuf" },		/*  6 */
	{ "_rtsbuf" },		/*  7 */
	{ "_rhtbuf" },		/*  8 */
	{ "_rhpbuf" },		/*  9 */
	{ "_rhmbuf" },		/* 10 */
	{ "_rhjbuf" },		/* 11 */
	{ "_rhkbuf" },		/* 12 */
	{ "" },
};


int bfoff[MBLKDEV];	/* offset into utab array, corresponding to */
			/* major device number */
int rboff[MBLKDEV];	/* offset into raw buf header array */

/*
 * number of devices for all blk mode maj device numbers
 * set up by bioinfo()
 *	
 */
#define	NNDEV	13
struct nlist ndev[] = {			/* raw buffer headers */
	{ "_nrk" },		/*  0 */
	{ "_nrp" },		/*  1 */
	{ "_nra" },		/*  2 */
	{ "_nrl" },		/*  3 */
	{ "_nhx" },		/*  4 */
	{ "_ntm" },		/*  5 */
	{ "_ntk" },		/*  6 */
	{ "_nts" },		/*  7 */
	{ "_nht" },		/*  8 */
	{ "_nhp" },		/*  9 */
	{ "_nhm" },		/* 10 */
	{ "_nhj" },		/* 11 */
	{ "_nhk" },		/* 12 */
	{ "" },
};
int nbkd[MBLKDEV];	/* array for number of raw buffer headers */
int	tbd;		/* total number of block device units */

int nts;
int nrh;
char nhpa[MAXRH];		/* array of number of drives on rh */
				/* controllers hp,hm,hj */

int nbuf;		/* number of buffers */
int nmount;		/* number of possable super block buffers */
struct buf *bufx;	/* pointer to buffer headers */
struct buf *bufbas;	/* pointer to buffer headers */
struct mount *mntp;	/* pointer to mount table */
struct mount *mntbas;	/* pointer to mount table */
struct buf *utab, *utabp; /* pointer to area calloc'ed for device utabs */
struct buf *rawbuf, *rbufp; /* pointer to area calloc'ed for */
			/* raw device headers */
int nutab;		/* number of utabs */

/* array of queue names and type of queue indicator */
struct bufq {
	char	*name;
	int	flag;	/* flag == 01 for devices with multiple queues */
			/* flag == 02 for multi-controller under same major # */
			/* flag == 04 for controller wait queue under same major # */
} dev[] = {
		{ "rk",0 },
		{ "rp",0 },
		{ "ra",7 },
		{ "rl",1 },
		{ "hx",0 },
		{ "tm",0 },
		{ "tk",6 },
		{ "ts",2 },
		{ "ht",0 },
		{ "hp",1 },
		{ "hm",1 },
		{ "hj",1 },
		{ "hk",1 },
		{ "u1",0 },
		{ "u2",0 },
		{ "u3",0 },
		{ "u4",0 },
		{ "",0 },
};

char	*nlre = "namelist read error\n";

qcmd()
{
	register struct buf *bp;
	int i, j, k, l, m, x;

	if(!(flags & BIOINFO))
		bioinfo();

	printf("\n********************* Buffer Headers ***********************\n\n");
	printf("%d buffers in system\n\n",nbuf);
	mntp = mntbas;

	for(i = 0; i < nmount; i++) {
		if(mntp[i].m_bufp) {
		    printf("Buffer #%3d:", bufnum(mntp[i].m_bufp)); 
		    printf(" Super Block for ");
		    printf("%s", dev[((mntp[i].m_dev) >> 8)].name);
		    if((dev[((mntp[i].m_dev) >> 8)].name) == "rk")
		 	printf(" unit %d\n", (mntp[i].m_dev & 07));
		    else {
			printf(" unit %d", ((mntp[i].m_dev >> 3) & 07));
		    	printf(" partition %d\n",(mntp[i].m_dev & 07));
		    }
		}
	}
	printf("\n********************* Free List Queue **********************\n\n");
	prfree();	 		/* print the freelist */
	printf("\n****************** End of Free List Queue ******************\n\n");
	printf("\n********************** Device Queues ***********************\n\n");
	for(i = 0; i < nblkdev; i++) {
		if ((bp = bsw[i].d_tab) == 0)
			continue;
		if(dev[i].flag & 04)	/* rawtab/tkwtab */
			prraw(i);
		if(dev[i].flag & 02) {	/* mscp/tk/ts/ - multiple controller */
			if (i == RA_BMAJ)
				m = nuda;
			else if (i == TK_BMAJ)
				m = ntk;
			else
				m = nts;
			for(j = 0; j < m; j++) {
				k = 0;
				prdev(i,(j+1));
				if (i == RA_BMAJ) {
					if(j > 0) {
						k += xnra[0];
						if(j > 1)
							k += xnra[1];
						if(j > 2)
							k += xnra[2];
					}
					for(l = 0; l < xnra[j]; l++,k++)
						prudev(i,0,(j<<8 | k));
				}
			}
		} else {
			switch(i) {	
			case HP_BMAJ:		/* hputab (hp) */
			case HM_BMAJ:		/* hputab (hm) */
			case HJ_BMAJ:		/* hputab (hj) */
				j = 2; 
				break;
			case RL_BMAJ:		/* rlutab */
				j = 1; 
				break;
			case HK_BMAJ:		/* hkutab */
				j = 5; 
				break;
			default:
				j = 0;
				break;
			}
			prdev(i,0);	
			if(j == 2) {	/* multi rh code */
				x = k = 0;
				if(i > HP_BMAJ) {
					x++;
					k += nhpa[0];
				}
				if(i > HM_BMAJ) {
					x++;
					k += nhpa[1];
				}
				for(l = 0; l < nhpa[x]; l++,k++)
					prudev(i,j,k);
			} else if(j) {	/* all non ra & rh with utab */
				for(k = 0; k < nbh[j]; k++)
					prudev(i,j,k);
			}
		}
	}
	close(mem);
}

bufnum(bp)
struct buf *bp;
{
	int i, j;
	struct buf *tbp;

	if(((struct buf *)bp >= (struct buf *)bqnl[2].n_value) 
	    && ((struct buf *)bp <= ((struct buf *)bqnl[2].n_value + nbuf)))
		return((struct buf *)bp - (struct buf *)bqnl[2].n_value);
	if((struct buf *)bp == 0)
		return(01000);				/* NULL pointer */
	if((struct buf *)bp == (struct buf *)bqnl[1].n_value)
		return(01001);				/* bfreelist */
	if((struct buf *)bp == (struct buf *)misc[0].n_value)
		return(05000);				/* swbuf1 */
	if((struct buf *)bp == (struct buf *)misc[1].n_value)
		return(05001);				/* swbuf2 */
	if((struct buf *)bp == (struct buf *)misc[2].n_value)
		return(05002);				/* rawtab */
	if((struct buf *)bp == (struct buf *)misc[3].n_value)
		return(05003);				/* tkwtab */
	for(i = 0; i < nblkdev; i++) {
		if(bsw[i].d_tab == bp)
			return(02000 + i);
	}
	for(i = 0; i < NUTAB; i++) { 		/* utab */
		tbp = dutab[i].n_value;
		if(tbp == NULL)
			continue;
		for(j = 0; j < nbh[i]; j++) {
			if((tbp + j) == bp)
				return(03000 | (((bfoff[i] + j) << 3) | i));
		}
	}
	for(i = 0; i < NRAWBUF; i++) { 		/* raw headers */
		tbp = rbuf[i].n_value;
		if(tbp == NULL)
			continue;
		for(j = 0; j < nbkd[i]; j++) {
			if((tbp + j) == bp) {
				return(04000 | (((rboff[i] + j) << 4) | i));
			}
		}
	}
	return(-1);				/* illegal */
}

/*
 * Print out the free list
 * Headed by _bfreelist
 */
prfree()
{
	struct buf *bp;
	int diff, numb, x;
	char name[40];

	bp = &frbuf; 
	cchk(bp,bqnl[1].n_value,1,"Freelist");
	sprintf(name,"bfreeli: address = %-8o", bqnl[1].n_value);
	prbuf(bp,1,name,left);
	x = 1;
	while(bp->av_forw != bqnl[1].n_value) {
		if((diff = bufnum(bp->av_forw)) != -1) {
			bp = &bufx[diff];
			sprintf(name,"buf #%3d: address = %-8o", diff, ((struct buf*)bqnl[2].n_value + diff));
		} else {
			printf("fatal error\n");
			exit(1);
		}
		if(x == 1) {
			x++;
			prbuf(bp,1,name,right);
			output(x);
			x = 0;
		} else {
			prbuf(bp,1,name,left);
			x++;
		}
		bp = &bufx[diff];
	}
	if(x)
		output(x);
}

/*
 * Print out a buffer
 */
prbuf(bp,type,name,out)
struct buf *bp;
int type;		/* 1 = follow av_forw; 2 = follow b_forw */
char *name;
char *out;
{
	char local[40];
	int i, j, diff;
	int bufadd;

	sprintf(out,"%-30s ",name);
	sprintf(out+40,"%20s","=========================");

	for(bufadd = 80; bufadd < 240; bufadd += 40) {
		*(out+bufadd) = '\0';
		switch(bufadd) {
		case 80:
			sprintf(out+bufadd,"%-7s ","b_forw");
			sprintf(local,"%08o ", bp->b_forw);
			diff = bufnum(bp->b_forw);
			break;
		case 120:
			sprintf(out+bufadd,"%-7s ","b_back");
			sprintf(local,"%08o ", bp->b_back);
			diff = bufnum(bp->b_back);
			break;
		case 160:
			sprintf(out+bufadd,"%-7s ","av_forw");
			sprintf(local,"%08o ", bp->av_forw);
			diff = bufnum(bp->av_forw);
			break;
		case 200:
			sprintf(out+bufadd,"%-7s ","av_back");
			sprintf(local,"%08o ", bp->av_back);
			diff = bufnum(bp->av_back);
			break;
		}
		strcat(out+bufadd, local);
		if(diff == -1) {
			sprintf(local,"       ");
		} else if(diff < 01000) {
			sprintf(local,"buf #%3d ",diff);
		} else if(diff == 01000) {
			sprintf(local,"null    ");
		} else if(diff == 01001) {
			sprintf(local,"bfreeli ");
		} else if(diff == 05000) {
			sprintf(local,"swbuf1 ");
		} else if(diff == 05001) {
			sprintf(local,"swbuf2 ");
		} else if(diff == 05002) {
			sprintf(local,"rawtab ");
		} else if(diff == 05003) {
			sprintf(local,"tkwtab ");
		} else if(diff >= 04000) {
			for(i = 0; i < 6; i++)
				local[i] = rbuf[(diff & 017)].n_name[i+1];
			j = (((diff - 04000) >> 4) - rboff[diff&017]);
			if(j <= 7) {
				local[i++] = ' ';
				local[i++] = (char)('0' + j);
			}
			local[i] = '\0';
		} else if(diff >= 03000) {
			for(i = 0; i < 6; i++)
				local[i] = dutab[(diff & 7)].n_name[i+1];
			j = (((diff - 03000) >> 3) - bfoff[diff&07]);
			if(j <= 7) {
				local[i++] = ' ';
				local[i++] = (char)('0' + j);
			}
			local[i] = '\0';
		} else {
			sprintf(local, "%s", dev[diff-02000].name);
			strcat(local, "tab  ");
		}
		strcat(out+bufadd, local);
		if((type == 2) && (bufadd == 80)) {
			sprintf(local," >");
			strcat(out+bufadd, local);
		}
		if((type == 1) && (bufadd == 160)) {
			sprintf(local," >");
			strcat(out+bufadd, local);
		}
	}
	sprintf(out+240,"%20s","=========================");
}

/*
 * Print out device buffer queues
 */
prdev(maj,flag)
int maj;	/* block major device number */
int flag;	/* flag is 1 for mscp devices (multiple controllers on same
		   major device number */
{
	struct buf *bp;
	char temp[40];
	int tmp, diff, x;
	int i, j;
	char name[20];
	char local[40];

	sprintf(name,"%stab\0",dev[maj].name);
	i = 0;
	temp[0] = '\0';
	if(flag) { 	/* multiple controllers on same maj dev # (mscp) */
		i = (flag - 1);
		printf("************* Beginning of %stab: controller %d *************\n\n", dev[maj].name, i);
		if(!i) {
			bp = &devbuf[maj];
			cchk(bp,bsw[maj].d_tab,1,&name);
		} else {
			switch(maj) {
			case RA_BMAJ:
				bp = &ra_devt[i];
				break;
			case TK_BMAJ:
				bp = &tk_devt[i];
				break;
			case TS_BMAJ:
				bp = &ts_devt[i];
				break;
			/* DEFAULT NOT NEEDED */
			}
			cchk(bp,((struct buf *)bsw[maj].d_tab + i),1,&name);
		}
	} else { 			/* normal */
		printf("******************** Beginning of %stab ********************\n\n", dev[maj].name);
		bp = &devbuf[maj];
		cchk(bp,bsw[maj].d_tab,1,&name);
	}
	sprintf(temp, "%stab: address = %o", dev[maj].name, ((struct buf *)bsw[maj].d_tab + i));
	prbuf(bp, 1, temp, left);
	x = 1;
	while(bp->av_forw != NULL) {
		diff = bufnum(bp->b_forw);
		if((diff == -1) || (diff == 01000))
			break;
		else if(diff == 05000) {
			bp = &swbuf1;
			sprintf(temp,"swbuf1: address = %-8o", diff, ((struct buf*)misc[0].n_value));
		} else if(diff == 05001) {
			bp = &swbuf2;
			sprintf(temp,"swbuf2: address = %-8o", diff, ((struct buf*)misc[1].n_value));
		} else if(diff == 05002) {
			bp = &rawtab;
			sprintf(temp,"rawtab: address = %-8o", diff, ((struct buf*)misc[2].n_value));
		} else if(diff == 05003) {
			bp = &tkwtab;
			sprintf(temp,"tkwtab: address = %-8o", diff, ((struct buf*)misc[3].n_value));
		} else if(diff < 01000) {
			bp = &bufx[diff];
			sprintf(temp,"buf #%3d: address = %-8o", diff, ((struct buf*)bqnl[2].n_value + diff));
		} else if(diff >= 04000) {
			bp = ((struct buf *)rawbuf + (diff - 04000));
			for(i = 0; i < 6; i++)
				local[i] = rbuf[(diff & 017)].n_name[i+1];
			j = (((diff - 04000) >> 4) - rboff[diff&017]);
			if(j <= 7) {
				local[i++] = ' ';
				local[i++] = (char)('0' + j);
			}
			local[i] = '\0';
			sprintf(temp,"%s: address = %-8o", local,((struct buf *)rbuf[diff&017].n_value) + rboff[diff&017] + ((diff - 04000) >> 4));
		} else if(diff >= 03000) {
			bp = ((struct buf *)utab + (diff - 03000));
			for(i = 0; i < 6; i++)
				local[i] = dutab[(diff & 07)].n_name[i+1];
			j = (((diff - 03000) >> 3) - bfoff[diff&07]);
			if(j <= 7) {
				local[i++] = ' ';
				local[i++] = (char)('0' + j);
			}
			local[i] = '\0';
			sprintf(temp,"%s: address = %-8o", local, (struct buf *)dutab[diff&07].n_value + bfoff[diff&07] + ((diff - 03000) >> 3));
		} else {
			printf("device queue fatal error\n");
			break;
		}
		if(x == 1) {
			x++;
			prbuf(bp,1,temp,right);
			output(x);
			x = 0;
		} else {
			prbuf(bp,1,temp,left);
			x++;
		}
	}
	if(x)
		output(x);
	printf("************************ End of %stab **********************\n\n", dev[maj].name);
}

/*
 * print a utab queue
 */
prudev(maj,pos,off)
int maj,pos,off;
/*
 * maj == blk maj dev #
 * pos == position of device in dutab[] and nbh[] arrays and
 * 		controller number for mscp devices
 * off == offset into array of utabs
 */
{
	struct buf *bp;
	char temp[40];
	int tmp, diff;
	int i, j;
	int lk;
	char name[20];
	int bufoff, x;

	x = 0;
	sprintf(name,"%sutab\0",dev[maj].name);
	temp[0] = '\0';
	bp = utab; 		/* set bp to base of utab area */
	if(maj == 2) { 	/* mscp is first item in utab */
		lk = 0;
		bp += (off & 0377);
		j = (off >> 8);
		if(j > 0)
			lk = xnra[0];
		if(j > 1)
			lk += xnra[1];
		if(j > 2)
			lk += xnra[2];
		printf("******** Beginning of %sutab: controller %d: drive %d ********\n\n", dev[maj].name, j, ((off & 0377) - lk));
		cchk(bp,((struct buf *)dutab[pos].n_value + (off & 0377)),1,&name);
 		sprintf(temp, "%sutab: address = %o", dev[maj].name, ((struct buf *)dutab[pos].n_value + (off & 0377)));
	} else { 			/* normal */
		switch(pos) {
		case 1:		/* rl */
			bufoff = (nbkd[2] + off);
			bp += bufoff;
			lk = off;
			break;
		case 5:		/* hk */
			bufoff =  (nbkd[2] + nbkd[3] + nbkd[9] + nbkd[10] + nbkd[11] + off);
			bp += bufoff;
			lk = off;
			break;
		case 2:		/* h[pmj] */
			bufoff = (nbkd[2] + nbkd[3] + off);
			bp += bufoff;
			lk = off;
			for(i = 0; lk >= nhpa[i] ;i++) {
				x++;
				lk -= nhpa[i];
			}
			break;
		}
		printf("************** Beginning of %sutab: drive %d ****************\n\n", dev[maj].name, lk);
		cchk(bp,((struct buf *)dutab[pos].n_value + bufoff),1,&name);
		sprintf(temp, "%sutab: address = %o", dev[maj].name, ((struct buf *)dutab[pos+x].n_value + lk));
	}
	prbuf(bp, 1, temp, left);
	i = 1;
	while(bp->av_forw != NULL) {
		diff = bufnum(bp->b_forw);
		if((diff == -1) || (diff == 01000))
			break;
		else if(diff == 05000)
			bp = & swbuf1;
		else if(diff == 05001)
			bp = &swbuf2;
		else if(diff == 05002)
			bp = &rawtab;
		else if(diff == 05003)
			bp = &tkwtab;
		else if(diff < 01000) {
			bp = &bufx[diff];
			sprintf(temp,"buf #%3d: address = %-8o", diff, ((struct buf*)bqnl[2].n_value + diff));
		} else {
			printf("device queue fatal error\n");
			break;
		}
		if(i == 1) {
			i++;
			prbuf(bp,1,temp,right);
			output(i);
			i = 0;
		} else {
			prbuf(bp,1,temp,left);
			i++;
		}
	}
	if(i)
		output(i);
	printf("********************** End of %sutab ***********************\n\n", dev[maj].name);
}

/*
 * actually print the buffer
 */
output(x)
int x;
{
	int i;

	if(x == 1) {
		for(i = 0; i < 7; i++)
			printf("%-35s\n", (left+i));
	} else {
		for(i = 0; i < 7; i++) {
			printf("%-35s", (left+i));
			printf("%-35s\n", (right+i));
		}
	}
	printf("\n\n");
}

/*
 * check a queue for consistency of
 * the forward backward pointers
 */

cchk(bp,rmbp,type,name)
struct buf *bp;			/* starting place */
struct buf *rmbp;		/* real mem addr */
int type;
char *name;
{
	struct buf *x, *y, *z;
	int bad, count, diff, next, indx;

	count = bad = 0;
	if(type == 1) {
		if((bp->av_forw == rmbp) && (bp->av_back == rmbp)) {
			goto out;
		} else if(bp->av_forw == NULL) {
			goto out;
		} else if(bp->av_forw == bp->av_back) {
			count++;
			diff = bufnum(bp->av_forw);
			if((diff == -1) || ((diff >= 01002) && (diff < 02000)))
				bad++;
			else if(diff == 05000)
				x = &swbuf1;
			else if(diff == 05001)
				x = &swbuf2;
			else if(diff == 05002)
				x = &rawtab;
			else if(diff == 05003)
				x = &tkwtab;
			else if(diff >= 04000) {
				x = ((struct buf *)rawbuf + rboff[diff & 017]);
				x += ((diff - 04000) >> 4);
			} else if(diff >= 03000)
				x = ((struct buf *)utab + ((diff - 03000) >> 3));
			else if(diff >= 02000)
				x = bsw[(diff - 02000)].d_tab;
			else
				x = &bufx[diff];
			if(x->av_forw != rmbp)
				bad++;
			if(x->av_back != rmbp)
				bad++;
			goto out;
		} else {
			x = bp;
			diff = bufnum(x->av_forw);
			if((diff == -1) || ((diff >= 01002) && (diff < 02000))) {
				bad++;
				goto out;
			} else if(diff == 05000)
				y = &swbuf1;
			else if(diff == 05001)
				y = &swbuf2;
			else if(diff == 05002)
				y = &rawtab;
			else if(diff == 05003)
				y = &tkwtab;
			else if(diff >= 04000) {
				y = ((struct buf *)rawbuf + rboff[diff & 017]);
				y += ((diff - 04000) >> 4);
			} else if(diff >= 03000)
				y = ((struct buf *)utab + ((diff - 03000) >> 3));
			else if(diff >= 02000)
				y = bsw[(diff - 02000)].d_tab;
			else
				y = &bufx[diff];
			if(y->av_back != rmbp) {
				bad++;
				goto out;
			}
			count++;
			while(y->av_forw != rmbp) {
				count++;
				diff = bufnum(y->av_forw);
				if((diff == -1) || ((diff >= 01002) && (diff < 02000))) {
					bad++;
				} else if(diff == 05000)
					z = &swbuf1;
				else if(diff == 05001)
					z = &swbuf2;
				else if(diff == 05002)
					z = &rawtab;
				else if(diff == 05003)
					z = &tkwtab;
				else if(diff >= 04000) {
					z = ((struct buf *)rawbuf + rboff[diff & 017]);
					z += ((diff - 04000) >> 4);
				} else if(diff >= 03000)
					z = ((struct buf *)utab + ((diff - 03000) >> 3));
				else if(diff >= 02000)
					z = bsw[(diff - 02000)].d_tab;
				else
					z = &bufx[diff];
				if(x->av_forw != z->av_back) {
					bad++;
					goto out;
				}
				x = y;
				y = z;
			}
			if(bp->av_back != x->av_forw) {
				bad++;
				goto out;
			}
		}
	} else if(type == 2) {
		if((bp->b_forw == rmbp) && (bp->b_back == rmbp)) {
			goto out;
		} else if(bp->b_forw == NULL) {
			goto out;
		} else if(bp->b_forw == bp->b_back) {
			count++;
			diff = bufnum(bp->b_forw);
			if(diff == 01000)
				goto out;
			else if((diff == -1) || ((diff >= 01002) && (diff < 02000)))
				bad++;
			else if(diff == 05000)
				x = &swbuf1;
			else if(diff == 05001)
				x = &swbuf2;
			else if(diff == 05002)
				x = &rawtab;
			else if(diff == 05003)
				x = &tkwtab;
			else if(diff >= 04000) {
				x = ((struct buf *)rawbuf + rboff[diff & 017]);
				x += ((diff - 04000) >> 4);
			} else if(diff >= 03000)
				x = ((struct buf *)utab + ((diff - 03000) >> 3));
			else if(diff >= 02000)
				x = bsw[(diff - 02000)].d_tab;
			else
				x = &bufx[diff];
			if(x->b_forw != rmbp)
				bad++;
			if(x->b_back != rmbp)
				bad++;
			goto out;
		} else {
			x = bp;
			diff = bufnum(x->b_forw);
			if((diff == -1) || ((diff >= 01002) && (diff < 02000))) {
				bad++;
				goto out;
			} else if(diff == 05000)
				y = &swbuf1;
			else if(diff == 05001)
				y = &swbuf2;
			else if(diff == 05002)
				y = &rawtab;
			else if(diff == 05003)
				y = &tkwtab;
			else if(diff >= 04000) {
				y = ((struct buf *)rawbuf + rboff[diff & 017]);
				y += ((diff - 04000) >> 4);
			} else if(diff >= 03000)
				y = ((struct buf *)utab + ((diff - 03000) >> 3));
			else if(diff >= 02000)
				y = bsw[(diff - 02000)].d_tab;
			else
				y = &bufx[diff];
			if(y->b_back != rmbp) {
				bad++;
				goto out;
			}
			count++;
			while(y->b_forw != rmbp) {
				count++;
				diff = bufnum(y->b_forw);
				if((diff == -1) || ((diff >= 01002) && (diff < 02000))) {
					bad++;
				} else if(diff == 05000)
					z = &swbuf1;
				else if(diff == 05001)
					z = &swbuf2;
				else if(diff == 05002)
					z = &rawtab;
				else if(diff == 05003)
					z = &tkwtab;
				else if(diff >= 04000) {
					z = ((struct buf *)rawbuf + rboff[diff & 017]);
					z += ((diff - 04000) >> 4);
				} else if(diff >= 03000)
					z = ((struct buf *)utab + ((diff - 03000) >> 3));
				else if(diff >= 02000)
					z = bsw[(diff - 02000)].d_tab;
				else
					z = &bufx[diff];
				if(x->b_forw != z->b_back) {
					bad++;
					goto out;
				}
				x = y;
				y = z;
			}
			if(bp->b_back != x->b_forw) {
				bad++;
				goto out;
			}
		}
	}
out:
	printf("%d buffer headers in %s queue\n", count, name);
	printf("%s consistency check: ", name);
	if(bad)
		printf("bad\n\n");
	else
		printf("OK\n\n");
}

/*
 * routine to obtain values for block I/O
 * devices
 */

bioinfo()
{
	int i, j, rt;

	if(flags & BIOINFO)
		return;
	if(!(flags & RAINFO))
		rainfo();
	if(!(flags & TKINFO))
		tkinfo();
	nlist(nlfil, bqnl);
	nlist(nlfil, dutab);
	nlist(nlfil, rbuf);
	nlist(nlfil, misc);
	nlist(nlfil, ndev);
	if(bqnl[0].n_type == 0) {
		printf("Can't access name list in %s\n", nlfil);
		exit(1);
	}
	lseek(mem, (long)bqnl[6].n_value, 0);
	if(read(mem, &nblkdev, sizeof(nblkdev)) != sizeof(nblkdev))
		printf("%s", nlre);
	if(nblkdev > MBLKDEV) {
		printf("\nKernel has more than %d block devices in bdevsw[]!",
			MBLKDEV);
		exit(1);
	}
	lseek(mem, (long)bqnl[0].n_value, 0);
	if(read(mem, &nbuf, sizeof(nbuf)) != sizeof(nbuf))
		printf("%s", nlre);
	lseek(mem, (long)bqnl[1].n_value, 0);
	if(read(mem, &frbuf, sizeof(struct buf)) != sizeof(struct buf))
		printf("%s", nlre);
	lseek(mem, (long)bqnl[2].n_value, 0);
	bufbas = calloc(nbuf, sizeof(struct buf));
	bufx = bufbas;
	if(bufx == NULL) {
		printf("cda: can\'t allocate memory for buffer headers !\n");
		exit(1);
	}
	for(i = 0; i < nbuf; i++)
		if(read(mem, bufx++, sizeof(struct buf)) != sizeof(struct buf))
			printf("%s", nlre);
	bufx = bufbas;
	lseek(mem, (long)bqnl[3].n_value, 0);
	if(read(mem, &bsw, sizeof(bsw)) != sizeof(bsw))
		printf("%s", nlre);
	for(i = 0; i < nblkdev; i++) {
		if(!bsw[i].d_tab)
			continue;
		lseek(mem, (long)bsw[i].d_tab, 0);
		if(read(mem, &devbuf[i], sizeof(struct buf)) != sizeof(struct buf))
			printf("%s", nlre);
	}
/*
 * Find any additional ??tab device tables,
 * which may exist for drivers that support
 * multiple controllers (RA, TK, TS, HP).
 */
	if(nuda) {
		lseek(mem, (long)bsw[RA_BMAJ].d_tab, 0);
		j = sizeof(struct buf) * nuda;
		if(read(mem, (char *)&ra_devt, j) != j)
			printf("%s", nlre);
	}
	if(ntk) {
		lseek(mem, (long)bsw[TK_BMAJ].d_tab, 0);
		j = sizeof(struct buf) * ntk;
		if(read(mem, (char *)&tk_devt, j) != j)
			printf("%s", nlre);
	}
	if(nts) {
		lseek(mem, (long)bsw[TS_BMAJ].d_tab, 0);
		j = sizeof(struct buf) * nts;
		if(read(mem, (char *)&ts_devt, j) != j)
			printf("%s", nlre);
	}
	lseek(mem, (long)misc[0].n_value, 0);
	if(read(mem, &swbuf1, sizeof(swbuf1)) != sizeof(swbuf1))
		printf("%s", nlre);
	lseek(mem, (long)misc[1].n_value, 0);
	if(read(mem, &swbuf2, sizeof(swbuf2)) != sizeof(swbuf2))
		printf("%s", nlre);
	lseek(mem, (long)misc[3].n_value, 0);
	if(read(mem, &tkwtab, sizeof(struct buf)) != sizeof(struct buf))
		printf("%s", nlre);
	lseek(mem, (long)bqnl[4].n_value, 0);
	if(read(mem, &nmount, sizeof(nmount)) != sizeof(nmount))
		printf("%s", nlre);
	mntbas = calloc(nmount, sizeof(struct mount));
	mntp = mntbas;
	if(mntp == NULL) {
		printf("cda: can\'t allocate memory for mount table !\n");
		exit(1);
	}
	lseek(mem, (long)bqnl[5].n_value, 0);
	for(i = 0; i < nmount; i++)
		if(read(mem, mntp++, sizeof(struct mount)) != sizeof(struct mount))
			printf("%s", nlre);
/* determine number of rxxbuf's */
	for(i = 0; i < NNDEV; i++) {
		switch(i) {
		case RA_BMAJ:
			nbkd[i] = nra;
			break;
		case TK_BMAJ:
			nbkd[i] = ntk;
			break;
		case TS_BMAJ:
			nbkd[i] = nts;
			break;
		case HP_BMAJ:
			if(ndev[i].n_value) {
				lseek(mem, (long)ndev[i].n_value, 0);
				read(mem, &nhpa, sizeof(nhpa));
				nbkd[9] = nhpa[0];
				nbkd[10] = nhpa[1];
				nbkd[11] = nhpa[2];
			}
			break;
		case HM_BMAJ:
		case HJ_BMAJ:
			break;
		default:
			if(ndev[i].n_value) {
				lseek(mem, (long)ndev[i].n_value, 0);
				if(read(mem, &nbkd[i], sizeof(int)) != sizeof(int))
					printf("%s", nlre);
			}
			break;
		}
		tbd += nbkd[i];
	}
	nts = nbkd[7];
/* variables setup for utab structures */
	nbh[0] = nbkd[2];
	nbh[1] = nbkd[3];
	nbh[2] = nbkd[9];
	nbh[3] = nbkd[10];
	nbh[4] = nbkd[11];
	nbh[5] = nbkd[12];
	nutab = nbkd[2] + nbkd[3] + nbkd[9] + nbkd[10] + nbkd[11] + nbkd[12];

	utab = calloc(nutab, sizeof(struct buf));
	rawbuf = calloc(tbd, sizeof(struct buf));
	rbufp = rawbuf;
	utabp = utab;
	if((utabp == NULL) || (rawbuf == NULL)) {
		printf("cda: can\'t allocate memory for buffer headers !\n");
		exit(1);
	}
	rt = 0;
	for(i = 0; i < NRAWBUF; i++) {
		lseek(mem, (long)rbuf[i].n_value, 0);
		rboff[i] = rt;
		rt += nbkd[i];
		for(j = 0; j < nbkd[i]; j++)
			if(read(mem, rbufp++, sizeof(struct buf)) != sizeof(struct buf))
				printf("%s", nlre);
	}
	if(nbkd[2]) {	/* mscp */
		bfoff[0] = 0;
		lseek(mem, (long)misc[2].n_value, 0);
		if(read(mem, &rawtab, sizeof(struct buf)) != sizeof(struct buf))
			printf("%s", nlre);
		lseek(mem, (long)dutab[0].n_value, 0);
		for(i = 0; i < nbkd[2]; i++)
			if(read(mem, utabp++, sizeof(struct buf)) != sizeof(struct buf))
				printf("%s", nlre);
	}
	if(nbkd[3]) {	/* rl */
		bfoff[1] = nbkd[2];
		lseek(mem, (long)dutab[1].n_value, 0);
		for(i = 0; i < nbkd[3]; i++)
			if(read(mem, utabp++, sizeof(struct buf)) != sizeof(struct buf))
				printf("%s", nlre);
	}
	if(nbkd[9]) {	/* hp */
		bfoff[2] = (nbkd[2] + nbkd[3]);
		lseek(mem, (long)dutab[2].n_value, 0);
		for(i = 0; i < (nbkd[9] + nbkd[10] + nbkd[11]); i++)
			if(read(mem, utabp++, sizeof(struct buf)) != sizeof(struct buf))
				printf("%s", nlre);
	}
	if(nbkd[10]) {	/* hm */
		bfoff[3] = (nbkd[2] + nbkd[3] + nbkd[9]);
		dutab[3].n_value = ((struct buf *)dutab[2].n_value + nbkd[9]);
		rbuf[10].n_value = ((struct buf *)rbuf[9].n_value + nbkd[9]);
	}
	if(nbkd[11]) {	/* hj */
		bfoff[4] = (nbkd[2] + nbkd[3] + nbkd[9] + nbkd[10]);
		dutab[4].n_value = ((struct buf *)dutab[2].n_value + nbkd[9] + nbkd[10]);
		rbuf[11].n_value = ((struct buf *)rbuf[9].n_value + nbkd[9] + nbkd[10]);
	}
	if(nbkd[12]) {	/* hk */
		bfoff[5] = (nbkd[2] + nbkd[3] + nbkd[9] + nbkd[10] + nbkd[11]);
		lseek(mem, (long)dutab[5].n_value, 0);
		for(i = 0; i < nbkd[12]; i++)
			if(read(mem, utabp++, sizeof(struct buf)) != sizeof(struct buf))
				printf("%s", nlre);
	}
	utabp = utab;
	rbufp = rawbuf;
}

/*
 * print rawtab
 */
prraw(maj)
{
	struct buf *bp, *obp;
	char temp[40];
	int tmp, diff, x, i, k, l;
	char name[20];
	int xx;
	char local[40];
	
	if (maj == 2) {
		bp = &rawtab;
		i = 2;
	} else {
		bp = &tkwtab;
		i = 3;
	}
	sprintf(name,"%swtab\0",dev[maj].name);
	printf("******************** Beginning of %swtab ********************\n\n", dev[maj].name);
	cchk(bp,misc[i].n_value,1,&name);
	sprintf(temp, "%swtab: address = %o", dev[maj].name, (struct buf *)misc[i].n_value);
	prbuf(bp, 1, temp, left);
	x = 1;
	while((bp->av_forw != NULL) && (bp->av_forw != misc[i].n_value)) {
		diff = bufnum(bp->av_forw);
		if((diff == -1) || (diff == 01000))
			break;
		else if(diff == 05000) {
			bp = &swbuf1;
			sprintf(temp,"swbuf1: address = %-8o", (struct buf *)misc[0].n_value);
		} else if(diff == 05001) {
			bp = &swbuf2;
			sprintf(temp,"swbuf2: address = %-8o", (struct buf *)misc[1].n_value);
		} else if(diff == 05002) {
			bp = &rawtab;
			sprintf(temp,"rawtab: address = %-8o", (struct buf*)misc[2].n_value);
		} else if(diff == 05003) {
			bp = &tkwtab;
			sprintf(temp,"tkwtab: address = %-8o", (struct buf*)misc[3].n_value);
		} else if(diff < 01000) {
			bp = &bufx[diff];
			sprintf(temp,"buf #%3d: address = %-8o", diff, ((struct buf*)bqnl[2].n_value + diff));
		} else if(diff >= 04000) {
			bp = ((struct buf *)rawbuf + ((diff - 04000) >> 4));
			for(k = 0; k < 6; k++)
				local[k] = rbuf[(diff & 017)].n_name[k+1];
			l = (((diff - 04000) >> 4) - rboff[diff&017]);
			if(l <= 7) {
				local[k++] = ' ';
				local[k++] = (char)('0' + l);
			}
			local[k] = '\0';
			sprintf(temp,"%s: address = %-8o", local,((struct buf *)rbuf[diff&017].n_value) + rboff[diff&017] + ((diff - 04000) >> 4));
		} else if(diff >= 03000) {
			bp = ((struct buf *)utab + ((diff - 03000) >> 3));
			for(k = 0; k < 7; k++)
				local[k] = dutab[(diff & 07)].n_name[k+1];
			if(l <= 7) {
				local[k++] = ' ';
				local[k++] = (char)('0' + l);
			}
			local[k] = '\0';
			sprintf(temp,"%s: address = %-8o", local, (struct buf *)dutab[diff&07].n_value + bfoff[diff&07] + ((diff - 03000) >> 3));
		} else if(diff >= 02000) {
			bp = &devbuf[(diff - 02000)];
		} else {
			printf("device queue fatal error\n");
			break;
		}
		if(x == 1) {
			x++;
			prbuf(bp,1,temp,right);
			output(x);
			x = 0;
		} else {
			prbuf(bp,1,temp,left);
			x++;
		}
	}
	if(x)
		output(x);
	printf("*********************** End of %swtab **********************\n\n", dev[maj].name);
}
