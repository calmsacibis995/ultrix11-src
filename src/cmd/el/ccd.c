
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)ccd.c	3.1	7/8/87";
/*
 * ULTRIX-11 crash dump copy program (CCD)
 *
 * Fred Canter 10/10/83
 * Ohms 8/84
 * Chung-Wu Lee 2/12/85 - add tk50
 *
 * Copies the crash dump from:
 *	1) magtape
 *	2) TK50 magtape
 *	3) rx50/rx33 diskettes
 *	4) the swap area of the system disk
 * to a file (default: /usr/crash/core)
 *
 * Usage:	interactive !
 *
 * 1.	For dumps to the swap area of the system disk the crash 
 *	dump is written to the swap area starting at block number 
 *	(swaplo + 300). The 300 block offset allows the system to 
 *	reboot without stepping on the crash dump in the swap area.
 *
 * 2.	CCD must be run immediately after the system has been rebooted
 *	and any file system repairs have been completed.
 *
 * 3.	CCD makes gross checks to verify that there is a valid crash
 *	dump in the swap area.
 *
 * 4.	Only super-user can run CCD.
 */

#include <stdio.h>
#include <a.out.h>
#include <sys/devmaj.h>
#include <sys/types.h>
#include <sys/tk_info.h>
#include <sys/mtio.h>
#include <signal.h>

#define	YES	1
#define	NO	0

char	*unixname = "/unix";		/* Default kernel name		*/
char	*memfile = "/dev/mem";		/* /dev/mem			*/
char	*dumpfile = "/dev/dump";	/* Default name for dump device	*/
char	*corefile = "/usr/crash/core";	/* Default place to dump to	*/
char	ifile[50];			/* Name of the dump device	*/
char	ofile[50];			/* Place to put the core dump	*/
char	buf[10240];			/* Buffer for doing the copy	*/
char	line[50];			/* Temp buffer for user input	*/
int	fi = -1;			/* Input file descriptor	*/
int	fo = -1;			/* Output file descriptor	*/
char	tk_ctid;			/* Tmscp id - TK50 or TU81	*/
long	dataoff;			/* Offset of (text)data segment	*/
					/* for (non-)split I/D kernel	*/

/* Argument to pass to fd_exit() */
#define	READ_ERROR	0
#define	WRITE_ERROR	1
#define	COPY_DONE	2

/* Bit values for density argument for getdensity() */
#define	D800	01
#define	D1600	02
#define	D6250	04

/* This is used to rewind the tape after determining the blocking factor */
struct mtop mtop = {
	MTREW, 1,
};

/*
 * We have two namelists, one for the running kernel and one
 * for the kernel that crashed.  -Dave Borman 9/30/85
 */
struct nlist nlr[] = {
#define	X_REALMEM	0
	{ "_realmem" },
#define	X_TK_CTID	1
	{ "_tk_ctid" },
	{ "" },
};

struct nlist nld[] = {
#define	X_DUMP		0
	{ "dump" },
#define	X_DMP_DN	1
	{ "dmp_dn" },
#define	X_DMP_RX	2
	{ "dmp_rx" },
#define	X_DMP_TK	3
	{ "dmp_tk" },
#define	X_RL_DMPLO	4
	{ "rl_dmplo" },
#define	X_RL_DMPHI	5
	{ "rl_dmphi" },
#define	X_RA_DMPLO	6
	{ "ra_dmplo" },
#define	X_RA_DMPHI	7
	{ "ra_dmphi" },
#define	X_RP_DMPLO	8
	{ "rp_dmplo" },
#define	X_RP_DMPHI	9
	{ "rp_dmphi" },
#define	X_HK_DMPLO	10
	{ "hk_dmplo" },
#define	X_HK_DMPHI	11
	{ "hk_dmphi" },
#define	X_HP_DMPLO	12
	{ "hp_dmplo" },
#define	X_HP_DMPHI	13
	{ "hp_dmphi" },
	{ "" },
};

char	*rxnm = "\nFloppy disk unit number < 1 > ? ";
char	*rxnh[] =
{
	"",
	"Enter the unit number of the floppy drive where the dump diskettes",
	"will be loaded, then press <RETURN>. The default unit number is",
	"1, press <RETURN> to use the default unit.",
	"",
	0
};

