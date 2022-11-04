
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)subr.c	3.0	4/21/86";
#include "mkconf.h"

/*
 * ovload checks to see if the module will fit
 * into the overlay and then loads it if possible.
 * Returns 0 if it can't be loaded, 1 if it can.
 */

ovload(tp, ovn)
struct ovtab *tp;
{
	register struct ovdes *dp;

	dp = &ovdtab[ovn];
	if (dp->nentry >= 12 || dp->size + tp->mts > 8192)
		return(0);
	dp->omns[dp->nentry++] = tp->mpn;
	dp->size += tp->mts;
	return(1);
}

puke(s)
char **s;
{
	char *c;

	while(c = *s++) {
		printf(c);
		printf("\n");
	}
}

	int mauscount = 0;
struct ptab {
	char	*name;
	int	*varp;
} ptab[] = {
	"nswap",	&nswap,
	"elnb",		&elnb,
	"nbuf",		&nbuf,
	"ninode",	&ninode,
	"nfile",	&nfile,
	"nproc",	&nproc,
	"mapsize",	&mapsize,
	"ncall",	&ncall,
	"ntext",	&ntext,
	"nclist",	&nclist,
	"maxuprc",	&maxuprc,
	"timezone",	&timezone,
	"dstflag",	&dstflag,
	"msgmax",	&msgmax,
	"msgmnb",	&msgmnb,
	"msgtql",	&msgtql,
	"msgssz",	&msgssz,
	"msgseg",	&msgseg,
	"msgmap",	&msgmap,
	"msgmni",	&msgmni,
	"semmap",	&semmap,
	"semmni",	&semmni,
	"semmns",	&semmns,
	"semmnu",	&semmnu,
	"semume",	&semume,
	"semmsl",	&semmsl,
	"semopm",	&semopm,
	"flckrec",	&flckrec,
	"flckfil",	&flckfil,
	"nmaus",	&nmaus,
	"mbufs",	&mbufs,
	"miosize",	&miosize,
	"allocs",	&allocs,
	0
};

struct ptab lptab[] = {
	"swplo",	(int *)&swplo,
	"elsb",		(int *)&elsb,
	0
};

