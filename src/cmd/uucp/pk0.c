
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)pk0.c	3.0	4/22/86";


/*
 * packet driver
 */

/* decvax!larry - added significant number of comments */

extern	char	*malloc();

#define USER	1
#include <stdio.h>
#include <sys/param.h>
#include <sys/buf.h>
#ifdef	PADDR
#undef	PADDR
#endif	PADDR
#include "pk.p"
#include "pk.h"



char next[8]	={ 1,2,3,4,5,6,7,0};	/* packet sequence numbers */
char mask[8]	={ 1,2,4,010,020,040,0100,0200 };

struct pack *pklines[NPLINES];


/*
 * receive control messages
 * pkcntl() implements the protocol "state machine"
 * - the initialization stage consists of three steps:
 *  1. send window size (and receive window size) - INITA
 *  2. send packet size (and receive packet size) - INITAB
 *  3. send final ack. - INITC
 * - after the init stage data can be sent or received.
 * - data is transferred in packets until the window is reached
 * - acks are returned for each packet received after which time
 *	another packet can be sent.
 * 
 */
pkcntl(c, pk)
register struct pack *pk;
{
register cntl, val;

	val = c & MOD8;
	cntl = (c>>3) & MOD8;

	if ( ! ISCNTL(c) ) {
		fprintf(stderr, "not cntl\n");
		return;
	}

	if (pk->p_mode & 02)
		fprintf(stderr, "%o ",c);
	switch(cntl) {

	case INITB:
		/*   get packet size */
		val++;
		pk->p_xsize = pksizes[val];
		pk->p_lpsize = val;
		pk->p_bits = DTOM(pk->p_xsize);
		if (pk->p_state & LIVE) {
			pk->p_msg |= M_INITC;
			break;
		}
		pk->p_state |= INITb;
		if ((pk->p_state & INITa)==0) {

		/*   if remote side has not entered state A it has not 
 		*   received our original control message.
 		*   therefore send original message again
 		*/

#ifdef DEBUGOUT
		fprintf(stderr, "not in A got INITB: state=%o, p_msg=%o, p_rmsg=%o, p_swindow=%o\n",
			pk->p_state, pk->p_msg, pk->p_rmsg, pk->p_swindow);
#endif
			break;
		}
		/* sync'ed up so far. Try for final state: INITC - send ack. */
		pk->p_rmsg &= ~M_INITA;
		pk->p_msg |= M_INITC;
#ifdef DEBUGOUT
		fprintf(stderr, "In A got INITB: state=%o, p_msg=%o, p_rmsg=%o, p_swindow=%o\n",
			pk->p_state, pk->p_msg, pk->p_rmsg, pk->p_swindow);
#endif
		break;

	case INITC:
		/* get final ack */
		if ((pk->p_state&INITab)==INITab) {
			/*  both sides are now ready */
			pk->p_state = LIVE;
			WAKEUP(&pk->p_state);
			pk->p_rmsg &= ~M_INITB;
#ifdef DEBUGOUT
			fprintf(stderr, "In AB got INITC: state=%o, p_msg=%o, p_rmsg=%o, p_swindow=%o\n",
			pk->p_state, pk->p_msg, pk->p_rmsg, pk->p_swindow);
#endif
		} else {
			/* remote did not receive INITB message */
			pk->p_msg |= M_INITB;
#ifdef DEBUGOUT
			fprintf(stderr, "Not in AB got INITB: state=%o, p_msg=%o, p_rmsg=%o, p_swindow=%o\n",
			pk->p_state, pk->p_msg, pk->p_rmsg, pk->p_swindow);
#endif
		}
		if (val)
			pk->p_swindow = val;
		break;
	case INITA:
		if (val==0 && pk->p_state&LIVE) {
			fprintf(stderr, "alloc change not implemented\n");
			break;
		}
		/* val is the recive window at the remote site */
		if (val) {
			pk->p_state |= INITa;
			pk->p_msg |= M_INITB;
			pk->p_rmsg |= M_INITB;
			pk->p_swindow = val;
#ifdef DEBUGOUT
			fprintf(stderr, "received INITA: state=%o, p_msg=%o, p_rmsg=%o, p_swindow=%o\n",
			pk->p_state, pk->p_msg, pk->p_rmsg, pk->p_swindow);
#endif
		}
		break;
	case RJ:   /* packet was rejected at remote site - send again */
#ifdef DEBUGOUT
		fprintf(stderr, "pkcntl, case=RJ\n");
#endif
		Totalrxmts++;
		pk->p_state |= RXMIT;
		pk->p_msg |= M_RR;
	case RR:
		/* packet was acknowledged or was a dup packet.
		 * In either case we dont have to send it again.
		 */

#ifdef DEBUGOUT
		fprintf(stderr, "pkcntl, case=RR\n");
#endif
		pk->p_rpr = val;
		if (pksack(pk)==0) {
			WAKEUP(&pk->p_ps);
		}
		break;
	case SRJ:
		fprintf(stderr, "srj not implemented\n");
		break;
	case CLOSE:
#ifdef DEBUGOUT
		fprintf(stderr, "pkcntl, case=CLOSE\n");
#endif
		pk->p_state = DOWN+RCLOSE;
		SIGNAL;
		WAKEUP(&pk->p_pr);
		WAKEUP(&pk->p_ps);
		WAKEUP(&pk->p_state);
		return;
	}
out:
	/* send response */
	if (pk->p_msg)
		pkoutput(pk);
}

