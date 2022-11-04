
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)pk1.c	3.0	4/22/86";

/*
 * packet driver support routines
 *
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
#include <setjmp.h>
#include <signal.h>
#include <errno.h>

#define PKMAXSTMSG 40
int Errorrate;
int Connodata = 0;
int Ntimeout = 0;
extern int errno;
#define CONNODATA 10  /* max control messages that can arrive between data */
#define NTIMEOUT 50

struct pack *pklines[NPLINES];

/*
 * start initial synchronization.
 */

struct pack *
pkopen(ifn, ofn)
int ifn, ofn;
{
	struct pack *pk;
	char **bp;
	int i;

/* increment number of active lines.  only valid if in kernel space
 * can only have one active line at a time.
 * Furthermore it is not decremented anywhere. If one uucico 
 * daemon tries to contact more than NPLINES sites then
 * this codes will return(NULL)
 */
	if (++pkactive >= NPLINES)
		return(NULL);
	/* initialize packet control structure */
	if ((pk = (struct pack *) malloc(sizeof (struct pack))) == NULL)
		return(NULL);
	pkzero((caddr_t) pk, sizeof (struct pack));
	/* i/o pointers */
	pk->p_ifn = ifn;
	pk->p_ofn = ofn;
	pk->p_xsize = pk->p_rsize = PACKSIZE;
	pk->p_rwindow = pk->p_swindow = WINDOWS;
	/*  allocate input windows 
	 *  and build pool of input buffers 
	 */
	for (i = 0; i < pk->p_rwindow; i++) {
		if ((bp = (char **) GETEPACK) == NULL)
			break;
		*bp = (char *) pk->p_ipool;
		pk->p_ipool = bp;
	}
	if (i == 0)
		return(NULL);
	pk->p_rwindow = i;

	/* start synchronization */
#ifdef DEBUGOUT
	fprintf(stderr,"pkopen, start synchronization\n");
#endif
	pk->p_msg = pk->p_rmsg = M_INITA;
	/* get first available line */
	for (i = 0; i < NPLINES; i++) {
		if (pklines[i] == NULL) {
			pklines[i] = pk;
			break;
		}
	}
	if (i >= NPLINES)
		return(NULL);
	/* start sync phase */
	pkoutput(pk);

/* loop until line comes on line or have exceeded the max allowable number
 * of startup messages
 */
	for (i = 0; i < PKMAXSTMSG; i++) {
		PKGETPKT(pk);
		if ((pk->p_state & LIVE) != 0)
			break;
	}
	if (i >= PKMAXSTMSG)
		return(NULL);

	pkreset(pk);
	return(pk);
}


/*
 * input framing and block checking.
 * frame layout for most devices is:
 *	
 *	S|K|X|Y|C|Z|  ... data ... |
 *
 *	where 	S	== initial synch byte
 *		K	== encoded frame size (indexes pksizes[])
 *		X, Y	== block check bytes
 *		C	== control byte
 *		Z	== XOR of header (K^X^Y^C)
 *		data	== 0 or more data bytes
 *
 */

int pksizes[] = {
	1, 32, 64, 128, 256, 512, 1024, 2048, 4096, 1
};

#define GETRIES 5
/*
 * Pseudo-dma byte collection.
 */

pkgetpack(ipk)
struct pack *ipk;
{
	int ret, k, tries;
	register char *p;
	struct pack *pk;
	struct header *h;
	unsigned short sum;
	int ifn;
	char **bp;
	char hdchk;

	pk = PADDR;

	if ((pk->p_state & DOWN) ||
	  Connodata > CONNODATA /* || Ntimeout > NTIMEOUT */)
		/* have exceed the max number of startup messages between
		 * data messages or line has been shut down
 		 */
		pkfail();
	ifn = pk->p_ifn;