char	*mtnm = "\nMagtape unit number < 0 > ? ";
char	*mtnh[] =
{
	"",
	"Enter the unit number of the tape drive where the crash dump",
	"tape is mounted, then press <RETURN>. To use the default unit",
	"number of zero, press <RETURN>.",
	"",
	0
};

char	*mtdh[] =
{
	"",
	"Enter the density at which the crash dump tape was written, then",
	&buf[512],			/* filled in by getdensity() */
	"there is no default.",
	"",
	0
};

/*
 * This string gets sprintf()ed into a buffer along with the default.
 */
char	*idevm = "\nCopy dump from ([m]agtape, [s]wap area, [r]x50/33, [t]k50) < %c > ? ";
char	*idevh[] =
{
	"",
	"Enter the first letter of the crash dump input device, that is:",
	"",
	"  m - copy the dump from a magtape",
	"  s - copy the dump from the system disk swap area",
	"  r - copy the dump from RX50/RX33 diskette(s)",
	"  t - copy the dump from TK50 magtape",
	"  ? - no default, current kernel has no dump device",
	"",
	"The letter enclosed in < > is the default response. This will be",
	"the dump device configured into the current ULTRIX-11 kernel. To",
	"use the default dump input device press <RETURN>.",
	"",
	0
};

char	*filem = "\nCopy crash dump to file < /usr/crash/core > ? ";
char	*fileh[] =
{
	"",
	"Enter the full pathname of the file that is to receive the crash",
	"dump, followed by <RETURN>. Type just <RETURN> to use the default",
	"file (/usr/crash/core).",
	"",
	"Ensure that the file system, where the output file will be created,",
	"has sufficient free space to hold the crash dump.  Use the disk free",
	"(df) command to print the amount of free space.",
	"",
	0
};
char	*nlrm = "\nFile containing currently running kernel < /unix > ? ";
char	*nlrh[] =
{
	"",
	"Enter the name of the file from which the currently running system",
	"was bootstrapped. Type just <RETURN> to use the default /unix.",
	"",
	"The file containg the currently running system contains the namelist",
	"which is a symbol table used to obtain the values of certain",
	"operating system parameters from memory. These parameters include",
	"how much memory is on the system.",
	"",
	"The name of the currently running kernel should always be /unix.",
	"However, if you booted a different kernel file and have not yet",
	"renamed it /unix, make sure you enter the correct name.",
	"",
	0
};
char	*nldm = "\nFile containing kernel that crashed < /unix > ? ";
char	*nldh[] =
{
	"",
	"Enter the name of the file from which contains the kernel that",
	"crashed. Type just <RETURN> to use the default /unix.",
	"",
	"The file containing the kernel that crashed contains the namelist",
	"which is a symbol table used to obtain the values of certain",
	"operating system parameters at the time of the crash. These",
	"parameters include the crash dump device.",
	"",
	0
};
char	*confm = "\nReady to begin Copy <y or n> ? ";
char	*confh[] =
{
	"",
	"This is the last chance to abort the copy. Type y or n followed",
	"by <RETURN>, y for yes or n for no. Typing just <RETURN> will be",
	"treated as a no answer.",
	"",
	0
};

/*
 * nblkm gets sprintf()ed into a buffer along with the block count
 *	-Dave Borman 9/29/85
 */
char	*nblkm = "\nNumber of K bytes to copy < %d > ? ";
char	*nblkh[] =
{
	"",
	"Enter the number of K bytes (1024 bytes) to be copied followed by",
	"<RETURN>.  Type just <RETURN> to use the default, which is based",
	"upon the amount of memory on the system.",
	"",
	"This allows the length of the output file to be limited to a given",
	"number of K bytes. This saves copy time and disk space.  The copy",
	"always begins at the start of the crash dump.",
	"",
	"If the disk space is available, it is wise to copy the entire crash",
	"dump. Remember that if the dump is saved in the swap area, once the",
	"system comes up in multi-user mode and becomes busy, the swap area",
	"will be overwritten!",
	"",
	0
};