/* pkaccept is equivalent to ttyinput. i.e. it is the part
 * of the line discipline which reads in the data off 
 * the network and wakes up the higher level pkread routine.
 * Of course this is not quite the way it goes since it is no
 * longer is a line discipline.
 */

pkaccept(pk)
register struct pack *pk;
{
register x,seq;
char m, cntl, *p, imask, **bp;
int bad,accept,skip,s,t,h,cc;
unsigned short sum;


	bad = accept = skip = 0;
	/*
	 * wait for input
	 */
	LOCK;
	x = next[pk->p_pr];   /* pk->p_pr is the last packet# received */

	/* wait for packet to arrive 
	 *  p_imap == 0 implies no data has arrived
	 */

	while ((imask=pk->p_imap) == 0 && pk->p_rcount==0) {
	/* p_imap is a bit map of input buffers
	 * rcount is the # of active receive buffers
	 */

#ifdef DEBUGOUT
		fprintf(stderr, "in pkaccept-get another packet\n");
#endif

		PKGETPKT(pk);
		/* pkgetpack will call pkdata which sets a bit in 
		 * imap when a packet arrives
		 */
		SLEEP(&pk->p_pr, PKIPRI);
	}
	pk->p_imap = 0;
	UNLOCK;


	/*
	 * determine input window in m.
	 */
	t = (~(-1<<(int)(pk->p_rwindow))) <<x;
	m = t;
	m |= t>>8;
#ifdef DEBUGOUT
	fprintf(stderr,"pkaccept,m=%o,t=%o\n",(int)m,(int)t);
#endif


	/*
	 * mark newly accepted input buffers
	 */
	for(x=0; x<8; x++) {

		if ((imask & mask[x]) == 0)
			continue; /* nothing has arrived in buffer x */

		if (((cntl=pk->p_is[x])&0200)==0) {
			/* if bad buffer then release space allocated to it */
			bad++;
free:
#ifdef DEBUGOUT
			fprintf(stderr,"read bad packet?\n");
#endif
			bp = (char **)pk->p_ib[x];
			LOCK;
			*bp = (char *)pk->p_ipool;
			pk->p_ipool = bp;
			pk->p_is[x] = 0;
			UNLOCK;
			continue;
		}

		pk->p_is[x] = ~(B_COPY+B_MARK);
		sum = (unsigned)chksum(pk->p_ib[x], pk->p_rsize) ^ (unsigned)(cntl&0377);
		sum += pk->p_isum[x];
		if (sum == CHECK) {
			seq = (cntl>>3) & MOD8; /* seq# of packet */
			if (m & mask[seq]) {
				if (pk->p_is[seq] & (B_COPY | B_MARK)) {
				dup:
#ifdef DEBUGOUT
					fprintf(stderr,"read in a duplicate packet\n");
#endif
					pk->p_msg |= M_RR;
					skip++;
					goto free;
				}
				if (x != seq) {
#ifdef DEBUGOUT
				fprintf(stderr,"seq number is = %d\n", seq);
#endif
				/* switch to current buffer? */
					LOCK;
					p = pk->p_ib[x];
					pk->p_ib[x] = pk->p_ib[seq];
					pk->p_is[x] = pk->p_is[seq];
					pk->p_ib[seq] = p;
					UNLOCK;
				}
				/* indicate buffer in use ? */
				pk->p_is[seq] = B_MARK;
				accept++;  /* inc # of accepted buffers */
				cc = 0;
				if (cntl&B_SHORT) {
				/* data fits in one buffer */
#ifdef DEBUGOUT
					fprintf(stderr,"short packet received\n");
#endif
					pk->p_is[seq] = B_MARK+B_SHORT;
					p = pk->p_ib[seq];
					cc = (unsigned)*p++ & 0377;
					if (cc & 0200) {
						cc &= 0177;
						cc |= *p << 7;
					}
				}
				pk->p_isum[seq] = pk->p_rsize - cc;
			} else {
				goto dup;
			}
		} else {
			bad++;
			/* check sum did not check out */
			goto free;
		}
	}

	/*
	 * scan window again turning marked buffers into
	 * COPY buffers and looking for missing sequence
	 * numbers.
	 */
	accept = 0;
	for(x=next[pk->p_pr],t=h= -1; m & mask[x]; x = next[x]) {
		if (pk->p_is[x] & B_MARK)
			pk->p_is[x] |= B_COPY;
	/*  hole code 
		if (pk->p_is[x] & B_COPY) {
			if (h<0 && t>=0)
				h = x;
		} else {
			if (t<0)
				t = x;
		}
	*/
		if (pk->p_is[x] & B_COPY) {
			if (t >= 0) {
				bp = (char **)pk->p_ib[x];
				LOCK;
				*bp = (char *)pk->p_ipool;
				pk->p_ipool = bp;
				pk->p_is[x] = 0;
				UNLOCK;
#ifdef DEBUGOUT
				fprintf(stderr,"pkaccept, skip=%d\n",skip);
#endif
				skip++;
			} else 
				accept++;
		} else {
			if (t<0)
				t = x;
		}
	}

	if (bad) {
		pk->p_msg |= M_RJ;
	}

	if (skip) {
		pk->p_msg |= M_RR;
	}

	pk->p_rcount = accept;
	return(accept);
}

