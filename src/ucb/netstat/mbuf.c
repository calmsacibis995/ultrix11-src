
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
/*
 * Based on static char sccsid[] = "@(#)mbuf.c	5.1 (Berkeley) 6/4/85";
 */
static char *Sccsid = "@(#)mbuf.c	3.0	(ULTRIX-11)	4/22/86";
#endif not lint

#include <stdio.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#define	YES	1
typedef int bool;

struct	mbstat mbstat;
extern	int kmem;

static struct mbtypes {
	int	mt_type;
	char	*mt_name;
} mbtypes[] = {
	{ MT_DATA,	"data" },
	{ MT_HEADER,	"packet headers" },
	{ MT_SOCKET,	"socket structures" },
	{ MT_PCB,	"protocol control blocks" },
	{ MT_RTABLE,	"routing table entries" },
	{ MT_HTABLE,	"IMP host table entries" },
	{ MT_ATABLE,	"address resolution tables" },
	{ MT_FTABLE,	"fragment reassembly queue headers" },
	{ MT_SONAME,	"socket names and addresses" },
	{ MT_ZOMBIE,	"zombie process information" },
	{ MT_SOOPTS,	"socket options" },
	{ MT_RIGHTS,	"access rights" },
	{ MT_IFADDR,	"interface addresses" }, 
	{ 0, 0 }
};

#ifndef	pdp11
int nmbtypes = sizeof(mbstat.m_mtypes) / sizeof(short);
bool seen[256];			/* "have we seen this type yet?" */
#endif	pdp11

/*
 * Print mbuf statistics.
 */
#ifndef pdp11
mbpr(mbaddr)
	off_t mbaddr;
{
	register int totmem, totfree, totmbufs;
	register int i;
	register struct mbtypes *mp;
#else	pdp11
mbpr(mbaddr, mbsizeaddr, allocstart, alloct)
int mbaddr, mbsizeaddr;
unsigned allocstart, alloct;
{
	register int totfree, totmbufs;
	register struct mbtypes *mp;
	register int i;
	unsigned mbsize, nmbufs, allocsize;
#endif pdp11

#ifndef pdp11
	if (nmbtypes != 256) {
		fprintf(stderr, "unexpected change to mbstat; check source\n");
		return;
	}
#endif pdp11
	if (mbaddr == 0) {
		printf("mbstat: symbol not in namelist\n");
		return;
	}
#ifdef	pdp11
	klseek(kmem, alloct, 0);
	if (read(kmem, &allocsize, sizeof (allocsize)) != sizeof (allocsize)) {
		printf("alloct: bad read\n");
		return;
	}
	allocsize -= (allocstart + 2);
#endif	pdp11
	klseek(kmem, mbaddr, 0);
	if (read(kmem, &mbstat, sizeof (mbstat)) != sizeof (mbstat)) {
		printf("mbstat: bad read\n");
		return;
	}
#ifdef	pdp11
	klseek(kmem, mbsizeaddr, 0);
	if (read(kmem, &mbsize, sizeof (mbsize)) != sizeof (mbsize)) {
		printf("mbsize: bad read\n");
		return;
	}
	nmbufs = mbsize/MSIZE;
	printf("%d/%d mbufs in use:\n", nmbufs - mbstat.m_mbfree, nmbufs);
	printf("%d mbufs never used\n", mbstat.m_mbufs);
#else pdp11
	printf("%d/%d mbufs in use:\n",
		mbstat.m_mbufs - mbstat.m_mtypes[MT_FREE], mbstat.m_mbufs);
#endif pdp11
#ifdef	pdp11
	totmbufs = nmbufs - mbstat.m_mbfree;	/* can't detect missing mbufs */
#else	pdp11
	totmbufs = 0;
	for (mp = mbtypes; mp->mt_name; mp++)
		if (mbstat.m_mtypes[mp->mt_type]) {
			seen[mp->mt_type] = YES;
			printf("\t%d mbufs allocated to %s\n",
			    mbstat.m_mtypes[mp->mt_type], mp->mt_name);
			totmbufs += mbstat.m_mtypes[mp->mt_type];
		}
	seen[MT_FREE] = YES;
	for (i = 0; i < nmbtypes; i++)
		if (!seen[i] && mbstat.m_mtypes[i]) {
			printf("\t%d mbufs allocated to <mbuf type %d>\n",
			    mbstat.m_mtypes[i], i);
			totmbufs += mbstat.m_mtypes[i];
		}
#endif	pdp11
#ifndef pdp11
	if (totmbufs != mbstat.m_mbufs - mbstat.m_mtypes[MT_FREE])
		printf("*** %d mbufs missing ***\n",
			(mbstat.m_mbufs - mbstat.m_mtypes[MT_FREE]) - totmbufs);
	printf("%d/%d mapped pages in use\n",
		mbstat.m_clusters - mbstat.m_clfree, mbstat.m_clusters);
	totmem = mbstat.m_mbufs * MSIZE + mbstat.m_clusters * CLBYTES;
	totfree = mbstat.m_mtypes[MT_FREE]*MSIZE + mbstat.m_clfree * CLBYTES;
	printf("%d Kbytes allocated to network (%d%% in use)\n",
		totmem / 1024, (totmem - totfree) * 100 / totmem);
#else pdp11
	/*
	 * Following can't happen (see totmbufs setup code above),
	 * pdp11s don't maintain mbstat.m_mtypes[].
	 * Fred Canter -- 2/2/86
	 */
	if (totmbufs != nmbufs - mbstat.m_mbfree)
		printf("*** %d mbufs missing ***\n",
			(nmbufs - mbstat.m_mbfree) - totmbufs);
	/* end of can't happen */
	printf("%u/%u bytes general allocation in use:\n",
		allocsize - mbstat.m_clfree, allocsize);
	printf("%u bytes never used\n", mbstat.m_clusters);
	totfree = mbstat.m_mbfree;
	printf("%d Kbytes allocated to network (%d%% in use)\n",
		mbsize / 1024, (nmbufs - totfree) * 100 / nmbufs);
#endif pdp11
	printf("%d requests for memory denied\n", mbstat.m_drops);
}
