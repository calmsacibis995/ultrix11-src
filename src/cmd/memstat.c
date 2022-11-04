
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)memstat.c	3.1	10/27/87";
#include <stdio.h>
#include <sys/param.h>
#include <a.out.h>
#include <sys/proc.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <time.h>
#include <sys/timeb.h>
#include <core.h>
#include <sys/text.h>
#include <sys/map.h>
#include <sys/clist.h>
#include <sys/ipc.h>
#include <sys/msg.h>

long	timbuf;
char	*ap;
char	*timezone();
char	*asctime();
struct	tm *localtime();

struct nlist nli[] = {
	{ "_proc" },
#define				X_PROC		0
	{ "_nproc" },
#define				X_NPROC		1
	{ "_mapsize" },
#define				X_MAPSIZE	2
	{ "_coremap" },
#define				X_COREMAP	3
	{ "_usermem" },
#define				X_USERMEM	4
	{ "_realmem" },
#define				X_REALMEM	5
#define				X_REQUIRED	5
	{ "_bpaddr" },
#define				X_BPADDR	6
	{ "_nbuf" },
#define				X_NBUF		7
	{ "_clststrt" },
#define				X_CLSTSTRT	8
	{ "_nclist" },
#define				X_NCLIST	9
	{ "_mbbase" },
#define				X_MBBASE	10
	{ "_mbsize" },
#define				X_MBSIZE	11
	{ "_mauscore" },
#define				X_MAUSCORE	12
	{ "_mausend" },
#define				X_MAUSEND	13
	{ "_msgbase" },
#define				X_MSGBASE	14
	{ "_msginfo" },
#define				X_MSGINFO	15
	{ "_de_dmaa" },
#define				X_DE_DMAA	16
	{ "_de_dmas" },
#define				X_DE_DMAS	17
	{ 0 }
 };

int mem, swmem, interval;
int nprc, dsrt;
int	mapsiz;
char *infile = "/unix";
char *coref = "/dev/mem";
unsigned realmem;
unsigned usrbase;
char *usage = "memstat: usage  memstat [ifn] [interval] [corefile] [namelist]\n";

char *memtypes[] = {
	"",
	"*FREE MEMORY*",
#define MFRE	1
	"*TEXT SEGMENT*",
#define TXT	2
	"*BUFFERS*",
#define	BUFS	3
	"*CLISTS*",
#define	CLISTS	4
	"*MBUFS*",
#define	MBUFS	5
	"*MAUS*",
#define	MAUS	6
	"*MESSAGES*",
#define	MSG	7
	"*DEUNA DMA*",
#define	DE_DMA	8
	"*(UNKNOWN)*",
#define	MISSING	9
};

struct prcsrt{
	int		srtsts;
	int		srtpid;
	unsigned	srtsadr;
	unsigned	srtxadr;
	unsigned	srtxsiz;
	unsigned	srttsiz;
	unsigned	srtdsiz;
	unsigned	srtssiz;
	char		srtarg[DIRSIZ+2];
};

struct proc prcs;
struct text txt;
struct user u;
struct map  cmp;
struct prcsrt prc[256];
int soscnt, sopcnt;
int sospnt;
unsigned de_dmaa;
unsigned de_dmas;

main(argc, argp)
int argc;
char *argp[];
{
	int cnt, cnt1;
	char *ap;
	int	compare();

	argp++;
	for(ap = *argp++; ap && *ap; ap++)
		switch(*ap){
			case 'f':
				coref = *argp++;
				break;
			case 'i':
				interval = atoi(*argp++);
				break;
			case 'n':
				infile = *argp++;
				break;
			case '-':
				break;
			default:
				fprintf(stderr, "%s", usage);
				exit(1);
				break;
		}
	initmem();
	while(1){
		dsrt = lprc();
		qsort(prc, dsrt, sizeof(struct prcsrt), compare);
		memsts();
		if(interval)
			sleep(interval);
		else
			exit(0);
	}
}

