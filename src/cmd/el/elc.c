
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)elc.c	3.0	4/21/86";
/*
 * ULTRIX-11 error logging copy program (elc).
 *
 * Copies error log records from the kernel buffer
 * to the error log device `/dev/errlog', usually
 * located in a protion of the swap area.
 *
 * Fred Canter 5/11/83
 */

#include <sys/param.h>	/* does not matter which one */
#include <sys/tmscp.h>	/* must preceed errlog.h (EL_MAXSZ) */
#include <sys/errlog.h>
#include <a.out.h>
#include <stdio.h>
#include <signal.h>

#define	R	0
#define	W	1
#define	RW	2

/*
 * System error log information,
 * obtained from unix via the errlog system call (EL_INFO).
 */

struct	el_data	el_data;
dev_t	el_dev;		/* major/minor device of error log device */
daddr_t	el_sb;		/* error log starting block number */
int	el_nb;		/* error log length in blocks */

/* input buffer, holds one record from the kernel buffer. */

int	ibuf[EL_MAXSZ];

/* output buffer, disk i/o buffer for error log device. */

int	obuf[256+1];

int	elbn;	/* current active block # on error log device */
int	fio;
int	elfirst;

char	*errdev = "/dev/errlog";

main ()
{
	register struct elrhdr *ehp;
	register *bpi, *bpo;
	int bwflag, d, i;
	int elb, sz;
	int mem;

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	close(stdin);
	close(stdout);
/*
 * Make this a system process, so that it will not
 * be swapped (locked in memory).
 */
	lock(1);

/* Give this process a hi priority. */

/* only SU can execute /etc/elc 
	if(getuid()) {
		fprintf(stderr,"\nelc: not SU\n");
		exit(1);
	}
 */
	nice((int) -20);
/*
 * Wait for the first error to be logged,
 * this will always be and error log startup.
 * This is done to insure that the unix monitor
 * is named unix and that the /dev/errlog file
 * is set up for the correct device.
 * If this was not done elc would fail when
 * attempting to locate the first free record in the
 * error log.
 */
/*	errlog(EL_WAIT, 0);	*/	
/*
 * Get error log information from unix
 * via the errlog system call.
 */
	errlog(EL_INFO, &el_data);
	el_dev = el_data.el_dev;
	el_sb = el_data.el_sb;
	el_nb = el_data.el_nb;
/*
 * Make the special file `/dev/errlog',
 * this insures that it is always the correct
 * major/minor device. 
 */
	unlink(errdev);
	if(mknod(errdev, 020600, el_dev) < 0) {
		fprintf(stderr, "\nelc: Can't mknod %s\n", errdev);
		exit(1);	/* FATAL ! */
	}

/* open error log device for read & write */

	if((fio = open(errdev, RW)) < 0) {
		fprintf(stderr, "\nelc: Can't open %s\n", errdev);
		exit(1);	/* FATAL ! */
		}

/*
 * Find the block in the error log that contains the next 
 * open slot, read it into the disk i/o buffer and
 * set the disk i/o buffer pointer.
 */

elinit:
	for(bpo = &obuf; bpo < &obuf[256]; *bpo++ = 0);
	lseek(fio, (long)(el_sb * 512), 0);
	for(i=0; i<el_nb; i++) {
		if(read(fio, (char *)&obuf, 512) != 512) {
			fprintf(stderr,"\nelc: read error bn = %D,", i+el_sb);
			goto elwait;
			}
		bpo = &obuf;
		ehp = bpo;
		while((bpo < &obuf[256]) && (ehp->e_type != E_EOB)) {
		if(ehp->e_type == E_EOF)	/* empty slot */
			goto out;
		if(elfirst++ == 0) {	/* check for error log initialized */
		 if((ehp->e_type != E_SU) ||
		  (ehp->e_size != (sizeof(struct elrhdr)+sizeof(struct el_su))))
			goto elwait1;
		}
		if((ehp->e_type < 0) ||
			(ehp->e_type > E_BD) ||
			(ehp->e_size == 0) ||
			(ehp->e_size & 1) ||
			(ehp->e_size > EL_MAXSZ)) {
	badrec:
			fprintf(stderr, "\7\7\7elc: bad error record, ");
			goto elwait;
			}
		bpo += (ehp->e_size / 2);	/* not empty, adjust pointer */
		ehp = bpo;
		}
	}
out:
	elbn = i;	/* save block number */

/*
 * Get one record from the kernel error buffer,
 * move it to the disk I/O buffer,
 * and write to the error log device as required.
 */

ecloop:

	bpi = &ibuf;
	ehp = bpi;
	errlog(EL_REC, (caddr_t)&ibuf);		/* get one record */
	if(ehp->e_type == E_INIT)	/* error log has been reinitialized */
		goto elinit;	/* reset error log file pointers. */
	if(ehp->e_type == E_BADR)
		goto badrec;		/* bad error record detected */
	if(ehp->e_type == E_EOF) {	/* kernel buffer is empty */
		if(bwflag) {	/* if record added to block */
		if(bwrite(elbn))	/* update error log */
			goto elwait;	/* write error on error log */
		bwflag = 0;
		if((el_nb - elbn) < 4) {
/*
 * Only 3 free blocks remain in the error log,
 * print the number of blocks used message
 * as a warning.
 */
			elb = elbn;
			if(elb >= el_nb)
				elb = el_nb - 1;
			fprintf(stderr,"\n\07\07\07elc: ERROR LOG has - ");
			fprintf(stderr,"%d of %d blocks used\n",++elb,el_nb);
			}
		}
		errlog(EL_WAIT, 0);	/* sleep at hi priority */
					/* awaken when errors to log */
		goto ecloop;
		}
	
/*
 * Move the record to the disk buffer
 */

	/* d is set to amount of space remaining in */
	/* obuf and checked to see if the record will fit. */

	sz = ehp->e_size / 2;	/* size of error log record */
	d = (&obuf[256] - (bpo + sz));
	if(d == 0) {	/* buffer will be exactly full */
		for(i=sz; i; i--)
			*bpo++ = *bpi++;
		if(elwb())
			goto elwait;	/* write error on error log */
		bpo = &obuf;
		bwflag = 0;	/* error log updated */
		}
	else if(d > 0) {	/* record fits, buffer not full */
		for(i=sz; i; i--)
			*bpo++ = *bpi++;
		bwflag++;	/* error log will require update */
		}
	else {			/* record will not fit */
		*bpo = E_EOB;	/* fake end of block */
		if(elwb())
			goto elwait;	/* write error on error log */
		bpo = &obuf;
		for(i=sz; i; i--)
			*bpo++ = *bpi++;
		bwflag++;
		}
	goto ecloop;
/*
 * Come here on error log errors.
 * Cause ELC to hang until the error log 
 * file is reinitialized.
 */

elwait:
	fprintf(stderr, " error logging disabled\n");
elwait1:
	errlog(EL_OFFNL, 0);	/* disable error logging, don't log it ! */
	errlog(EL_WAIT, 0);	/* wait for next error log record */
	ehp = &ibuf;
	errlog(EL_REC, (caddr_t)&ibuf);	/* fetch record */
	if(ehp->e_type == E_INIT)	/* is it error log init */
		goto elinit;		/* yes, restart ELC */
	goto elwait1;			/* no, wait for init record */

}

/* write a block to the error log device */

bwrite(bno)
{
	if(bno >= el_nb) {	/* don't allow write past end of error log */
		errlog(EL_OFFNL, 0);	/* turn logging off !!! */
		fprintf(stderr,"\nelc: error log full,");
		return(1);
		}
	lseek(fio, (long)((bno + el_sb) * 512), 0);
	if(write(fio, (char *)&obuf, 512) != 512) {
		fprintf(stderr,"\nelc: write error bn = %D,",bno+el_sb);
		errlog(EL_OFFNL, 0);	/* turn error logging off */
		return(1);
	}
	return(0);
}

/*
 * Write the disk buffer out to the current block,
 * zero the disk buffer,
 * and increment the block number (elbn).
 */

elwb()
{
	register int *p;

	if(bwrite(elbn))
		return(1);	/* write error on error log */
	elbn++;
	for(p = &obuf; p < &obuf[256]; *p++ = 0);
	return(0);
}