main()
{
	register int	i, j, k;
	int		mem, fdd, dd_in, dd_maj, unit, *dumpbuf;
	unsigned int	dmplo, dmphi, nblk, dsize, ov[16];
	int		onintr();
	struct exec	dump_exc;


	signal(SIGINT, onintr);
	signal(SIGQUIT, onintr);

	printf("\n\nULTRIX-11 Crash Dump Copy Program\n");
	printf("<Respond to a question with ?<RETURN> for help>\n");
	printf("<Help text contains important information, must reading!>\n");
	/*
	 * Get names of the currently running kernel
	 * and the crashed kernel.
	 */
	if (getline(line, nlrm, nlrh) == 1)
		strcpy(line, unixname);
	if (nlist(line, nlr) < 0)
		errexit(0, "\nCan't access namelist in %s\n", line);
	if ((mem = open(memfile, 0)) < 0)
		errexit(1, "\nCan't access memory (%s)\n", memfile);

	if (getline(line, nldm, nldh) == 1)
		strcpy(line, unixname);
	if (nlist(line, nld) < 0)
		errexit(0, "\nCan't access namelist in %s\n", line);
	if ((fdd = open(line, 0)) < 0)
		errexit(1, "\nCan't open %s\n", line);

	/*
	 * Set up to read values from /unix.  If we are
	 * split I/D, the the stuff we want is in the
	 * data segement, if we aren't split I/D then
	 * the stuff we want is in the text segment.
	 * So, dataoff points to the data for split I/D,
	 * and to the text for non-split I/D.
	 */
	dataoff = sizeof(struct exec);
	read(fdd, (char *)&dump_exc, sizeof(struct exec));
	read(fdd, (char *)&ov[0], 32);	/* get overlay sizes */
	switch (dump_exc.a_magic) {
	case 0450:
		dataoff += 2L*sizeof(struct ovlhdr);
		break;
	case 0451:
		dataoff += sizeof(struct ovlhdr);
		dataoff += ov[8] + ov[9] + ov[10] + ov[11];
		dataoff += ov[12] + ov[13] + ov[14] + ov[15];
	case 0431:
		dataoff += (unsigned)dump_exc.a_text;
		dataoff += ov[1] + ov[2] + ov[3] + ov[4];
		dataoff += ov[5] + ov[6] + ov[7];
		/*FALLTHROUGH*/
	case 0430:
		dataoff += sizeof(struct ovlhdr);
		break;
	}

	/* read symbols from kernel that crashed */
	if (nld[X_REALMEM].n_value == 0)
		errexit(0, "\nCan't find symbol realmem in namelist\n");
	if(nld[X_DUMP].n_value == 0)		/* no dump dev */
		dd_in = '?';
	else if(nld[X_DMP_TK].n_value)		/* TK50 */
		dd_in = 't';
	else if(nld[X_DMP_RX].n_value)		/* RX50/RX33 */
		dd_in = 'r';
	else if(nld[X_DMP_DN].n_value)		/* swap area */
		dd_in = 's';
	else
		dd_in = 'm';		/* magtape */

	sprintf(buf, idevm, dd_in);
	for (;;) {
		if (getline(line, buf, idevh) == 1) {
			if (dd_in == '?') {
			   printf("\nNO DEFAULT: please enter input device!\n");
			   continue;
			}
		} else {
			switch (line[0]) {
			case 't':
			case 'm':
			case 'r':
			case 's':
				dd_in = line[0];
				break;
			default:
				printf("\n(%s) - not a valid input device!\n",
									line);
				continue;
			}
		}
		if (dd_in == 't') {
			if (nlr[X_TK_CTID].n_value) {
				lseek(mem, (long)nlr[X_TK_CTID].n_value, 0);
				read(mem, (char *)&tk_ctid, sizeof(tk_ctid));
				i = (tk_ctid >> 4) & 017;
				if (i != TK50 && i != TU81) {
					printf("\n(%o) - unknown type of TMSCP magtape !\n", i);
					continue;
				}
			} else {
				printf("\n(%s) - kernel not configured for TMSCP magtape !\n", line);
				continue;
			}
		}
		break;
	}
	i = D1600|D6250;
	unit = 0;
	switch (dd_in) {
	case 'm':		/* magtape */
		unit = getunit(mtnm, mtnh, 0, 7);
		i = D800|D1600;
		/*FALLTHROUGH*/
	case 't':		/* TK50 magtape */
		if (dd_in == 't' && ((tk_ctid>>4)&017) == TK50)
			sprintf(ifile, "/dev/rtk0");
		else
			getdensity(ifile, unit, i);
		break;
	case 'r':		/* RX50/RX33 */
		unit = getunit(rxnm, rxnh, 1, 3);
		sprintf(ifile, "/dev/rrx%d", unit);
		break;
	case 's':		/* swap area */
		/* read values from a.out of kernel or /dev/mem */
		if (nld[X_DMP_DN].n_value == 0) {
			errexit(0, "\ndmp_dn missing: system disk not dump device!\n");
		} else {
			lseek(fdd, (long)nld[X_DMP_DN].n_value + dataoff, 0);
			read(fdd, (char *)&unit, sizeof(unit));
		}
		if (nld[X_RL_DMPLO].n_value && nld[X_RL_DMPHI].n_value) {
			dd_maj = RL_RMAJ;
			dmplo = nld[X_RL_DMPLO].n_value;
			dmphi = nld[X_RL_DMPHI].n_value;
		} else if(nld[X_RA_DMPLO].n_value && nld[X_RA_DMPHI].n_value) {
			dd_maj = RA_RMAJ;
			dmplo = nld[X_RA_DMPLO].n_value;
			dmphi = nld[X_RA_DMPHI].n_value;
		} else if(nld[X_RP_DMPLO].n_value && nld[X_RP_DMPHI].n_value) {
			dd_maj = RP_RMAJ;
			dmplo = nld[X_RP_DMPLO].n_value;
			dmphi = nld[X_RP_DMPHI].n_value;
		} else if(nld[X_HK_DMPLO].n_value && nld[X_HK_DMPHI].n_value) {
			dd_maj = HK_RMAJ;
			dmplo = nld[X_HK_DMPLO].n_value;
			dmphi = nld[X_HK_DMPHI].n_value;
		} else if(nld[X_HP_DMPLO].n_value && nld[X_HP_DMPHI].n_value) {
			dd_maj = HP_RMAJ;
			dmplo = nld[X_HP_DMPLO].n_value;
			dmphi = nld[X_HP_DMPHI].n_value;
		} else
			errexit(0, "\nCan't find dump disk!\n");
		lseek(fdd, (long)dmplo + dataoff, 0);
		read(fdd, (char *)&dmplo, sizeof(dmplo));
		lseek(fdd, (long)dmphi + dataoff, 0);
		read(fdd, (char *)&dmphi, sizeof(dmphi));
		strcpy(ifile, dumpfile);
		unlink(ifile);
		i = (dd_maj << 8) | (unit << 3) | 7;
		if (mknod(ifile, 020400, i) < 0)
			errexit(1, "\nCan't create %s file!\n", ifile);
		break;
	}
	close(fdd);			/* all done with the crashed kernel */
	if (getline(ofile, filem, fileh) == 1)
		strcpy(ofile, corefile);
	if ((access(ofile, 0) == 0) &&
	    (getline(line, "\n\7\7\7File exists, ok to overwrite it", 0) == NO))
		errexit(0, "\nCopy aborted!\n");
	if ((fo = creat(ofile, 0644)) < 0)
		errexit(1, "\nCan't create %s\n", ofile);
	/*
	 * Get memory size for current system, and convert
	 * it to 1K bytes (from clicks).
	 */
	lseek(mem, (long)nlr[X_REALMEM].n_value, 0);
	read(mem, (char *)&nblk, sizeof(nblk));
	close(mem);			/* all done with /dev/mem */
	nblk = (nblk+017)>>4;		/* convert from clicks to K bytes */
	if (nblk == 0)
		nblk = 4096;			/* 4 meg */
	sprintf(buf, nblkm, nblk);		/* put count in help message */
	for (;;) {
		if (getline(line, buf, nblkh) == 1)
			i = nblk;
		else
			i = atoi(line);
		if (i > 0) {
			nblk = i;
			break;
		}
		printf("\nBad block count, try again!\n");
	}
	ulimit(2, (long)nblk*2L);/* set max limit in 512 byte blocks */
	switch (dd_in) {
		int *tmpdump; 
	case 's':
		if ((fi = open(ifile, 0)) < 0)
			errexit(1,"\nCan't open input file (%s) for reading!\n",
					ifile);
		/*
		 * Verify valid dump in swap area, look for 5227 (inc
		 * instruction) at address dump: and >= 0 at dump+2.
		 */
		lseek(fi, (dmplo + (long)nld[X_DUMP].n_value/512L) * 512L, 0);
		dumpbuf = (int *)(buf + (nld[X_DUMP].n_value % 512));
		tmpdump = dumpbuf; 
		read(fi, (char *)dumpbuf, (sizeof(int) * 256));

		if ((*dumpbuf != 05227) || (*++dumpbuf < 0)) 
			if(*tmpdump != 00167)  /* for Non sep-id, it will be jmp instruction */
		     		printf("\n\7\7\7Valid dump not present in swap area!\n");
		i = (dmphi - dmplo)/2;		/* get size in K bytes */
		if (nblk < (unsigned)i) 
			i = nblk;
		printf("\nCopy %d K bytes starting at disk block", i);
		printf(" %d in swap area to %s\n", dmplo, ofile);
		if ((getline(line, confm, confh) == 1) || (line[0] != 'y'))
			errexit(0, "\nCopy aborted!\n");
		lseek(fi, (long)dmplo * 512L, 0);
		i = docopy(i, 0, 5120);
		break;
	case 'r':
		k = 0;	/* floppy disk number */
		i = 0;	/* number of Kbytes copied */
		do {
			k++;
			printf("\nInsert dump diskette number");
			printf(" %d into floppy disk unit %d.", k, unit);
			prtc();
			if ((fi = open(ifile, 0)) < 0)
			    errexit(1, "\nCan't open input file (%s) for reading!\n", ifile);
			/*
			 * Determine which type diskette, rx50 or rx33?
			 * If we can read block 800, its rx33,
			 * otherwise assume rx50.
			 */
			lseek(fi, (long)(512L * 800L), 0);
			if(read(fi, buf, 512) == 512)
				dsize = 1200;
			else
				dsize = 400;
			lseek(fi, (long)0L, 0);
			j = (nblk > dsize) ? dsize : nblk;
			i = docopy(j, i, 5120);
				nblk -= j;
			if (nblk <= 0)
				fd_exit(COPY_DONE, i, 0);
			close(fi);
			printf("\n\nTotal of %d K bytes copied", i);
			printf(" from %d diskette(s).", k);
			printf("\n\nRemove dump diskette number");
			printf(" %d from floppy disk unit %d.", k, unit);
			prtc();
		} while (getline(line, "\nCopy more diskettes ", 0) == YES);
		break;
	case 'm':
	case 't':
		printf("\nMount the crash dump tape on magtape unit %d.", unit);
		prtc();
		if ((fi = open(ifile, 0)) < 0)
			errexit(1,"\nCan't open input file (%s) for reading!\n",
					ifile);
		j = read(fi, buf, 10240);	/* determine blocking factor */
		i = (j/512)*512;		/* round to 512 byte boundry */
		if (j < 0 || i != j)
			fd_exit(READ_ERROR, 0, j);
		ioctl(fi, MTIOCTOP, &mtop);	/* rewind the tape */
		i = (i+1023)/1024;		/* get Kbytes for rounding */
		nblk = (nblk + i - 1)/i;	/* round up block count */
		nblk *= i;
		i = docopy(nblk, 0, j);
		break;
	}
	fd_exit(COPY_DONE, i, 0);
	exit(0);
}