/* pkread() is the high level input routine of a potential
 * line discipline.  pkaccept is called to read in data off
 * the network.  pkaccept should "wakeup" pkread when
 * all of the data is read in or there is no more buffer 
 * space. Pkread will move the data into user space and
 * return to high level routine that evoked it. The
 * higher level routine will have to reevoke pkread
 * if there is more data to read in.
 * pkread() returns the number characters read in.
 */

pkread(S)
SDEF;  /* struct pack *ipk, char *ibuf, int icount  */
{
register struct pack *pk;
register x,s;
int is,cc,xfr,count;
char *cp, **bp;

	pk = PADDR;
	xfr = 0;
	count = 0;
 	/* read data off the line */
	while (pkaccept(pk)==0);


	/* move data to user space and free up buffers */

	while (UCOUNT) { /* UCOUNT==icount */

		x = next[pk->p_pr];
		is = pk->p_is[x];

		if (is & B_COPY) {
			cc = MIN(pk->p_isum[x], UCOUNT);
			if (cc==0 && xfr) {
				break;
			}
			if (is & B_RESID)
				cp = pk->p_rptr;
			else {
				cp = pk->p_ib[x];
				if (is & B_SHORT) {
					if (*cp++ & 0200)
						cp++;
				}
			}
			IOMOVE(cp,cc,B_READ);
			count += cc;
			xfr++;
			pk->p_isum[x] -= cc;
			if (pk->p_isum[x] == 0) {
			/* free up buffers */
#ifdef DEBUGOUT
				fprintf(stderr,"pkread, packet read, x=%d\n",x);
#endif
				pk->p_pr = x;
				bp = (char **)pk->p_ib[x];
				LOCK;
				*bp = (char *)pk->p_ipool;
				pk->p_ipool = bp;
				pk->p_is[x] = 0;
				pk->p_rcount--;
				UNLOCK;
				pk->p_msg |= M_RR;
			} else {
				pk->p_rptr = cp+cc;
				pk->p_is[x] |= B_RESID;
			}
			if (cc==0)
				break;
		} else
			break;
	}
	pkoutput(pk);  /* xmit acknowledge of data to sender */
	return(count);
}