	/* find HEADER */
	for (tries = 0; tries < GETRIES; ) {
		p = (caddr_t) &pk->p_ihbuf;
		if ((ret = pkcget(ifn, p, 1)) < 0) {
			/* set up retransmit or REJ */
			tries++;
			/* try to force rexmit */
			pk->p_msg |= pk->p_rmsg;
#ifdef DEBUGOUT
			fprintf(stderr,"pkgetpack-rxmit, pk->p_msg=%d\n",pk->p_msg);
#endif
			if (pk->p_msg == 0) /* see if data or control */
				/* will cause pkoutput to send 
				 * last received packet sequence number.
				 * The remote site will use this number
				 * to determine which packets it needs
				 * to send (or resend)
				 */
				pk->p_msg |= M_RR;
			/* else resend previous startup control message */
			if ((pk->p_state & LIVE) == LIVE)
				pk->p_state |= RXMIT;
			
			pkoutput(pk);
			continue;
		}
		if (*p != SYN) /* wait for SYN character - indicates */
				/* start of packet */
			continue;
		p++;
		/* get rest of header */	
		ret = pkcget(ifn, p, HDRSIZ - 1);
		if (ret == -1)
			continue;
		break;
	}
	if (tries >= GETRIES) {
		PKDEBUG(4, "tries = %d\n", tries);
		pkfail();
	}

	Connodata++;
	h = (struct header * ) &pk->p_ihbuf;
	p = (caddr_t) h;
	hdchk = p[1] ^ p[2] ^ p[3] ^ p[4];
	p += 2;
	sum = (unsigned) *p++ & 0377;
	sum |= (unsigned) *p << 8;
	h->sum = sum;
	PKDEBUG(5, "pkgetpack,rec h->cntl %o\n", (unsigned) h->cntl);
	k = h->ksize;
	if (hdchk != h->ccntl) {
		/* bad header */
		PKDEBUG(5, "pkgetpack,bad header %o,", hdchk);
		PKDEBUG(5, "pkgetpack,h->ccntl %o\n", h->ccntl);
		return;
	}
	if (k == 9) {  /* check for control message */
		if (((h->sum + h->cntl) & 0xffff) == CHECK) {
			pkcntl(h->cntl, pk);
			PKDEBUG(5, "pkgetpack,control message header ok, state= %o\n", pk->p_state);
		}
		else {
			/*  bad header */
			PKDEBUG(5, "pkgetpack,bad header (k==9) %o\n", h->cntl);
			pk->p_state |= BADFRAME;
		}
		return;
	}
	if (k && pksizes[k] == pk->p_rsize) { /* see if received all of data */
#ifdef DEBUGOUT
		fprintf(stderr, "pkgetpack, received exactly one buffers worth of data\n");
#endif
		pk->p_rpr = h->cntl & MOD8;
		/* release buffer space */
		pksack(pk);
		Connodata = 0;
		bp = pk->p_ipool;
		pk->p_ipool = (char **) *bp;
		if (bp == NULL) {
			PKDEBUG(5, "pkgetpack, no more input buffers %s\n", "");
		return;
		}
	}
	else {
#ifdef DEBUGOUT
		fprintf(stderr, "pkgetpack, return\n");
#endif
		return;
	}
#ifdef DEBUGOUT
	fprintf(stderr,"pkgetpack, read in remaining data, pk->p_rsize=%d\n",pk->p_rsize);
#endif
	ret = pkcget(pk->p_ifn, (char *) bp, pk->p_rsize);
	if (ret == 0)
		pkdata(h->cntl, h->sum, pk, (char **) bp);
	return;
}

/* pack data into an available input packet buffer */

pkdata(c, sum, pk, bp)
char c;
unsigned short sum;
register struct pack *pk;
char **bp;
{
register x;
int t;
char m;

	if (pk->p_state & DRAINO || !(pk->p_state & LIVE)) {
		/* link is down - drain remaining output data?
		 * return buffer to pool and return
		 */
		pk->p_msg |= pk->p_rmsg;
		pkoutput(pk);
		goto drop;
	}
	t = next[pk->p_pr];  /* get next input buffer */
	for(x=pk->p_pr; x!=t; x = (x-1)&7) {
		if (pk->p_is[x] == 0)
			goto slot;
	}
drop:
	*bp = (char *)pk->p_ipool;
	pk->p_ipool = bp;
	return;

slot:
	m = mask[x];    /* use buffer indicated by mask[x] */
	pk->p_imap |= m;  /* indicate in p_imap that data is in this buffer */
	pk->p_is[x] = c;  /* status of buffer */
	pk->p_isum[x] = sum;  /* checksum associated with buffer */
	pk->p_ib[x] = (char *)bp;  /* next input buffer in list */
	return;
}



/*
 * setup input transfers
 */
pkrstart(pk)
{}

#define PKMAXBUF 128
/*
 * Start transmission on output device associated with pk.
 * For asynch devices (t_line==1) framing is
 * imposed.  For devices with framing and crc
 * in the driver (t_line==2) the transfer is
 * passed on to the driver.
 */
