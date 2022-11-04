
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * SCCSID: @(#)in_cksum.c	3.0	4/21/86
 *	Based on in_cksum.c 1.12 82/06/20
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>

/*
 * Checksum routine for Internet Protocol family headers.
 * This routine is very heavily used in the network
 * code and should be rewritten for each CPU to be as fast as possible.
 */

#if pdp11
#ifdef	CKSUMDEBUG
int     in_ckodd;                       /* number of calls on odd start add */
int     in_ckprint = 0;                 /* print sums */
#endif	CKSUMDEBUG

in_cksum(m, len)
	struct mbuf *m;
	int len;
{
	register char *w;               /* known to be r4 */
	register u_int mlen = 0;        /* known to be r3 */
	register u_int sum = 0;         /* known to be r2 */
	u_int plen = 0;

	MAPSAVE();
#ifdef	CKSUMDEBUG
	if (in_ckprint) printf("ck m%o l%o",m,len);
#endif	CKSUMDEBUG
	for (;;) {
		/*
		 * Each trip around loop adds in
		 * words from one mbuf segment.
		 */
		w = mtod(m, u_char *);
		if (plen & 01) {
			/*
			 * The last segment was an odd length, add the high
			 * order byte into the checksum.
			 */
/* BEGIN SECTION AFFECTED BY ED SCRIPT. BE CAREFULL WHEN CHANGING! */
			sum = in_ckadd(sum,(*w++ << 8));
/* END SECTION AFFECTED BY ED SCRIPT. */
			mlen = m->m_len - 1;
			len--;
		} else
			mlen = m->m_len;
		m = m->m_next;
		if (len < mlen)
			mlen = len;
		len -= mlen;
		plen = mlen;
#ifdef	CKSUMDEBUG
		if (((int)w&01) && in_ckodd++ == 0)
			printf("cksum: odd\n");
#endif	CKSUMDEBUG
/* BEGIN SECTION AFFECTED BY ED SCRIPT. BE CAREFULL WHEN CHANGING! */
		if (mlen > 0)
			in_ckbuf();     /* arguments already in registers */
/* END SECTION AFFECTED BY ED SCRIPT. */
		if (len == 0)
			break;
		/*
		 * Locate the next block with some data.
		 */
		for (;;) {
			if (m == 0) {
				printf("cksum: out of data\n");
				goto done;
			}
			if (m->m_len)
				break;
			m = m->m_next;
		}
	}
done:
	MAPREST();
#ifdef	CKSUMDEBUG
	if (in_ckprint) printf(" s%o\n",~sum);
#endif	CKSUMDEBUG
	return (~sum);
}
#endif