/*
 * Get density fills in "str" with the tape name.
 * unit is the tape unit, and density is a bit mask
 * of the valid tape densities.
 */
getdensity(str, unit, density)
char	*str;
int	unit;
register int	density;
{
	register int c;

	/* set up question and help message */
	sprintf(&buf[512], "press <RETURN>. The density will be%s%s%s%s,",
		density&D800  ? " 800 BPI" : "",
		density&D800  ? ((density==D800|D1600) ? " or" : ",") : "",
		density&D1600 ? " 1600 BPI" : "",
		density&D6250 ? " or 6250 BPI" : "");
	sprintf(buf, "\nCrash dump tape density <%s%s%s > ? ",
		density&D800  ? " 800" : "",
		density&D1600 ? " 1600" : "",
		density&D6250 ? " 6250" : "");
	for (;;) {
		if (getline(line, buf, mtdh) == 1)
			continue;
		if (density&D800 && strcmp("800", line) == 0)
			c = 'm';
		else if(density&D1600 && strcmp("1600", line) == 0)
			c = 'h';
		else if(density&D1600 && strcmp("6250", line) == 0)
			c = 'g';
		else {
			printf("\nBad tape density!\n");
			continue;
		}
		break;
	}
	sprintf(str, "/dev/r%ct%d", c, unit);
}