pkxstart(pk, cntl, x)
struct pack *pk;
char cntl;
register x;
{
	register char *p;
	int ret;
	short checkword;
	char hdchk;

	p = (caddr_t) &pk->p_ohbuf;
	*p++ = SYN;
	if (x < 0) {
		/* this is a control message */
		*p++ = hdchk = 9;
		checkword = cntl;
	}
	else {
		/* sending a data packet */
		*p++ = hdchk = pk->p_lpsize;
		checkword = pk->p_osum[x] ^ (unsigned)(cntl & 0377);
	}
	/* set up header */
	checkword = CHECK - checkword;
	*p = checkword;
	hdchk ^= *p++;
	*p = checkword>>8;
	hdchk ^= *p++;
	*p = cntl;
	hdchk ^= *p++;
	*p = hdchk;
	 /*  writes  */
	PKDEBUG(5, "pkxstart, send cntl=%o\n", (unsigned) cntl);
	p = (caddr_t) & pk->p_ohbuf;
	if (x < 0) { /* send control message */
#ifdef PROTODEBUG
		GENERROR(p, HDRSIZ);
#endif
		errno=0;
		ret = write(pk->p_ofn, p, HDRSIZ);
		PKASSERT(ret == HDRSIZ, "PKXSTART ret", "", errno);
	}
	else {  /* send packet data */
		char buf[PKMAXBUF + HDRSIZ], *b;
		int i;
		for (i = 0, b = buf; i < HDRSIZ; i++) 
			*b++ = *p++;
		for (i = 0, p = pk->p_ob[x]; i < pk->p_xsize; i++)
			*b++ = *p++;
#ifdef PROTODEBUG
		GENERROR(buf, pk->p_xsize + HDRSIZ);
#endif
		errno = 0;
		ret = write(pk->p_ofn, buf, pk->p_xsize + HDRSIZ);
		PKASSERT(ret == pk->p_xsize + HDRSIZ,
		  "PKXSTART ret", "", errno);
		Connodata = 0; /* can reset because a data packet was sent */
	}
	if (pk->p_msg) /* send additional control messages */
		pkoutput(pk);
	return;
}


pkmove(p1, p2, count, flag)
char *p1, *p2;
int count, flag;
{
	char *s, *d;
	int i;
	if (flag == B_WRITE) {
		s = p2;
		d = p1;
	}
	else {
		s = p1;
		d = p2;
	}
	for (i = 0; i < count; i++)
		*d++ = *s++;
	return;
}


/***
 *	pkcget(fn, b, n)	get n characters from input
 *	char *b;		- buffer for characters
 *	int fn;			- file descriptor
 *	int n;			- requested number of characters
 *
 *	return codes:
 *		n - number of characters returned
 *		0 - end of file
 */

jmp_buf Getjbuf;
cgalarm() { longjmp(Getjbuf, 1); }

pkcget(fn, b, n)
int fn, n;
char *b;
{
	int nchars, ret;
	extern int linebaud;

	if (setjmp(Getjbuf)) {
		Ntimeout++;
		PKDEBUG(4, "alarm %d\n", Ntimeout);
		return(-1);
	}
	signal(SIGALRM, cgalarm);

	alarm((unsigned)(n < HDRSIZ ? 25 : 30));
	for (nchars = 0; nchars < n; ) {
		errno = 0;
		ret = read(fn, b, n - nchars);
#ifdef DEBUGOUT
		if (ret != n - nchars)
			fprintf(stderr,"pkcget, ret=%d, n-nchars=%d\n",ret,n-nchars);
#endif
		if (ret == 0) {
			alarm(0);
			return(-1);
		}
		PKASSERT(ret > 0, "PKCGET READ", "", errno);
		b += ret;
		nchars += ret;
		if (nchars < n)
			if ((linebaud > 0) && (linebaud < 4800))
				sleep(1);
			else
				nap(1);
	}
	alarm(0);
	return(0);
}


#ifdef PROTODEBUG
generror(p, s)
char *p;
int s;
{
	int r;
	if (Errorrate != 0 && (rand() % Errorrate) == 0) {
		r = rand() % s;
fprintf(stderr, "gen err at %o, (%o), ", r, (unsigned) *(p + r));
		*(p + r) += 1;
	}
	return;
}


#endif