/* transmit as much data as possible i.e until exceed window
 * return with count of bytes sent.  
 * checksum data in buffers
 */

pkwrite(S)
SDEF;  /* struct pack *ipk, char *ibuf, int icount */
{
register struct pack *pk;
register x;
int partial;
caddr_t cp;
int cc, s, fc, count;

	pk = PADDR;
	if (pk->p_state&DOWN || !pk->p_state&LIVE) {
	/* connection is down */
		SETERROR;
		return(-1);
	}

	count = UCOUNT;
	do {
		LOCK;  /* null at user level */
		while (pk->p_xcount>=pk->p_swindow)  {

			/* out of buffer space */
#ifdef DEBUGOUT
			fprintf(stderr,"pkwrite, no more transmit buffers?\n");
#endif
			/* try to evoke ack of previously sent packets? */

			pkoutput(pk);

			/* wait for an ack to free up buffers */

			PKGETPKT(pk);
			SLEEP(&pk->p_ps,PKOPRI);
		}
		/* get seq# of next packet buffer to xmit */
		x = next[pk->p_pscopy];
		while (pk->p_os[x]!=B_NULL)  { /* buffer in use -wait for ack */
			PKGETPKT(pk);
			SLEEP(&pk->p_ps,PKOPRI);
		}
		/* reserve output buffer */
		pk->p_os[x] = B_MARK;
		pk->p_pscopy = x;
		/* increment # of active transmit buffers */
		pk->p_xcount++;
		UNLOCK;  /* null at user level */

		cp = pk->p_ob[x] = GETEPACK;  /* allocate space for buffer */
		partial = 0;
		if ((int)UCOUNT < pk->p_xsize) {
		/* data can fit into one buffer */
			cc = UCOUNT;
			fc = pk->p_xsize - cc;
			*cp = fc&0177;
			if (fc > 127) {
				*cp++ |= 0200;
				*cp++ = fc>>7;
			} else
				cp++;
			partial = B_SHORT;
#ifdef DEBUGOUT
			fprintf(stderr,"pkwrite, a short message\n");
#endif
		} else {
			/* need more than one buffer to xmit data */
			cc = pk->p_xsize;
#ifdef DEBUGOUT
			fprintf(stderr,"pkwrite, a long message\n");
#endif
		}
		/* get data from user area */
		IOMOVE(cp,cc,B_WRITE);
 		/* checksum data */
		pk->p_osum[x] = chksum(pk->p_ob[x], pk->p_xsize);
		pk->p_os[x] = B_READY+partial;
		pkoutput(pk);
	} while (UCOUNT);

	return(count);
}

/* free buffer space for data that has been acknowledged */
pksack(pk)
register struct pack *pk;
{
register x, i;

	i = 0;
	for(x=pk->p_ps; x!=pk->p_rpr; ) {
		x = next[x];
		if (pk->p_os[x]&B_SENT) {
#ifdef DEBUGOUT
			fprintf(stderr,"pksack, release buffer space\n");
#endif
			i++;
			pk->p_os[x] = B_NULL;
			pk->p_state &= ~WAITO;
			pk->p_xcount--;
			FREEPACK(pk->p_ob[x], pk->p_bits);
			pk->p_ps = x;
			WAKEUP(&pk->p_ps);
		}
	}
	return(i);
}



/* determine nature of data to be sent and tranmsit.
 * interpret control messages and send required data
 */