/*
 * Get unit number. Arguments are: question, help message,
 * default unit number, and maximum unit number.
 */
getunit(nm, nh, defunit, maxunit)
{
	for (;;) {
		if (getline(line, nm, nh) == 1)
			return(defunit);
		if ((line[0] < '0') || (line[0] > '0' + maxunit)) {
			printf("\nBad unit number!\n");
			continue;
		}
		return(line[0] - '0');
	}
	/*NOTREACHED*/
}

docopy(bcnt, nb, bufsize)
register int bcnt;
int nb, bufsize;
{
	register int i, cnt;
	int dblk = 0;

	/*
	 * We do the copy in terms of 512 byte blocks, because
	 * tapes get dumped with a 512 byte blocking factor.
	 * So, we convert to 512 byte blocks, but return values
	 * based on 1K byte blocks.
	 */
	bcnt *=2;
	while (bcnt > 0) {
		cnt = (bcnt > (bufsize/512)) ? bufsize : bcnt*512;
		if ((i = read(fi, buf, cnt)) != cnt) {
			if (i > 0) {	/* write out partial block */
				if ((cnt = write(fo, buf, i)) > 0)
					dblk += cnt/512;
			}
			fd_exit(READ_ERROR, nb + dblk/2, i);
		}
		if ((i = write(fo, buf, cnt)) > 0)
			dblk += i/512;
		if (i != cnt)
			fd_exit(WRITE_ERROR, nb + dblk/2, i);
		bcnt -= i/512;
	}
	return(nb + dblk/2);	/* return cum. total of K bytes copied */
}