lprc()
{
	register int cnt;
	register struct prcsrt *ptr;

	for(cnt = 0, ptr = &prc[0]; cnt < nprc; cnt++) {
		lseek(mem, (long)(nli[X_PROC].n_value+(sizeof(struct proc)*cnt)), 0);
		read(mem, &prcs, sizeof(struct proc));
		if(prcs.p_stat == 0 || prcs.p_flag == 0)
			continue;
		if((prcs.p_flag & SLOAD) == 0)
			continue;
		if(prcs.p_stat == SZOMB)
			continue;
		lseek(swmem, ctob((long)prcs.p_addr), 0);
		if(read(swmem, &u, sizeof(struct user)) != sizeof(struct user)){
			printf("Fatal Ublock read. PID = %d\n", prcs.p_pid);
			exit(1);
		}
		if(prcs.p_pid == 0){
			strcpy(ptr->srtarg, "swapper");
			usrbase = prcs.p_addr;
		} else
			strcpy(ptr->srtarg, &u.u_comm);
		ptr->srtxadr = 0;
		ptr->srtsts = u.u_exdata.ux_mag;
		ptr->srtxsiz = prcs.p_size;
		ptr->srttsiz = u.u_tsize;
		if(prcs.p_textp){
			lseek(mem, (long)prcs.p_textp, 0);
			read(mem, &txt, sizeof(struct text));
			ptr->srtxadr = txt.x_caddr;
			ptr->srttsiz = txt.x_size;
		}
		ptr->srtpid = prcs.p_pid;
		ptr->srtsadr = prcs.p_addr;
		ptr->srtdsiz = u.u_dsize;
		ptr->srtssiz = u.u_ssize;
		ptr++;
		if(prcs.p_textp){
			if(found(txt.x_caddr, ptr)){
				ptr->srtsts = TXT;
				ptr->srtsadr = txt.x_caddr;
				ptr->srtxsiz = txt.x_size;
				strcpy(ptr->srtarg, &u.u_comm);
				ptr++;
			}
		}
	}

	lseek(mem, (long)nli[X_COREMAP].n_value, 0);
	read(mem, &cmp, sizeof(struct map));	/* skip the first entry */
	for(cnt = 1; cnt < mapsiz; cnt++) {
		read(mem, &cmp, sizeof(struct map));
		if(cmp.m_size == 0)
			continue;
		ptr->srtsts = MFRE;
		ptr->srtsadr = cmp.m_addr;
		ptr->srtxsiz = cmp.m_size;
		ptr++;
	}
	if (nli[X_BPADDR].n_value) {
		lseek(mem, (long)nli[X_NBUF].n_value, 0);
		read(mem, &(ptr->srtxsiz), sizeof(ptr->srtxsiz));
		ptr->srtxsiz <<= (BSHIFT-6);
		lseek(mem, (long)nli[X_BPADDR].n_value, 0);
		read(mem, &(ptr->srtsadr), sizeof(ptr->srtsadr));
		ptr->srtsts = BUFS;
		ptr++;
	}
	if (nli[X_CLSTSTRT].n_value && nli[X_NCLIST].n_value) {
		lseek(mem, (long)nli[X_NCLIST].n_value, 0);
		read(mem, &(ptr->srtxsiz), sizeof(ptr->srtxsiz));
		ptr->srtxsiz = ((long)(ptr->srtxsiz+1)*sizeof(struct cblock))/64;
		lseek(mem, (long)nli[X_CLSTSTRT].n_value, 0);
		read(mem, &(ptr->srtsadr), sizeof(ptr->srtsadr));
		ptr->srtsts = CLISTS;
		ptr++;
	}
	if (nli[X_MBBASE].n_value && nli[X_MBSIZE].n_value) {
		lseek(mem, (long)nli[X_MBSIZE].n_value, 0);
		read(mem, &(ptr->srtxsiz), sizeof(ptr->srtxsiz));
		ptr->srtxsiz /= 64;
		lseek(mem, (long)nli[X_MBBASE].n_value, 0);
		read(mem, &(ptr->srtsadr), sizeof(ptr->srtsadr));
		if (ptr->srtxsiz && ptr->srtsadr) {
			ptr->srtsts = MBUFS;
			ptr++;
		}
	}
	if (nli[X_MAUSCORE].n_value && nli[X_MAUSEND].n_value) {
		lseek(mem, (long)nli[X_MAUSEND].n_value, 0);
		read(mem, &(ptr->srtxsiz), sizeof(ptr->srtxsiz));
		lseek(mem, (long)nli[X_MAUSCORE].n_value, 0);
		read(mem, &(ptr->srtsadr), sizeof(ptr->srtsadr));
		ptr->srtxsiz -= ptr->srtsadr;
		ptr->srtsts = MAUS;
		ptr++;
	}
	if (nli[X_MSGBASE].n_value && nli[X_MSGINFO].n_value) {
		struct msginfo msginfo;
		lseek(mem, (long)nli[X_MSGINFO].n_value, 0);
		read(mem, &msginfo, sizeof(msginfo));
		if (ptr->srtxsiz = ((long)msginfo.msgseg * msginfo.msgssz)/64) {
			lseek(mem, (long)nli[X_MSGBASE].n_value, 0);
			read(mem, &(ptr->srtsadr), sizeof(ptr->srtsadr));
			ptr->srtsts = MSG;
			ptr++;
		}
	}
	if (nli[X_DE_DMAA].n_value) {
		lseek(mem, (long)nli[X_DE_DMAA].n_value, 0);
		read(mem, &de_dmaa, sizeof(de_dmaa));
		lseek(mem, (long)nli[X_DE_DMAS].n_value, 0);
		read(mem, &de_dmas, sizeof(de_dmas));
printf("%o %d\n", de_dmaa, de_dmas);
		if (de_dmaa) {
			ptr->srtsts = DE_DMA;
			ptr->srtsadr = de_dmaa;
			ptr->srtxsiz = de_dmas;
			ptr++;
		}
	}
	lseek(mem, (long)nli[X_REALMEM].n_value, 0);
	read(mem, &realmem, sizeof(realmem));
	return(ptr - prc);
}