pkoutput(pk)
register struct pack *pk;
{
register x,rx;
int s;
char bstate;
int i;
SDEF;
int flag;

	flag = 0;
	ISYSTEM;
	LOCK;
	/* if line is busy don't use.  - can not happen though */
	if (pk->p_obusy++ || OBUSY) {
		pk->p_obusy--;
		UNLOCK;
		return;
	}
	UNLOCK;


	/*
	 * find seq number and buffer state
	 * of next output packet
	 */
	if (pk->p_state&RXMIT)  {
		/* if retransmitting then resend the next
		 * packet expected by the receiver.
		 * pk->p_rpr is the last packet received by the receiver.
		 */
		pk->p_nxtps = next[pk->p_rpr];
		flag++; /* not used anywhere */
	}
	x = pk->p_nxtps;   /* seq# of next buffer to transmit */
	bstate = pk->p_os[x]; /* output status of latter buffer */


	/*
	 * Send control packet if indicated
	 */
	if (pk->p_msg) {
		if (pk->p_msg & ~M_RR || !(bstate&B_READY) ) {
			x = pk->p_msg;
			for(i=0; i<8; i++) 
				if (x&1)
					break; else
				x >>= 1;
			x = i;
			x <<= 3;
			switch(i) {
			case CLOSE:
#ifdef DEBUGOUT
				fprintf(stderr, "pkoutput: send CLOSE\n");
#endif
				break;
			case RJ:  /* reject message */
#ifdef DEBUGOUT
				fprintf(stderr, "pkoutput: send RJ: \n");
#endif
			case RR:  /* dup or positive ack or timout*/
#ifdef DEBUGOUT
				fprintf(stderr, "pkoutput: send RR: \n");
#endif
				/* send seq# of last packet received */
				x += pk->p_pr;
				break;
			case SRJ:
#ifdef DEBUGOUT
				fprintf(stderr, "pkoutput: send SRJ: \n");
#endif
				break;
			case INITB:  /* send receive packet size */
				x += pksize(pk->p_rsize);
#ifdef DEBUGOUT
				fprintf(stderr, "pkoutput: send INITB: x=%o\n",x);
#endif
				break;
			case INITC:  /* send receive window size */
				     /* used as positive ack 	*/
				x += pk->p_rwindow;
#ifdef DEBUGOUT
				fprintf(stderr, "pkoutput: send INITC: x=%o\n",x);
#endif
				break;
			case INITA:   /* send receive window size */
				x += pk->p_rwindow;
#ifdef DEBUGOUT
				fprintf(stderr, "pkoutput: send INITA: x=%o\n",x);
#endif
				break;
			}
			pk->p_msg &= ~mask[i]; /* erase prev. control message */
#ifdef DEBUGOUT
			fprintf(stderr, "pkoutput: pk->p_msg=%o\n", pk->p_msg);
#endif
			pkxstart(pk, x, -1); /* start output */
			goto out;
		}
	}


	/*
	 * Don't send data packets if line is marked dead.
	 */
	if (pk->p_state&DOWN) {
		WAKEUP(&pk->p_ps);
		goto out;
	}
	/*
	 * Start transmission (or retransmission) of data packets.
	 */
	if (bstate & (B_READY|B_SENT)) {
		char seq;

		Totalpackets++;
		bstate |= B_SENT;
		seq = x;
		pk->p_nxtps = next[x]; /* next buffer for output */

		/* x=  |short/long |seq# of packet|last recv. #|  */
		x = 0200+pk->p_pr+(seq<<3);
		if (bstate & B_SHORT)  /* complete message in one packet */
			x |= 0100;
#ifdef DEBUGOUT
		fprintf(stderr,"pkoutput, transmit another packet\n");
#endif
		pkxstart(pk, x, seq);
		pk->p_os[seq] = bstate;  /* set initialy by pkwrite */
		pk->p_state &= ~RXMIT;
		pk->p_nout++;  /* total number of packets transmitted */
		goto out;
	}
	/*
	 * enable timeout if there's nothing to send
	 * and transmission buffers are languishing
	 * - does not appear to be used anywhere though
	 */
#ifdef DEBUGOUT
	fprintf(stderr,"pkoutput: nothing to transmit, pk->p_xcount=%d\n", pk->p_xcount);
#endif
	if (pk->p_xcount) {
		pk->p_timer = 2;
		pk->p_state |= WAITO;  /* WAITO not referenced anywhere else */
				       /*  at user level */
	} else

		pk->p_state &= ~WAITO;
	WAKEUP(&pk->p_ps);
out:
	pk->p_obusy = 0;
}


/*
 * shut down line by
 *	ignoring new input
 *	letting output drain
 *	releasing space and turning off line discipline
 */