fd_exit(how, bcnt, err)
int how, bcnt;
{
	printf("\n%s %d K bytes copied\n",
		(how == READ_ERROR) ? "Read error:" :
		(how == WRITE_ERROR) ? "Write error:" : "Copy done:", bcnt);
	if (err < 0)
		perror("ccd");
	closeup();
	if (how == COPY_DONE)
		exit(0);
	exit(1);
}

/*
 * msg  - question to be ask
 * hmsg - help message (0 = no help, must answer y or n)
 */
getline(buf, msg, hmsg)
char	*buf;
char	*msg;
char	**hmsg;
{
	register int	i, cc;
	char		line[132];

	for (;;) {
		printf("%s", msg);
		if (hmsg == 0)
			printf(" <y or n> ? ");
		fflush(stdout);
		cc = read(0, (char *)line, 132);
		if (cc <= 0)
			continue;
		if (cc > 50) {
			printf("\nToo many characters, try again!\n");
			continue;
		}
		if (hmsg && (cc == 2) && (line[0] == '?')) {
			for(i=0; hmsg[i]; i++)
				printf("\n%s", hmsg[i]);
			continue;
		}
		line[50] = '\0';
		for (i=0; i<50; i++) {
			if (((buf[i] = line[i]) >= 'A') && (buf[i] <= 'Z'))
				buf[i] |= 040;	/* force lower case */
			else if ((buf[i] == '\r') || (buf[i] == '\n'))
				buf[i] = 0;
		}
		if (hmsg == 0) {
			if (!strcmp(buf, "y") || !strcmp(buf, "yes"))
				return(YES);
			if (!strcmp(buf, "n") || !strcmp(buf, "no"))
				return(NO);
			printf("\nPlease answer yes or no!\n");
			continue;
		}
		return(cc);
	}
	/*NOTREACHED*/
}

prtc()
{
	printf("\n\nPress <RETURN> to continue: ");
	while (getchar() != '\n')
		;
}

onintr()
{
	errexit(0, "\n\n\7\7\7Interrupt: ccd aborted\n\n");
}

errexit(doperror, fmt, a1, a2)
{
	printf(fmt, a1, a2);
	if (doperror)
		perror("ccd");
	closeup();
	exit(1);
}

closeup()
{
	unlink(dumpfile);
	if (fi >= 0)
		close(fi);
	if (fo >= 0)
		close(fo);
	sync();
}