memsts()
{
	int scnt;
	unsigned tadr, dadr, sadr;
	register struct prcsrt *ptr;
	unsigned missing;
	char *s;

	printf("\nTotal Memory = %u Kbytes\t", realmem/16);
	date();
	printf("\n");
printf("  Addr    Command          Pid    Size   Ublock     Text     Data      Stack\n");
printf("------------------------------------------------------------------------------\n");
	printf("    0     %-14s        %6ld\n    v\n",
		"ULTRIX-11", ctob((long)usrbase));
	missing = usrbase;
	for (ptr = &prc[0]; ptr < &prc[dsrt]; ptr++) {
		if (missing != ptr->srtsadr) {
			printf("%06o00  %-14s         %5ld\n    v\n",
				missing, memtypes[MISSING],
				(long)(ptr->srtsadr - missing)*64);
			missing = ptr->srtsadr;
		}
		missing += ptr->srtxsiz;
		switch (ptr->srtsts) {
		case MFRE:
		case BUFS:
		case CLISTS:
		case MBUFS:
		case MAUS:
		case MSG:
		case DE_DMA:
			printf("%06o00  %-14s         %5ld\n    v\n",
				ptr->srtsadr, memtypes[ptr->srtsts],
				(long)ptr->srtxsiz*64);
			continue;
		case TXT:
			printf("%06o00  %-14s         %5ld\n",
				ptr->srtsadr, memtypes[TXT],
				(long)ptr->srtxsiz*64);
			printf("    |\t  %-14s\n    v\n", ptr->srtarg);
			continue;
		default:
			break;
		}

		tadr = ptr->srtxadr > 0 ? ptr->srtxadr : ptr->srtsadr + USIZE;
		if(ptr->srtxadr == 0)
			dadr = ptr->srtdsiz > 0 ? tadr + ptr->srttsiz : 0;
		else
			dadr = ptr->srtdsiz > 0 ? ptr->srtsadr + USIZE : 0;
		sadr = ptr->srtssiz > 0 ? dadr + ptr->srtdsiz : 0;
		printf("%06o00  %-14s  %5d  %5ld  ", ptr->srtsadr,
			ptr->srtarg, ptr->srtpid, ((long)ptr->srtxsiz)*64);

		switch (ptr->srtsts) {
		case 0407:
			sadr = tadr + ptr->srtdsiz;
			printf("%06o00  %06o00  %8s  %06o00\n",
				ptr->srtsadr, tadr, "------->", sadr);
			printf("    v\t\t\t\t\t  %5d     %5ld     %5s     %5ld\n",
				USIZE*64, ((long)ptr->srtdsiz)*64,
				"     ", ((long)ptr->srtssiz)*64);
			break;
		case 0410:
		case 0411:
		case 0430:
		case 0431:
		case 0450:
		case 0451:
			printf("%06o00 *%06o00* %06o00  %06o00\n",
				ptr->srtsadr, tadr, dadr, sadr);
			printf("    v\t\t\t\t\t  %5d     %5ld     %5ld     %5ld\n",
				USIZE*64, ((long)ptr->srttsiz)*64,
				((long)ptr->srtdsiz)*64,
				((long)ptr->srtssiz)*64);
			break;
		case 0:
			printf("%06o00", ptr->srtsadr);
			/*FALLTHROUGH*/
		default:
			printf("\n    v\n");
			break;
		}
	}
	if (missing != realmem) {
		printf("%06o00  %-14s         %5ld\n    v\n", missing,
			memtypes[MISSING], (long)(realmem - missing)*64);
	}
	printf("%06o00\n", realmem);
}

date()
{
	register char *tzn;
	struct timeb info;
	struct tm *tp;

	ftime(&info);
	time(&timbuf);
	tp = localtime(&timbuf);
	ap = asctime(tp);
	tzn = timezone(info.timezone, tp->tm_isdst);
	printf("%.20s", ap);
	if (tzn)
		printf("%s", tzn);
	printf("%s", ap+19);
}

initmem()
{
	int cnt, cnt1;

	nlist(infile, nli);
	for(cnt = 0; cnt <= X_REQUIRED && nli[cnt].n_name[0]; cnt++){
		if(nli[cnt].n_value == 0){
			printf("memstat: FATAL: nlist failed for entry %s\n", nli[cnt].n_name);
			exit(1);
		}
	}
	if((mem = open(coref, 0)) <= 0){
		printf("memstat: FATAL : could not open %s\n", coref);
		exit(1);
	}
	swmem = open(coref, 0);
	/*
	 * get number of processes, base of swap, and mapsize
	 */
	lseek(mem, (long)nli[X_NPROC].n_value, 0);
	read(mem, &nprc, sizeof(nprc));
	lseek(mem, (long)nli[X_MAPSIZE].n_value, 0);
	read(mem, (char *)&mapsiz, sizeof(mapsiz));
}

found(tpnt, eprc)
register unsigned tpnt;
register struct prcsrt *eprc;
{
	register struct prcsrt *pt1;

	for (pt1 = &prc[0]; pt1 < eprc; pt1++)
		if(pt1->srtsadr == tpnt)
			return(0);
	return(1);
}

compare(p1, p2)
register struct prcsrt *p1, *p2;
{
	return((p1->srtsadr < p2->srtsadr) ? -1 : 1);
}