input()
{
	char line[50];
	char	*lp;
	register struct tab *q;
	register struct mscptab *mp;
	register struct ptab *ptp;
	int count, n;
	long num;
	unsigned int onum, csr;
	int cn;
	char keyw[32], dev[32];

	if (fgets(line, 100, stdin) == NULL)
		return(0);
	cn = -1;
	count = -1;
	lp = &line;
	n = 0;
	if((*lp >= '0') && (*lp <= '9')) {
		count = *lp++ - '0';
		n++;
	}
	 sscanf(lp, "%s%s%o%d", keyw, dev, &onum, &cn);
	n += sscanf(lp, "%s%s%ld", keyw, dev, &num);
	if (count == -1 && n>0) {
		count = 1;
		n++;
	}
	if (n<2)
		goto badl;
/*
 * Special case for MSCP disks, because driver
 * supports multiple controllers.
 */
	for(mp=mstab; mp->ms_dcn; mp++)
		if((strlen(keyw) == strlen(mp->ms_dcn)) &&
		   equal(mp->ms_dcn, keyw)) {
			if(mp->ms_cn >= 0) {
			    fprintf(stderr, "\n%s: %s\n", mp->ms_dcn,
			      "already configured, only one allowed!");
			    return(1);
			}
			if((n != 4) || (cn < 0)) {
			    fprintf(stderr, "\n%s: must specify %s\n",
			      mp->ms_dcn, "CSR, VECTOR, # of units, cntlr #!");
			    return(1);
			}
			if(sscanf(dev, "%o", &csr) <= 0)
				goto badl;
			if(csr < 0160010)
				goto badl;
			if((onum < 0120) || (onum > 0774))
				goto badl;
			mp->ms_cn = cn;
			mp->ms_nra = count;
			mp->ms_csr = csr & 0177776;
			mp->ms_vec = onum;
			for(q=table; q->name; q++)
				if(q->csr == MSCPDEV) {
					q->count = 1;	/* say RA configured */
					break;
				}
			return(1);
		}
/*
 * Special case for TS/TK magtape, because driver
 * supports multiple controllers.
 */
	if(equal(keyw, "ts") || equal(keyw, "tk")) {
		if((n != 4) || (cn < 0)) {
			fprintf(stderr, "\n(%s): must specify CSR, VECTOR, # of units!\n", keyw);
			return(1);
		}
		if(equal(keyw,"ts"))
			mp = tstab;
		else
			mp = tktab;
		mp += cn;
		if(sscanf(dev, "%o", &csr) <= 0)
			goto badl;
		if(csr < 0160010)
			goto badl;
		if((onum < 0120) || (onum > 0774))
			goto badl;
		mp->ms_cn = cn;
		mp->ms_csr = csr & 0177776;
		mp->ms_vec = onum;
		for(q=table; q->name; q++) {
			if(equal(q->name,keyw)) {
				q->count = 1;
				break;
			}
		}
		return(1);
	}
	for(q=table; q->name; q++)
	if((strlen(keyw) == strlen(q->name)) && equal(q->name, keyw)) {
		if(q->count < 0) {
			fprintf(stderr, "%s: no more, no less\n", keyw);
			return(1);
		}
		q->count += count;
		if(q->vec < 0300
		  && q->count > 1
		  && ((q->key & NUNIT) == 0)
		  && ((q->key & TAPE) == 0) 
		  && (strncmp(q->name, "if_",3))) {
			q->count = 1;
			fprintf(stderr, "%s: only one\n", keyw);
		}
		if(n > 2) {	/* non standard address/vector specified */
			if(n != 4)	/* MUST be both CSR & VECTOR */
				goto badl;
			if(sscanf(dev, "%o", &csr) <= 0)
				goto badl;
			if(csr < 0160010)
				goto badl;
			if((onum < 0120) || (onum > 0774))
				goto badl;
			q->key |= NSAV;
			q->csr = csr & 0177776;
			q->vec = onum;
			}
		if (!strncmp(q->name, "if_", 3)) {
			if (count > 1) {
				fprintf(stderr, "%s: one per line\n", keyw);
				q->count -= (count - 1);
			}
			strncpy(netattach[nnetattach].name, &q->name[3], 2);
			netattach[nnetattach].csr = q->csr;
			netattach[nnetattach].vec = q->vec;
			if (cn < 0)
				cn = 1;
			if (cn == 1)
				q->codec = "";
			netattach[nnetattach].nvec = cn;
			netattach[nnetattach++].unit = q->count - 1;
		}
		return(1);
	}
	if (equal(keyw, "pty")) {
		if(sscanf(dev,"%ld", &num) <= 0)
			goto badl;
		npty = num;
		return(1);
	}
	for (ptp = ptab; ptp->name; ptp++)
		if (equal(keyw, ptp->name)) {
			if (n<3)
				goto badl;
			if (sscanf(dev, "%ld", &num) <= 0)
				goto badl;
			*(ptp->varp) = num;
			return(1);
		}
	if (equal(keyw,"ulimit")) {
		if(sscanf(dev,"%ld",&num) <= 0)
			goto badl;
		ulimit = num;
		return(1);
	}
	for (ptp = lptab; ptp->name; ptp++)
		if (equal(keyw, ptp->name)) {
			if (n<3)
				goto badl;
			if (sscanf(dev, "%ld", &num) <= 0)
				goto badl;
			*((long *)ptp->varp) = num;
			return(1);
		}
	if (equal(keyw, "nmount")) {
		if(n<3)
			goto badl;
		if(sscanf(dev, "%ld", &num) <= 0)
			goto badl;
		if(num > 16) {
			fprintf(stderr, "\7\7\7nmount > 16, using 16 !\n");
			nmount = 16;
		} else
			nmount = num;
		return(1);
	}
	if (equal(keyw, "ncargs")) {
		if(n<3)
			goto badl;
		if(sscanf(dev, "%ld", &num) <= 0)
			goto badl;
		if(num < 512)
			goto badl;
		ncargs = num;
		return(1);
	}
	if (equal(keyw, "hz")) {
		if(n<3)
			goto badl;
		if(sscanf(dev, "%ld", &num) <= 0)
			goto badl;
/* Ohms 11/12/84 - commented out to allow for strange ac line freq.
		if((num != 60) && (num != 50))
			goto badl;
*/
		hz = num;
		return(1);
	}
	if (equal(keyw, "msgbufs")) {
		if(n<3)
			goto badl;
		if(sscanf(dev, "%ld", &num) <= 0)
			goto badl;
		if(num < 1)
			goto badl;
		msgbufs = num;
		return(1);
	}
	if (equal(keyw, "maxseg")) {
		if(n<3)
			goto badl;
		if(sscanf(dev, "%ld", &num) <= 0)
			goto badl;
		if(num < 2048)
			goto badl;
		maxseg = num;
		return(1);
	}
	if (equal(keyw, "mesg")) {
		mesg++;
		ipc++;
		return(1);
	}
	if (equal(keyw, "sema")) {
		sema++;
		ipc++;
		return(1);
	}
	if (equal(keyw, "flock")) {
		flock++;
		return(1);
	}
	if (equal(keyw, "maus")) {
		maus++;
		return(1);
	}
	if( (equal(keyw, "maus0")) ||
  	    (equal(keyw, "maus1")) ||
	    (equal(keyw, "maus2")) ||
	    (equal(keyw, "maus3")) ||
	    (equal(keyw, "maus4")) ||
	    (equal(keyw, "maus5")) ||
	    (equal(keyw, "maus6")) ||
	    (equal(keyw, "maus7")) ) {
		if(n<3)
			goto badl;
		if(sscanf(dev,"%ld", &num) <= 0)
			goto badl;
		if (mauscount >= nmaus)
			goto badl;
		mausize[mauscount] = num;
		mauscount++;
		return(1);
	}
		
	if (equal(keyw, "shuffle")) {
		shuffle++;
		return(1);
	}

	if (equal(keyw, "network")) {
		network++;
		return(1);
	}
	if (equal(keyw, "kfpsim")) {
		kfpsim++;
		return(1);
	}
	if (equal(keyw, "nomap")) {
		ubmap = 0;
		return(1);
	}
	if (equal(keyw, "generic")) {
		generic = 1;
		return(1);
	}
	if (equal(keyw, "ov")) {
		sid = 0;
		ov++;
		return;
	}
	if (equal(keyw, "nfp")) {
		nfp++;
		return(1);
	}
	if (equal(keyw, "dump")) {
		if (n != 3)
			goto badl;
		if(dump) {
			fprintf(stderr, "\ndump: only one!\n");
			return(1);
		}
		if (equal(dev, "ht"))
			dump = HT;
		else if(equal(dev, "tm"))
			dump = TM;
		else if(equal(dev, "ts"))
			dump = TS;
		else if(equal(dev, "rl"))
			dump = RL;
		else if(equal(dev, "ra"))
			dump = RA;
		else if(equal(dev, "rc"))
			dump = RC;
		else if(equal(dev, "rq"))
			dump = RD;
		else if(equal(dev, "rx"))
			dump = RX;
		else if(equal(dev, "hk"))
			dump = HK;
		else if(equal(dev, "hp"))
			dump = HP;
		else if(equal(dev, "rp"))
			dump = RP;
		else if(equal(dev, "tk"))
			dump = TK;
		else {
			dump = 0;
			goto badl;
		}
		return(1);
		}
	if (equal(keyw, "dumpdn")) {
		if (n<3)
			goto badl;
		if (sscanf(dev, "%ld", &num) <= 0)
			goto badl;
		if(dumpdn >= 0)
			fprintf(stderr, "\ndumpdn: only one!\n");
		else
			dumpdn = num;
		return(1);
	}
	if (equal(keyw, "dumplo")) {
		if (n<3)
			goto badl;
		if (sscanf(dev, "%ld", &num) <= 0)
			goto badl;
		if(dumplo >= 0)
			fprintf(stderr, "\ndumplo: only one!\n");
		else
			dumplo = num;
		return(1);
	}
	if (equal(keyw, "dumphi")) {
		if (n<3)
			goto badl;
		if (sscanf(dev, "%ld", &num) <= 0)
			goto badl;
		if(dumphi >= 0)
			fprintf(stderr, "\ndumphi: only one!\n");
		else
			dumphi = num;
		return(1);
	}
	if (equal(keyw, "done"))
		return(0);
/*
 * For multiple MSCP cnltrs...
 */
	if(equal("rc", dev) || equal("rq", dev))
		sprintf(dev, "ra");
	if (equal(keyw, "root")) {
		if (n<4)
			goto badl;
		for (q=table; q->name; q++) {
			if (equal(q->name, dev)) {
				q->key |= ROOT;
				rootmin = num;
				return(1);
			}
		}
		fprintf(stderr, "Can't find root\n");
		return(1);
	}
	if (equal(keyw, "swap")) {
		if (n<4)
			goto badl;
		for (q=table; q->name; q++) {
			if (equal(q->name, dev)) {
				q->key |= SWAP;
				swapmin = num;
				return(1);
			}
		}
		fprintf(stderr, "Can't find swap\n");
		return(1);
	}
	if (equal(keyw, "eldev")) {
		if (n<4)
			goto badl;
		for (q=table; q->name; q++) {
			if (equal(q->name, dev)) {
				q->key |= ERRLOG;
				elmin = num;
				return(1);
			}
		}
		fprintf(stderr, "Can't find eldev\n");
		return(1);
	}
	if (equal(keyw, "pipe")) {
		if (n<4)
			goto badl;
		for (q=table; q->name; q++) {
			if (equal(q->name, dev)) {
				q->key |= PIPE;
				pipemin = num;
				return(1);
			}
		}
		fprintf(stderr, "Can't find pipe\n");
		return(1);
	}
	fprintf(stderr, "%s: cannot find\n", keyw);
	return(1);
badl:
	fprintf(stderr, "Bad line: %s", line);
	return(1);
}

equal(a, b)
char *a, *b;
{
	return(!strcmp(a, b));
}

match(a, b)
register char *a, *b;
{
	register char *t;

	for (t = b; *a && *t; a++) {
		if (*a != *t) {
			while (*a && *a != '|')
				a++;
			if (*a == '\0')
				return(0);
			t = b;
		} else
			t++;
	}
	if (*a == '\0' || *a == '|')
		return(1);
	return(0);
}