pkclose(S)
SDEF;
{
register struct pack *pk;
register i,s,rbits;
char **bp;
int rcheck;
char *p;


	pk = PADDR;
	pk->p_state |= DRAINO;


	/*
	 * try to flush output
	 */
	i = 0;
	LOCK;
	pk->p_timer = 2;
#ifdef DEBUGOUT
	fprintf(stderr,"pkclose\n");
#endif
	while (pk->p_xcount && pk->p_state&LIVE) {
		if (pk->p_state&(RCLOSE+DOWN) || ++i > 2)
			break;
		pkoutput(pk);
		SLEEP(&pk->p_ps,PKOPRI);
	}
	pk->p_timer = 0;
	pk->p_state |= DOWN;
	UNLOCK;


	/*
	 * try to exchange CLOSE messages
	 */
	i = 0;
	while ((pk->p_state&RCLOSE)==0 && i<2) {
		pk->p_msg = M_CLOSE;
		pk->p_timer = 2;
		pkoutput(pk);
		SLEEP(&pk->p_ps, PKOPRI);
		i++;
	}


	for(i=0;i<NPLINES;i++)
		if (pklines[i]==pk)  {
			pklines[i] = NULL;
		}
	TURNOFF;


	/*
	 * free space
	 */
	rbits = DTOM(pk->p_rsize);
	rcheck = 0;
	for (i=0;i<8;i++) {
		if (pk->p_os[i]!=B_NULL) {
			FREEPACK(pk->p_ob[i],pk->p_bits);
			pk->p_xcount--;
		}
		if (pk->p_is[i]!=B_NULL)  {
			FREEPACK(pk->p_ib[i],rbits);
			rcheck++;
		}
	}
	LOCK;
	while (pk->p_ipool != NULL) {
		bp = pk->p_ipool;
		pk->p_ipool = (char **)*bp;
		rcheck++;
		FREEPACK(bp, rbits);
	}
	UNLOCK;
	if (rcheck  != pk->p_rwindow) {
		fprintf(stderr, "r short %d want %d\n",rcheck,pk->p_rwindow);
		fprintf(stderr, "rcount = %d\n",pk->p_rcount);
		fprintf(stderr, "xcount = %d\n",pk->p_xcount);
	}
	FREEPACK((caddr_t)pk, npbits);
}



/* reset pointers to buffer pool */
pkreset(pk)
register struct pack *pk;
{

	pk->p_ps = pk->p_pr =  pk->p_rpr = 0;
	pk->p_nxtps = 1;
}

#ifdef ULTRIX
/*
 * Vax-specific checksum generator, same checksum as portable version
 * but faster.  NOTE: ASSUMES n < 65535
 * 14 June 1983
 * Chris Torek
 * University of Maryland
 */
chksum (s, n)
register char *s;
register n;
{
	register sum, x;
	register unsigned t;

	sum = -1;
	x = 0;
	do {
		/* Rotate left, copying bit 15 to bit 0 */
		if (sum & 0x8000)
			sum <<= 1, sum++;
		else
			sum <<= 1;
		sum &= 0xffff;		/* Switch to unsigned short */
		t = sum;
		sum = (sum + (*s++ & 0377)) & 0xffff;
		x += sum ^ n;
		if ((unsigned)sum <= t)	/* (unsigned) not necessary */
			sum ^= x;	/* but doesn't hurt */
	} while (--n > 0);

	return (int) (short) sum;
}



#else

/*  old check sum routine */
chksum(s,n)
register char *s;
register n;
{
	register unsigned sum, t;
	register x;

	sum = -1;
	x = 0;

	do {
		if (sum&0x8000) {
			sum <<= 1;
			sum++;
		} else
			sum <<= 1;
		t = sum;
		sum += (unsigned)*s++ & 0377;
		x += sum^n;
		if ((sum&0xffff) <= (t&0xffff)) {
			sum ^= x;
		}
	} while (--n > 0);

	return(sum & 0xffff);
}
#endif


pkline(pk)
register struct pack *pk;
{
register i;
	for(i=0;i<NPLINES;i++) {
		if (pklines[i]==pk)
			return(i);
	}
	return(-i);
}

pkzero(s,n)
register char *s;
register n;
{
	while (n--)
		*s++ = 0;
}

pksize(n)
register n;
{
register k;

	n >>= 5;
	for(k=0; n >>= 1; k++);
	return(k);
}
