
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)mkconf.c	3.0	4/21/86";
/*
 * ULTRIX-11 make configuration program (mkconf.c).
 *
 * Reads the system configuration from the `?conf' file
 * and creates the following system files:
 *
 *	l.s		trap and interrupt vectors
 *	c.c		configuration table
 *	mch.h		machine language assist header
 *	devmaj.h	block & raw major device numbers
 *	ovload		overlay kernel load file
 *
 * Added rk06 & rk07 support.
 * Added rm02/3 & ts11 support.
 * Added A. P. Stettner's rl11 modificatons.
 * Added dz11 support.
 * Added improved core dump code,
 * uses unibus map (if requried) to
 * insure that all of memory is dumpped.
 * Added "ov" declaration for overlay kernel.
 * Added "nsid" declaration for no separate I/D space CPU.
 * Added "nfp" declaration for excluding floating point support code.
 * Added "|" for rx|rx2 and hs|ml.
 * Added automatic configuration of the text
 * overlay segments for the overlay kernel.
 * Added "dump" declaration to select core dump tape
 * and optionally specify dump tape CSR address.
 * Added error logging support.
 * Rearranged block & raw major device numbers.
 * Added `devmaj.h' major device number header file.
 * Moved CSR addresses from drivers to c.c file.
 * Added device CSR and vector specifications to `conf' file.
 * Removed `nsid' declaration.
 * Added `ov' & `sid' to mch0.s header file.
 * Moved tunable parameters from param.h to ?conf file.
 * Added number of drives specification for most disks, 3hp, 2hk.
 * Changed major device numbers again.
 * Added DZV11 support.
 * Added number of unit spec for tapes.
 * Added UDA50/RAxx, used RF11 major/minor device numbers
 * Added bad blocking support. SEE BADS, "hm", "hp", "hk"
 * Added core dump to RL and RD disk.
 * Added support for users to add device drivers.
 * More "dummy" entries, so major device #'s don't change when new dev added.
 * Changed rx to hx.
 * Changed PC to CAT.
 * Fixed up DU11 support code.
 * Split the program up into modules. -Dave B.
 *
 * Fred Canter 7/15/83
 * Jerry Brenner 1/29/83
 *
 */

#include "mkconf.h"
#define	PASS0	0
#define	PASS1	1
#define	PASS2	2

int	nkl;
int	dumpht, dumptm, dumpts, dumptk;
int	dumprl, dumpra, dumprx;
int	dumphk, dumprp, dumphp;
int	sav, nsav;

main()
{
	while(input());

	pass1();	/* create interrupt vectors; l.s */
	pass2();	/* create configuration tables; c.c */
	pass2_3();	/* create dds.h (MSCP & HP disk parameters) */
	pass2_5();	/* create mch.h */
	pass3();	/* create devmaj.h */
	pass4();	/* create ovload */

	exit(0);
}

/*
 * pass1 -- create interrupt vectors
 */

pass1()
{
	register struct tab *p;
	register struct mscptab *mp;
	register int i;
	int vect, n;

	nkl = 0;
	if(freopen("l.s", "w", stdout) == NULL) {
		fprintf(stderr, "\nCan't open `l.s' file !\n");
		exit(1);
	}
	if(!ov)
		puke(stra70);
	else
		puke(stra40);
	puke(stra1);
	if(ov)
		puke(stra2ov);
	else
		puke(stra2id);
	puke(stra3);
	sav = nsav = 0;
	vect = 0300;
	for(p=table; p->name; p++) {
		if(p->csr == MSCPDEV || equal(p->name,"ts") || equal(p->name,"tk"))
			continue;	/* skip this for RA/TS/TK */
		if((p->count != 0) && (p->key & INTR)) {
			if((p->key & NSAV) && (p->vec >= 0300))
				nsav++;
			if((p->vec == 0304) || (p->vec == 0310))
				if(p->key & NSAV) 
					nsav++;
				else {
					sav++;
					if((p->key & EVEN) && (vect & 04))
						vect += 4;
					p->vec = vect;
					if(p->key & DLV)
						vect += p->count * 010;
					else
						vect += p->count * 04;
				}
		}
	}
	for(vect=040; vect<01000; vect += 04) {
	    if(vect == 0240) {
	    	puke(strb);
	    	vect += 014;
	    }
	    if(vect == 0300)
	    	puke(strc);
	    for(i = 0; i < nnetattach; i++) {
		if (netattach[i].vec == vect) {
		    if(vect != 0300)
		    	printf("\n. = ZERO+%o\n", netattach[i].vec);
		    printf("\t%.2sin; br5+%d.\n",
				netattach[i].name, netattach[i].unit);
		    if (netattach[i].nvec != 1) {
			printf("\t%.2sou; br5+%d.\n",
				netattach[i].name, netattach[i].unit);
			vect += 04;
		    }
		    goto contin;
		}
	    }
	    for(mp=tstab; mp->ms_dcn; mp++) {
		if (mp->ms_cn < 0)
			continue;
		if(mp->ms_vec == vect) {
			if(vect != 0300)
				printf("\n. = ZERO+%o\n", mp->ms_vec);
	    		for(p=table; p->name; p++)
				if(equal(p->name,mp->ms_dcn))
					break;
			printf(p->codea, mp->ms_cn);
			goto contin;
			}
	    }
	    for(mp=tktab; mp->ms_dcn; mp++) {
		if (mp->ms_cn < 0)
			continue;
		if(mp->ms_vec == vect) {
			if(vect != 0300)
				printf("\n. = ZERO+%o\n", mp->ms_vec);
	    		for(p=table; p->name; p++)
				if(equal(p->name,mp->ms_dcn))
					break;
			printf(p->codea, mp->ms_cn);
			goto contin;
			}
	    }
	    for(p=table; p->name; p++) {
		if(equal(p->name,"ts") || equal(p->name,"tk"))
			continue;	/* skip this for TS/TK magtape */
		if (strncmp(p->name, "if_", 3) == 0)
			continue;
		/* Special case for MSCP cntlr (MSCP disk flag)----	*/
		if(!generic&&(p->count!=0)&&(p->key&INTR)&&(p->csr==MSCPDEV)) {
		    for(mp=mstab; mp->ms_dcn; mp++)
			if((mp->ms_cn >= 0) && (mp->ms_vec == vect)) {
				if(vect != 0300)
					printf("\n. = ZERO+%o\n", mp->ms_vec);
				printf(p->codea, mp->ms_cn);
				break;
			}
		    if(mp->ms_dcn == 0)
			continue;
		    else
			break;
		}
		if((p->count != 0) && (p->key & INTR) && (p->vec == vect)) {
		    if(vect != 0300)
		    	printf("\n. = ZERO+%o\n", p->vec);
		    n = p->count;
		    if(generic && (n >= 0))	/* generic kernel */
			goto novec;	    /* boot loads device intr vectors */
		    if(n < 0)		    /* we load trap/console vectors */
		    	n = -n;
		    if(p->key & (NUNIT|TAPE))
		    	n = 1;
		    for(i=0; i<n; i++) {
		    	if(p->key & KL) {
		    		printf(p->codea, nkl, nkl);
		    		nkl++;
		    	} else
		    		printf(p->codea, i, i);
		    }
		    if((p->key&DLV) == 0)
			vect += (n - 1) * 04;
		    else
			vect += ((n - 1) * 010) + 04;
		    break;
		}
	    }
	if(p->name == 0)
novec:
	    printf("\tsvi%o; br7+%d.\n", (vect&0700), ((vect >> 2) & 017));
contin:
	    ;
	}
	printf("\n. = ZERO+1000\n");
	if(ov)
		printf("\n\tjmp\tdump\n");
	puke(strd1);
	if(!ov)
		puke(strd2);
	puke(strd3);
	for(p=table; p->name; p++)
		if ((p->count != 0) && (p->key & INTR))
			printf("\n%s%s", p->codeb, p->codec);
/*
 * Force loading of certain data structures first in BSS,
 * so they can be permanently mapped by the first unibus map register.
 */

	puke(strd4);
	for(p=table; p->name; p++)	/* this only handles HK */
		if((p->count != 0) && (p->key & BADS)) {
			printf("\n\n.globl _%s_bads", p->name);
			printf("\n\n\tmov\t_%s_bads,r0", p->name);
		}
	for(p=table; p->name; p++)	/* HP HJ HM */
		if((p->count != 0) &&
		   (equal("hp", p->name) ||
		    equal("hm", p->name) ||
		    equal("hj", p->name))) {
			printf("\n\n.globl _hp_bads");
			printf("\n\n\tmov\t_hp_bads,r0");
			break;
		}
	for(p=table; p->name; p++)	/* MSCP */
		if((p->count != 0) &&
		    equal("ra", p->name)) {
			printf("\n\n.globl _uda, _ra_rp, _ra_cp");
			printf("\n\n\tmov\t_uda,r0");
			printf("\n\n\tmov\t_ra_rp,r0");
			printf("\n\n\tmov\t_ra_cp,r0");
			break;
		}
	for(p=table; p->name; p++)	/* TK50/TU81 */
		if((p->count != 0) &&
		    equal("tk", p->name)) {
			printf("\n\n.globl _tk");
			printf("\n\n\tmov\t_tk,r0");
			break;
		}
	for(p=table; p->name; p++)	/* TS11 */
		if((p->count != 0) &&
		    equal("ts", p->name)) {
			printf("\n\n.globl _cmdpkt, _chrbuf, _mesbuf");
			printf("\n\n\tmov\t_cmdpkt,r0");
			printf("\n\n\tmov\t_chrbuf,r0");
			printf("\n\n\tmov\t_mesbuf,r0");
			break;
		}
	for(p=table; p->name; p++)	/* USER DEVICES */
		if((p->count != 0) &&
		   (equal("u1", p->name) ||
		    equal("u2", p->name) ||
		    equal("u3", p->name) ||
		    equal("u4", p->name))) {
			printf("\n\n.globl _%s_mbuf", p->name);
			printf("\n\n\tmov\t_%s_mbuf,r0", p->name);
		}
	printf("\n\n.globl _ub_end");
	printf("\n\n\tmov\t_ub_end,r0\n");
}


/*
 * pass 2 -- create configuration table
 */

pass2()
{
	register struct tab *p;
	register char *q;
	register int i;
	char *ntab;
	int numkl;
	int ntty;
	int	mauscount, maustart;

	if(freopen("c.c", "w", stdout) == NULL) {
		fprintf(stderr, "\nCan't open `c.c' file !\n");
		exit(1);
	}
	/*
	 * declarations
	 */
	puke(stre);
	if (kfpsim)
		puke(strfpe);
	else
		puke(strnofpe);
	puke(strutsn);
	if(maus == 0)
		puke(strmausno);
	else {
		printf("\n#include <sys/maus.h>\n");
		printf("struct mausmap mausmap[] = {\n");
		maustart = 0;
		for(mauscount = 0; mauscount < nmaus; mauscount++) {
			printf("\t%d,\t%d,\n",maustart,mausize[mauscount]);
			maustart += mausize[mauscount];
		}
		printf("\t-1,\t-1\n");
		printf("};\n");
	}
	if(sema == 0)
		puke(strsemno);
	if(mesg == 0)
		puke(strmsgno);
	if(flock == 0)
		puke(strflckno);
	if(shuffle == 0)
		puke(strshuff);
	if(!ubmap)
		puke(stro);	/* no uinbus map */
	if(generic)
		puke(strgk);	/* generic kernel */
	for (i=0; q=btab[i]; i++) {
		for (p=table; p->name; p++)
		if (match(q, p->name) && (p->key&BLOCK) && p->count) {
			if (p->codef) {
				if (*p->codef)
					printf("%s\n", p->codef);
			} else {
				printf("int\t%sopen(), ", p->name);
				printf("%sclose(), ", p->name);
				printf("%sstrategy();\n", p->name);
				printf("struct\tbuf\t%stab;\n", p->name);
			}
		}
	}
	puke(stre1);
	for(i=0; q=btab[i]; i++) {
	    if(!equal(q, "dummy"))
		for(p=table; p->name; p++)
		if(match(q, p->name) &&
		   (p->key&BLOCK) && p->count) {
			if (p->coded)
				printf("%s", p->coded);
			else {
				printf("\t%sopen, ", p->name);
				printf("%sclose, ", p->name);
				printf("%sstrategy, &%stab,", p->name, p->name);
			}
			printf("\t/* %s = %d */\n",
				p->name, i);
			if(p->key & ROOT)
				rootmaj = i;
			if (p->key & SWAP)
				swapmaj = i;
			if (p->key & PIPE)
				pipemaj = i;
			goto newb;
		}
		printf("	nodev, nodev, nodev, 0, /* %s = %d */\n", q, i);
	newb:;
	}
	for(p=table; p->name; p++)
		if((p->key&ERRLOG) && (p->key&CHAR) && p->count) {
			for(i=0; q=ctab[i]; i++)
				if(match(q, p->name)) {
					elmaj = i;
					break;
					}
				}
	if(rootmaj < 0) {
		fprintf(stderr, "No root device given\n");
		exit(1);
	}
	if (swapmaj < 0) {
		fprintf(stderr, "No swap device given\n");
		exit(1);
	}
	if (pipemaj < 0) {
		fprintf(stderr, "No pipe device given\n");
		exit(1);
	}
	if (elmaj < 0) {
		fprintf(stderr, "No error log device given\n");
		exit(1);
	}
	puke(strf);
	for (i=0; q=ctab[i]; i++) {
		for (p=table; p->name; p++)
		if (match(q, p->name) && (p->key&CHAR) && p->count) {
			if (p->codeg) {
				if (*p->codeg)
					printf("%s\n", p->codeg);
			} else {
				printf("int\t");
				if ((p->key&BLOCK) == 0)
					printf("%sopen(), %sclose(), ",
						p->name, p->name);
				printf("%sread(), %swrite()", p->name, p->name);
				if ((p->key&(BLOCK|TAPE)) != BLOCK)
					printf(", %sioctl()", p->name);
				printf(";\n");
			}
		}
	}
	ntty = 0;	/* total number of tty lines */
	for(p=table; p->name; p++) {
/*
 * The "kl"/"dl" code will break
 * if kl is not ahead of dl in table.
 */
		if(equal(p->name, "kl")) {
			printf("\nint\tnkl11 = %d;", p->count + 1);
			numkl = p->count + 1;
		}
		if(equal(p->name, "dl")) {
			printf("\nint\tndl11 = %d;", p->count);
			printf("\nstruct\ttty\t*kl11[%d+%d];\n",
				numkl, p->count);
			ntty += (numkl + p->count);
		}
		if(equal(p->name, "dh") && p->count) {
			ntty += (p->count*16);
			printf("\nstruct\ttty\t*dh11[%d];", p->count*16);
			printf("\nint\tdh_local[%d];", p->count);
			printf("\nchar\tdhcc[%d];", p->count*16);
			printf("\nint\tdhchars[%d];", p->count);
			printf("\nint\tdhsar[%d];", p->count);
			printf("\nint	ndh11 = %d;\n", p->count*16);
			}
		if(equal(p->name, "uh") && p->count) {
			ntty += (p->count*16);
			printf("\nstruct\ttty\t*uh11[%d];", p->count*16);
			printf("\nint\tuh_local[%d];", p->count);
			printf("\nchar\tuhcc[%d];", p->count*16);
			printf("\nint\tuhchars[%d];", p->count);
			printf("\nint	nuh11 = %d;", p->count*16);
			printf("\nchar	uh_shft = %d;", 4);
			printf("\nchar	uh_mask = 0%o;\n", 15);
			}
		if(equal(p->name, "uhv") && p->count) {
			ntty += (p->count*8);
			printf("\nstruct\ttty\t*uh11[%d];", p->count*8);
			printf("\nint\tuh_local[%d];", p->count);
			printf("\nchar\tuhcc[%d];", p->count*8);
			printf("\nint\tuhchars[%d];", p->count);
			printf("\nint	nuh11 = %d;", p->count*8);
			printf("\nchar	uh_shft = %d;", 3);
			printf("\nchar	uh_mask = %d;\n", 7);
			}
		if(equal(p->name, "dz") && p->count) {
			ntty += (p->count*8);
			printf("\nstruct\ttty\t*dz_tty[%d];", p->count*8);
			printf("\nchar\tdz_local[%d];", p->count);
			printf("\nchar\tdz_brk[%d];", p->count);
			printf("\nint	dz_cnt = %d;", p->count*8);
			printf("\nchar	dz_shft = %d;", 3);
			printf("\nchar	dz_mask = %d;\n", 7);
			}
		if(equal(p->name, "dzv") && p->count) {
			ntty += (p->count*4);
			printf("\nstruct\ttty\t*dz_tty[%d];", p->count*4);
			printf("\nchar\tdz_local[%d];", p->count);
			printf("\nchar\tdz_brk[%d];", p->count);
			printf("\nint	dz_cnt = %d;", p->count*4);
			printf("\nchar	dz_shft = %d;", 2);
			printf("\nchar	dz_mask = %d;\n", 3);
			}
		if(equal(p->name, "du") && p->count) {
			printf("\nstruct du {");
			printf("\n\tstruct dureg\t*du_addr;");
			printf("\n\tint\t\tdu_state;");
			printf("\n\tstruct proc\t*du_proc;");
			printf("\n\tstruct buf\t*du_buf;");
			printf("\n\tcaddr_t\t\tdu_bufb;");
			printf("\n\tcaddr_t\t\tdu_bufp;");
			printf("\n\tint\t\tdu_nxmit;");
			printf("\n\tint\t\tdu_timer;");
			printf("\n} du[%d];", p->count);
			printf("\nint\tndu = %d;\n", p->count);
		}
		if(equal(p->name, "u1") && p->count)
			printf("\nstruct\ttty\t*u1_tty[];");
		if(equal(p->name, "u2") && p->count)
			printf("\nstruct\ttty\t*u2_tty[];");
		if(equal(p->name, "u3") && p->count)
			printf("\nstruct\ttty\t*u3_tty[];");
		if(equal(p->name, "u4") && p->count)
			printf("\nstruct\ttty\t*u4_tty[];");
		}
	printf("\nint\tntty = %d;", ntty);
	printf("\nstruct\ttty\ttty_ts[%d];\n", ntty);
	if (npty >0) {
		printf("\nint\tnpty = %d;\n", npty);
		printf("struct\tpt_ioctl\tpt_ioctl[%d];\n", npty);
		printf("struct\ttty\tpty_ts[%d];\n", npty);
		printf("struct\ttty\t*pt_tty[] = {\n");
		for (i = 0; i < npty; i++)
			printf("\t&pty_ts[%d],\n", i);
		printf("};\n");
	}
	if (nnetattach) {
		printf("\nnetattach()\n{\n");
		for (i = 0; i < nnetattach; i++) {
			printf("\t%.2sattach(%d, 0%o, 0%o);\n",
				netattach[i].name,
				netattach[i].unit,
				netattach[i].csr,
				netattach[i].vec);
		}
		printf("}\n");
	}
	puke(strf1);
	for (i=0, q=ctab[0]; q; q = ctab[++i]) {
		if (!equal(q, "dummy")) {
			for (p=table; p->name; p++) {
				if (match(q, p->name) && (p->key&CHAR)
				    && p->count) {
					if (p->codee)
						printf("%s", p->codee);
					else if (p->key&TAPE) {
						printf("\t%sopen,   %sclose,  ",
							p->name, p->name);
						printf("%sread,   %swrite,\n\t",
							p->name, p->name);
						printf("%sioctl,    nulldev,  ",
							p->name);
						printf("0,        seltrue,");
					} else if (p->key&BLOCK){
						printf("\t%sopen,   nulldev,  ",
							p->name);
						printf("%sread,   %swrite,\n",
							p->name, p->name);
						printf("\tnodev,    nulldev,  ");
						printf("0,        seltrue,");
					}
					printf("\t/* %s = %d */\n", p->name, i);
					break;
				}
			}
			if (p->name)
				continue;
		}
		printf("\tnodev,    nodev,    nodev,    nodev,\n");
		printf("\tnodev,    nulldev,  0,        nodev,");
		printf("\t/* %s = %d */\n", q, i);
	}
	puke(strh);
	puke(strj);
	if(ov)
		elbsiz = 192;
	else
		elbsiz = 350;
	printf(strg, rootmaj, rootmin,
		swapmaj, swapmin,
		pipemaj, pipemin,
		elmaj, elmin,
		nldisp,
		swplo, nswap, elsb, elnb);
/*
 * NOTE: changed the way this works, see globl.c
	if(nbuf < 0)
		nbuf = 40;
	if(ninode < 0)
		if(ov)
			ninode = 100;
		else
			ninode = 200;
	if(nfile < 0)
		if(ov)
			nfile = 80;
		else
			nfile = 175;
	if(nmount < 0)
		if(ov)
			nmount = 5;
		else
			nmount = 8;
	if(maxuprc < 0)
		if(ov)
			maxuprc = 15;
		else
			maxuprc = 25;
	if(ncall < 0)
		ncall = 20;
	if(nproc < 0)
		if(ov)
			nproc = 75;
		else
			nproc = 150;
*/
	if (mapsize < 0)
		mapsize = 30 + (nproc/2);
/*
 * NOTE:
	if(ntext < 0)
		if(ov)
			ntext = 25;
		else
			ntext = 40;
	if(nclist < 0)
		if(ov)
			nclist = 60;
		else
			nclist = 110;
*/
	printf(strg1, ninode, nmount);
	printf(strg2, mapsize, mapsize, nclist, msgbufs);
	printf(strg3);
	printf(strg4, ninode,
		nmount, mapsize, ncall,
		nclist, elbsiz, maxuprc,
		timezone, dstflag, ncargs, hz, msgbufs, maxseg, ulimit);
/*
	printf(strn);
	printf("\tlong\tbufcount[NBUF];\n");
	printf("} io_info = {NBUF};\n");
*/
/*
 * Number of drives and all structures
 * that change with the number of drives,
 * for most but not all disk drivers.
 */
	first = 0;
	for(p=table; p->name; p++) {
		if(equal("hp", p->name) ||
		   equal("hm", p->name) ||
		   equal("hj", p->name))
			continue;
		if(((p->key & (NUNIT|IOS|UTAB|BADS)) == 0) || (p->count == 0))
			continue;
		if(p->csr == MSCPDEV)
			continue;	/* don't do this for MSCP disks */
		if(first == 0) {
			printf("\n/*\n * Number of disk drives and ");
			printf("\n * related structures.\n */\n");
			first++;
			}
		if(p->key & NUNIT)
			printf("\nint\tn%s\t= %d;", p->name, p->count);
		if(p->key & IOS)
			printf("\nstruct\tios\t%s_ios[%d];", p->name, p->count);
		if(p->key & UTAB) {
			printf("\nstruct\tbuf\t%sutab[%d];", p->name, p->count);
			}
		if(p->key & BADS) {
			printf("\n#include <sys/%sbad.h>", p->name);
			printf("\nstruct\t%sbad\t%s_bads[%d];",
				p->name, p->name, p->count);
		}
		}
		if(first != 0)
			printf("\n");
/*
 * Number of drives and all structures
 * that change with the number of drives,
 * for most but not all tape drivers.
 */
	first = 0;
	for(p=table; p->name; p++)
		if((p->key & TAPE) && (p->count != 0)) {
			if(equal("ts", p->name) || equal("tk", p->name))
				continue;
			if(first == 0) {
				printf("\n/*\n * Number of tape drives ");
				printf("\n * and related arrays.\n */\n");
				first++;
			}
			if(equal("tk", p->name))
			    printf("\nint\ttk_ivec = 0%o;", p->vec);
			else {
			    printf("\nint\tn%s\t= %d;", p->name, p->count);
			    printf("\nu_short\t%s_flags[%d];", p->name, p->count);
			    printf("\nchar\t%s_openf[%d];", p->name, p->count);
			    printf("\ndaddr_t\t%s_blkno[%d];", p->name, p->count);
			    printf("\ndaddr_t\t%s_nxrec[%d];", p->name, p->count);
			    printf("\nu_short\t%s_erreg[%d];", p->name, p->count);
			    printf("\nu_short\t%s_dsreg[%d];", p->name, p->count);
			    printf("\nshort\t%s_resid[%d];", p->name, p->count);
			    printf("\nstruct\tbuf\tc%sbuf[%d];", p->name, p->count);
			}
		}
		if(first != 0)
			printf("\n");
/*
 * I/O device CSR address table in c.c
 */

	puke(strm2);	/* MSCP/TS/TK CSR/VECTOR addresses */
	puke(strm);
	for(i=0; q=ctab[i]; i++) {
		if(match(q, "dummy") ||
			match(q, "tty") ||
			match(q, "ptc") ||
			match(q, "pts") ||
			match(q, "dj") ||
			match(q, "mem")) {
				printf("\n\t0,\t\t/* dummy\tCSR address */");
				continue;
			}
		for(p=table; p->name; p++)
			if(match(q, p->name)) {
				if(p->csr == 0)
				  fprintf(stderr, "\n%s bad csr %o\n",q,p->csr);
			/* rx2 now hx
				if(match(q, "rx"))
					q = "rx";
			 */
				if(match(q, "dz")) {
					q = "dz";
/*
 * If DZ not configured move to DZV,
 * in case DZV at non standard address.
 * DZ must preceed DZV in table.
 * If neither configured, don't matter because
 * both have same standard address.
 */
					if(p->count == 0)
						p++;
				}
				if(match(q, "uh")) {
					q = "uh";
/*
 * If UH not configured move to UHV,
 * in case UHV at non standard address.
 * UH must preceed UHV in table.
 * If neither configured, don't matter because
 * both have same standard address.
 */
					if(p->count == 0)
						p++;
				}
				if(equal(p->name, "dhdm"))
					continue;
				if(equal(p->name, "console"))
					ntab = "\t";
				else
					ntab = "\t\t";
				if(p->csr == MSCPDEV)	/* MSCP disk */
		printf("\n\t&ra_csr[0],\t/* %s%sCSR address */", q, ntab);
				else if(equal(p->name,"ts"))	/* TS magtape */
		printf("\n\t&ts_csr[0],\t/* %s%sCSR address */", q, ntab);
				else if(equal(p->name,"tk"))	/* TK magtape */
		printf("\n\t&tk_csr[0],\t/* %s%sCSR address */", q, ntab);
				else
		printf("\n\t%07.o,\t/* %s%sCSR address */", p->csr, q,ntab);
				break;
			}
	}
	for(p=table; p->name; p++)
		if(equal(p->name, "dhdm")) {
			printf("\n\t%07.o,\t/* dhdm\t\tCSR address */", p->csr);
			break;
			}
	for(p=table; p->name; p++)
		if(equal(p->name, "kl")) {
			printf("\n\t%07.o,\t/* kl\t\tCSR address */", p->csr);
			break;
			}
	for(p=table; p->name; p++)
		if(equal(p->name, "dl")) {
			printf("\n\t%07.o,\t/* dl\t\tCSR address */", p->csr);
			break;
			}
	printf("\n};\n");
	puke(strm1);
	if (network) {
		if (allocs < 0)
			allocs = ov ? 1000 : 2000;
		printf("\nint\tallocs[%d];\n", allocs);
		printf("int\tnwords = %d;\n", allocs);
		if (miosize < 0)
			miosize = 8192;
		/* round up to nearest click */
		miosize = (miosize + 077) & ~077;
		printf("int\tmiosize = %d;\n", miosize);
		printf("int\tmbsize = (%d*MSIZE)+%d;\n", mbufs, miosize);
	} else {
		printf("netintr()\t\t/* dummy network */\n");
		printf("{\n}\n");
		printf("int\tnetisr = 0;\t/* dummy network */\n");
		printf("int\tmbsize = 0;\t/* dummy network */\n");
	}
}

/*
 * dds.h header file for dds.c (MSCP & HP disk parameters)
 */

pass2_3()
{
	register struct mscptab *mp;
	register int i;
	register struct tab *p;
	int	nra, csr, vec;
	int	nrh, c0, c1, c2;
	int	rs, rsl2;

	if(freopen("dds.h", "w", stdout) == NULL) {
		fprintf(stderr, "\n Can't open `dds.h' file !\n");
		exit(1);
	}
	i = 0;
	for(mp=mstab; mp->ms_dcn; mp++)
		if(mp->ms_cn >= 0)
			i++;
	puke(strq);
	printf("\n#define\tNUDA\t%d", i);
	if(i == 0)
		goto no_uda;
	for(i=0; i<4; i++) {
	    csr = vec = nra = rs = rsl2 = 0;
	    for(mp=mstab; mp->ms_dcn; mp++)
		if(mp->ms_cn == i) {
		    nra = mp->ms_nra;
		    csr = mp->ms_csr;
		    vec = mp->ms_vec;
		    /* set mscp comm. ring size (# of packets) */
		    /* rs = # packets, rsl2 = ring size log2 */
		    if(strcmp("ru", mp->ms_dcn) == 0) {	/* RUX1 cntlr */
			rs = 2;
			rsl2 = 1;
		    } else if(ov) {	/* NSID CPU, all other cntlrs */
			rs = 4;
			rsl2 = 2;
		    } else {		/* SID CPU, all other cntlrs */
			rs = 8;
			rsl2 = 3;
		    }
		    break;
		}
	    printf("\n#define\tC%d_NRA\t%d", i, nra);
	    printf("\n#define\tC%d_CSR\t0%o", i, csr);
	    printf("\n#define\tC%d_VEC\t0%o", i, vec);
	    printf("\n#define\tC%d_RS\t%d", i, rs);
	    printf("\n#define\tC%d_RSL2\t%d", i, rsl2);
	}
no_uda:
	printf("\n");
	for(p=table, nrh=0; p->name; p++) {
		if(equal("hp", p->name)) {
			c0 = p->count;
			if(c0)
				nrh++;
		}
		if(equal("hm", p->name)) {
			c1 = p->count;
			if(c1)
				nrh++;
		}
		if(equal("hj", p->name)) {
			c2 = p->count;
			if(c2)
				nrh++;
		}
	}
	printf("\n\n#define\tNRH\t%d", nrh);
	if(nrh == 0)
		goto no_rh;
	printf("\n#define\tC0_NHP\t%d", c0);
	printf("\n#define\tC1_NHP\t%d", c1);
	printf("\n#define\tC2_NHP\t%d", c2);
	printf("\n#define\tC3_NHP\t0");
no_rh:
	printf("\n");
	i = -1;
	for(mp=tstab; mp->ms_dcn; mp++) {
		if(mp->ms_cn >= 0)
			i = mp->ms_cn;
	}
	i++;
	printf("\n#define\tNTS\t%d", i);
	if(i == 0)
		goto no_ts;
	i = 0;
	for(mp=tstab; mp->ms_dcn; mp++) {
	    if(mp->ms_cn < 0) {
	    	printf("\n#define\tS%d_CSR\t0", i);
	    	printf("\n#define\tS%d_VEC\t0", i);
	    } else {
	    	printf("\n#define\tS%d_CSR\t0%o", mp->ms_cn, mp->ms_csr);
	    	printf("\n#define\tS%d_VEC\t0%o", mp->ms_cn, mp->ms_vec);
	    }
	    i++;
	}
no_ts:
	printf("\n");
	i = -1;
	for(mp=tktab; mp->ms_dcn; mp++) {
		if(mp->ms_cn >= 0)
			i = mp->ms_cn;
	}
	i++;
	printf("\n#define\tNTK\t%d", i);
	if(i == 0)
		goto no_tk;
	i = 0;
	for(mp=tktab; mp->ms_dcn; mp++) {
	    if(mp->ms_cn < 0) {
	    	printf("\n#define\tK%d_CSR\t0", i);
	    	printf("\n#define\tK%d_VEC\t0", i);
	    } else {
	    	printf("\n#define\tK%d_CSR\t0%o", mp->ms_cn, mp->ms_csr);
	    	printf("\n#define\tK%d_VEC\t0%o", mp->ms_cn, mp->ms_vec);
	    }
	    i++;
	}
no_tk:
	printf("\n");
	for(p=table; p->name; p++) {
		if(equal("hk", p->name)) {
			i = p->count;
			break;
		}
	}
	printf("\n\n#define\tNHK\t%d", i);
	for(p=table; p->name; p++) {
		if(equal("rp", p->name)) {
			i = p->count;
			break;
		}
	}
	printf("\n#define\tNRP\t%d", i);
	printf("\n");
	printf("\n#define\tNPROC\t%d", nproc);
	printf("\n#define\tNTEXT\t%d", ntext);
	printf("\n#define\tNFILE\t%d", nfile);
	printf("\n#define\tNBUF\t%d", nbuf);
	printf("\n#define\tELBSIZ\t%d", elbsiz);
	printf("\n#define\tNCALL\t%d", ncall);
	printf("\n");
/*
 * NOTE: changed way defaults work, see globl.c
	if(flock) {
		if(flckrec < 0)
			flckrec =  200;
		if(flckfil < 0)
			flckfil = 50;
	}
	if(mesg) {
		if(msgmap < 0)
			msgmap = 100;
		if(msgmax < 0)
			msgmax = 8192;
		if(msgmnb < 0)
			msgmnb = 16384;
		if(msgmni < 0)
			msgmni = 10;
		if(msgssz < 0)
			msgssz = 8;
		if(msgtql < 0)
			msgtql = 40;
		if(msgseg < 0)
			msgseg = 1024;
	}
	if(sema) {
		if(semmap < 0)
			semmap = 10;
		if(semmni < 0)
			semmni = 10;
		if(semmns < 0)
			semmns = 60;
		if(semmnu < 0)
			semmnu = 30;
		if(semmsl < 0)
			semmsl = 25;
		if(semopm < 0)
			semopm = 10;
		if(semume < 0)
			semume = 10;
		if(semvmx < 0)
			semvmx = 32767;
		if(semaem < 0)
			semaem = 16384;
	}
 */
	if(shuffle)
		printf("\n#define\tSHUFFLE\t1");
	if(maus) 
		printf("\n#define\tMAUS\t1");
	if(flock) {
		printf("\n#define\tFLOCK\t1");
		printf("\n#define\tFLCKREC\t%d",flckrec);
		printf("\n#define\tFLCKFIL\t%d",flckfil);
	}
	if(mesg) {
		printf("\n#define\tMESG\t%d", mesg);
		printf("\n#define\tMSGMAP\t%d", msgmap);
		printf("\n#define\tMSGMAX\t%d", msgmax);
		printf("\n#define\tMSGMNB\t%d", msgmnb);
		printf("\n#define\tMSGMNI\t%d", msgmni);
		printf("\n#define\tMSGSSZ\t%d", msgssz);
		printf("\n#define\tMSGTQL\t%d", msgtql);
		printf("\n#define\tMSGSEG\t%d", msgseg);
	}
	if(sema) {
		printf("\n#define\tSEMA\t%d", sema);
		printf("\n#define\tSEMMAP\t%d", semmap);
		printf("\n#define\tSEMMNI\t%d", semmni);
		printf("\n#define\tSEMMNS\t%d", semmns);
		printf("\n#define\tSEMMNU\t%d", semmnu);
		printf("\n#define\tSEMMSL\t%d", semmsl);
		printf("\n#define\tSEMOPM\t%d", semopm);
		printf("\n#define\tSEMUME\t%d", semume);
		printf("\n#define\tSEMVMX\t%d", semvmx);
		printf("\n#define\tSEMAEM\t%d", semaem);
	}
	if (nnetattach) {
		int j,n_de,n_qe;
		n_de = n_qe = 0;
		for (j = 0; j < nnetattach; j++) {
			if(!(strncmp(netattach[j].name,"qe",2)))
				n_qe++;
			if(!(strncmp(netattach[j].name,"de",2)))
				n_de++;
		}
		if(n_de) printf("\n#define\tNDE\t%d",n_de);
		if(n_qe) printf("\n#define\tNQE\t%d",n_qe);
	}
	printf("\n");
}

/*
 * mch.h header file for machine language assist
 */

pass2_5()
{
	register struct mscptab *mp;
	register int i;
	int	tape, disk, dmp_csr;
	char	*p;

	if(freopen("mch.h", "w", stdout) == NULL) {
		fprintf(stderr, "\nCan't open `mch.h' file !\n");
		exit(1);
	}
	tape = disk = 0;
	dumpht = dumpts = dumptm = dumprl = dumpra = dumprx = 0;
	dumphk = dumprp = dumphp = dumptk = 0;
	switch(dump) {
	case HT:
		p = "ht";
		tape++;
		dumpht++;
		break;
	case TS:
		p = "ts";
		tape++;
		dumpts++;
		break;
	case TM:
		p = "tm";
		tape++;
		dumptm++;
		break;
	case TK:
		p = "tk";
		tape++;
		dumptk++;
		break;
	case RL:
		p = "rl";
		disk++;
		dumprl++;
		break;
	case RX:
		p = "rq";
		disk++;
		dumprx++;
		dumpra++;
		break;
	case RA:
		p = "ra";
		disk++;
		dumpra++;
		break;
	case RD:
		p = "rq";
		disk++;
		dumpra++;
		break;
	case RC:
		p = "rc";
		disk++;
		dumpra++;
		break;
	case HK:
		p = "hk";
		disk++;
		dumphk++;
		break;
	case RP:
		p = "rp";
		disk++;
		dumprp++;
		break;
	case HP:
		p = "hp";
		disk++;
		dumphp++;
		break;
	default:
		fprintf(stderr, "\nNO crash dump: device not specified!\n");
		break;
	}
	if(disk && ((dumpdn < 0) || (dumplo < 0) || (dumphi < 0)))
		fprintf(stderr, "\n\7\7\7NO crash dump: ");
	if(disk && (dumpdn < 0)) {
		fprintf(stderr, "dumpdn missing!\n");
		dump = 0;
	}
	if(disk && (dumplo < 0)) {
		fprintf(stderr, "dumplo missing!\n");
		dump = 0;
	}
	if(disk && (dumphi < 0)) {
		fprintf(stderr, "dumphi missing!\n");
		dump = 0;
	}
	if(dump == 0) {
		disk = 0;
		goto no_dump;
	}
	dmp_csr = 0;
	if(tape || (disk && !dumpra)) {
		if (dumpts)
			dmp_csr = tstab[0].ms_csr;
		else if (dumptk)
			dmp_csr = tktab[0].ms_csr;
		else {
			for(i=0; table[i].name; i++)
				if(equal(table[i].name, p) && table[i].count)
					dmp_csr = table[i].csr;
		}
	}
	if(disk & dumpra) {
	    for (i=0; table[i].name; i++)
		if (equal(table[i].name, "ra") && table[i].count) {
			for(mp=mstab; mp->ms_dcn; mp++)
				if(equal(mp->ms_dcn, p) && (mp->ms_cn == 0))
					dmp_csr = mp->ms_csr;
			break;
		}
	}
	if(dump && (dmp_csr == 0)) {
		fprintf(stderr, "\n\7\7\7NO crash dump: ");
		fprintf(stderr, "device not configured or not on first ");
		fprintf(stderr, "RH/MSCP controller!\n");
no_dump:
		dumpht = dumpts = dumptm = dumptk = 0;
		dumprl = dumpra = dumprx = 0;
		dumphk = dumprp = dumphp = 0;
	}
	printf("\n#define HTDUMP %d", dumpht);
	printf("\n#define TSDUMP %d", dumpts);
	printf("\n#define TMDUMP %d", dumptm);
	printf("\n#define TKDUMP %d", dumptk);
	printf("\n#define RLDUMP %d", dumprl);
	printf("\n#define HKDUMP %d", dumphk);
	printf("\n#define RPDUMP %d", dumprp);
	printf("\n#define HPDUMP %d", dumphp);
	printf("\n#define RADUMP %d", dumpra);
	printf("\n#define RXDUMP %d\n", dumprx);
	if(dumpht)
		printf("\nHTCS1 = %o\n", dmp_csr);
	if(dumpts) {
		printf("\nTSDB = %o", dmp_csr);
		printf("\nTSSR = TSDB+2\n");
	}
	if(dumptm) {
		printf("\nMTS = %o", dmp_csr);
		printf("\nMTC = MTS+2\n");
	}
	if(dumptk) {
		printf("\nTKAIP = %o", dmp_csr);
		printf("\nTKASA = %o", dmp_csr+2);
	}
	if(dmp_csr && disk)
		printf("\nDSKCSR = %o\n", dmp_csr);
	for(i=0; btab[i]; i++) {
		if(strcmp(btab[i], p) == 0) 
			break;
		if(strcmp(btab[i],"ra") == 0)
			if( (strcmp(p,"rc") == 0) || (strcmp(p,"rq") == 0))
				break;
	}
	if(dumpht)
		printf("\nHT_BMAJ = %d\n", i);
	if(dumphp)
		printf("\nHP_BMAJ = %d\n", i);
	if(dumptk)
		printf("\nTK_BMAJ = %d\n", i);
	if(dumpra)
		printf("\nRA_BMAJ = %d\n", i);
	if(dmp_csr && disk) {
		printf("\ndumpdn\t= %d.", dumpdn);
		printf("\ndumplo\t= %D.", dumplo);
		printf("\ndumphi\t= %D.\n", dumphi);
	}
	if(!nfp)
		printf("\n#define FPP\t1\n");
}


/*
 * pass3 - create the devmaj.h major device number file
 */

pass3()
{
	register struct tab *p;
	register char *q;
	register int i;
	char	*c;
	char	trash[10];
	int elerr = 0;
	struct	mscptab	*mp;
#define	DEVMAJ	"/usr/include/sys/devmaj.h"
#define	TDEVMAJ	"devmaj.tmp"

	if (freopen(TDEVMAJ, "w", stdout) == NULL) {
		fprintf(stderr, "\nCan't open `devmaj.h' file !\n");
		exit(1);
	}
	puke(strl1);
	for(i=0; q=btab[i]; i++) {
		if(equal(q, "dummy"))
			continue;
		c = &trash[0];
		*c++ = (*q++ & ~040);	/* upper case */
		if((*q >= '0') && (*q <= '9'))
			*c++ = *q++;
		else
			*c++ = (*q++ & ~040);
		*c = 0;
		c = &trash[0];
		printf("#define\t%s_BMAJ\t%d\n", c, i);
		}
	puke(strl2);
	for(i=0; q=ctab[i]; i++) {
		if(equal(q, "dummy"))
			continue;
		c = &trash[0];
		while(*q && (*q != '|')) {
			if((*q >= '0') && (*q <= '9'))
				*c++ = *q++;
			else
				*c++ = (*q++ & ~040);
		}
		/* special case for ptc and pts */
		if (trash[0] == 'P' && trash[1] == 'T')
			trash[3] = 0;
		else
			trash[2] = 0;
		c = &trash[0];
		printf("#define\t%s_RMAJ\t%d\n", c, i);
		}
	printf("#define\tDM_RMAJ\t%d\n", i++);
	printf("#define\tKL_RMAJ\t%d\n", i++);
	printf("#define\tDL_RMAJ\t%d\n", i++);
	{
		FILE *f1,*f2;
		register int c1, c2;
		if ((f1 = fopen(DEVMAJ, "r")) == NULL) {
			fprintf(stderr, "Can't open %s\n", DEVMAJ);
			exit(1);
		}
		if ((f2 = fopen(TDEVMAJ, "r")) == NULL) {
			fprintf(stderr, "Can't re-open %s\n", TDEVMAJ);
			exit(1);
		}
		fflush(stdout);	/* make sure the whole thing is written out */
		do {
			c1 = getc(f1);
			c2 = getc(f2);
		} while ((c1 == c2) && (c1 != EOF));
		fclose(f1);
		if (c1 != c2) {
			if(freopen(DEVMAJ, "w", stdout) == NULL) {
				fprintf(stderr, "\nCan't open `%s' file !\n",
								DEVMAJ);
				exit(1);
			}
			rewind(f2);
			while ((c1 = getc(f2)) != EOF)
				putc((char)c1, stdout);
		}
		fclose(f2);
		unlink(TDEVMAJ);
	}
/*
 * Print a list of the configured devices
 * with their addresses and vectors.
 */

	fprintf(stderr, "\nDevice\tAddress\tVector\tunits\n\n");
	for(p=table; p->name; p++) {
		if(p->count == 0)
			continue;
		if(equal(p->name, "mem") ||
			equal(p->name, "parity") ||
			equal(p->name, "ptc") ||
			equal(p->name, "pts") ||
			equal(p->name, "tty"))
				continue;
		if(equal(p->name, "clock")) {
			fprintf(stderr, "kw11-l\t177456\t100\n");
			fprintf(stderr, "kw11-p\t172540\t104\n");
			continue;
			}
/*
 * Don't need to print count in front of name,
 * it is printed under units.
 *
		if((p->count > 1) && (p->key&NUNIT) == 0 && (p->key&TAPE) == 0)
			fprintf(stderr, "%d", p->count);
 */
		if(p->csr == MSCPDEV) {
			for(mp=mstab; mp->ms_dcn; mp++) {
				if(mp->ms_cn < 0)
					continue;
				fprintf(stderr, "%s\t%o\t%3.o\t%d",
				  mp->ms_dcn,mp->ms_csr,mp->ms_vec,mp->ms_nra);
				if((mp->ms_cn == 0) && dumpra)
					fprintf(stderr, "\t(crash dump device)");
				fprintf(stderr, "\n");
			}
		} else if(equal(p->name,"ts")) {
			for(mp=tstab; mp->ms_dcn; mp++) {
				if(mp->ms_cn < 0)
					continue;
				fprintf(stderr, "%s\t%o\t%3.o\t%d",
				  mp->ms_dcn,mp->ms_csr,mp->ms_vec,mp->ms_nra);
				if(dumpts && mp->ms_cn == 0)
					fprintf(stderr, "\t(crash dump device)");
				fprintf(stderr, "\n");
			}
		} else if(equal(p->name,"tk")) {
			for(mp=tktab; mp->ms_dcn; mp++) {
				if(mp->ms_cn < 0)
					continue;
				fprintf(stderr, "%s\t%o\t%3.o\t%d",
				  mp->ms_dcn,mp->ms_csr,mp->ms_vec,mp->ms_nra);
				if(dumptk && mp->ms_cn == 0)
					fprintf(stderr, "\t(crash dump device)");
				fprintf(stderr, "\n");
			}
		} else {
			fprintf(stderr, "%s\t%o\t", p->name, p->csr);
			fprintf(stderr, "%3.o", p->vec);
			if(p->count > 0)
				fprintf(stderr, "\t%d", p->count);
			if((equal(p->name, "tm") && dumptm)
				|| (equal(p->name, "ht") && dumpht)
				|| (equal(p->name, "rl") && dumprl)
				|| (equal(p->name, "hk") && dumphk)
				|| (equal(p->name, "rp") && dumprp)
				|| (equal(p->name, "hp") && dumphp))
					fprintf(stderr, "\t(crash dump device)");
			fprintf(stderr, "\n");
		}
	}
/*
 * Print the layout of the root, swap, pipe, & error log areas.
 */
	fprintf(stderr, "\nFilsys\tDevice\tmaj/min\tstart\tlength\n");
	for(p=table; p->name; p++) {
		if(p->key & ROOT)
			fprintf(stderr, "\nroot\t%s\t%3.d/%d",
				p->name, rootmaj, rootmin);
		if(p->key & PIPE)
			fprintf(stderr, "\npipe\t%s\t%3.d/%d",
				p->name, pipemaj, pipemin);
		if(p->key & SWAP)
			fprintf(stderr, "\nswap\t%s\t%3.d/%d\t%D\t%d",
			    p->name, swapmaj, swapmin, swplo, nswap);
		if(p->key &ERRLOG)
			fprintf(stderr, "\nerrlog\t%s\t%3.d/%d\t%D\t%d",
				p->name, elmaj, elmin, elsb, elnb);
		}
	fprintf(stderr, "\n");
/*
 * Print configuration warnings if any.
 */
/* NO LONGER USED - sysgen makes this unnecessary
	for(p=table; p->name; p++)
		if((p->key&ROOT) && (p->key&ERRLOG) && (rootmin == elmin)) {
			fprintf(stderr, "\7\7\7WARNING, root & error log on ");
			fprintf(stderr, "%s %d", p->name, rootmin);
			fprintf(stderr, " watchout for overlap !\n");
			break;
			}
	for(p=table; p->name; p++)
		if((p->key&ROOT) && (p->key&SWAP) && (rootmin == swapmin)) {
			fprintf(stderr, "\7\7\7WARNING, root & swap on ");
			fprintf(stderr, "%s %d", p->name, rootmin);
			fprintf(stderr, " watchout for overlap !\n");
			break;
			}
	for(p=table; p->name; p++)
		if((p->key&SWAP) && (p->key&ERRLOG) && (swapmin == elmin)) {
			if((elsb >= swplo) && (elsb < (swplo + nswap)))
				elerr++;
			if(((elsb+elnb)>swplo)&&((elsb+elnb)<(swplo+nswap)))
				elerr++;
			break;
			}
	if(elerr)
		fprintf(stderr, "\7\7\7WARNING, swap & error log overlap !\n");
	if(sav && nsav)
	   fprintf(stderr, "\7\7\7WARNING, floating & fixed vectors mixed !\n");
 */
}


/*
 * pass4 - create "ovload" file
 *
 * "ovload" is a shell file used by /sys/conf/makefile
 * to invoke the covld command to link the overlay
 * kernel and produce the executable file unix_ov or unix_id.
 *
 * The method to this madness is as follows:
 *
 * 1.	Set the base overlay table pointer to ovt (430)
 *	or sovt (431) depending on "ov".
 *
 * 2.	Use the size command to create the file text.sizes
 *	containing the text sizes of each object module.
 *	../ovdev/bio.o 2086+30+0 = 2116b = 04104b, etc.
 *
 * 3.	Transfer the text size of each object module from the
 *	text.sizes file to that module's slot in the ovtab structure.
 *
 * 4.	Check each module in the ovtab structure to insure that it's
 *	size was found and initialize the ovdes (overlay descriptor)
 *	array to all zeroes (all overlays empty).
 *
 * 5.	Scan the ovtab structure and load all of the modules that
 *	are always configured into their designated overlays.
 *
 * 6.	Search the mkconf "table" to find all configured devices
 *	(count > 0) and mark them as optional configured devices in
 *	the ovtab structure, one exception is "tty" which always included.
 *	If the "hp", "hk", or "hm" disks are configured,
 *	then the "dsort" module must be included
 *	in overlay 3.
 *	If "ra" or "rl" is configured, dsort.o must also be loaded.
 *	If the "dh" driver is configured and the "dhdm" module
 *	is not included, then the "dhfdm" module must be included
 *	in overlay 5.
 * ***  VOID !
 *	If the "rx" specification is included, it's name is changed
 *	to "rx2" because the rx2.o module supports both rx01 & rx02.
 * ***  above two lines void, rx now hx !!!!!
 *
 * *** VOID !
 * 7.	Check for "mpx" and "pack", if specified mark their object
 *	modules as optional configured devices in the ovtab structure.
 *	The packet driver needs the function sdata() from mx2.o !
 *	We also find and mark the floating point simulation module
 *	if it was configured in.
 * ***
 *
 * 8.	Scan the ovtab and load all optionaly configured modules
 *	that have fixed overlay assignments into their designated overlays.
 *	Except for the following special case any module with a
 *	fixed overlay assignment, which causes that overlay to
 *	overflow will cause a fatal error, i.e., overlay
 *	3 could overflow if all three big disk drivers (hp, hm, & hk)
 *	are configured. If this occurs the "hm" driver will be
 *	loaded into the first overlay with sufficient free space.
 *
 * 9.	Scan the ovtab and transfer any configured modules from
 *	pseudo overlays 8 thru 10 to real overlays 1 thru 7.
 *	The modules are placed into the overlays starting at one
 *	on a first fit basis. Actually this is done in two passes,
 *	first the existing overlays are scanned to locate a
 *	slot for the module, if the module will not fit into
 *	any existing overlay, then a new overlay is created and
 *	the module is loaded into it.
 *
 * 10.	Create the "ovload" shell file by scanning the overlay
 *	descriptor array and writing the pathnames of the
 *	modules to be loaded into the file.
 */

pass4()
{
	register struct tab *p;
	register struct	ovtab	*otp;
	struct	ovtab	*botp;
	struct	ovdes	*ovdp;
	struct	ovtab	*notp;
	char	*c;
	int i, n;
	int fi, ovcnt;
	char	buf[512];
	char	trash[50];

/* 1. */
	botp = ov ? ovt : sovt;	/* 0430 or 0431 kernel overlay table */
	notp = ov ? netovt : snetovt;
/* 2. */
	system(ov ? osizcmd : ssizcmd);
	if ((fi = fopen("text.sizes", "r")) == 0) {
		fprintf(stderr, "\nCan't open text.sizes file\n");
		exit(1);
	}
/* 3. */
	while (fgets(buf, 512, fi) != NULL) {
		ovcnt = sscanf(buf, "%[^\.]%s%[^+]%[^\n]",
					omn, trash, mtsize, trash);
		if (ovcnt != 4) {
			fprintf(stderr, "[%s]\n", buf);
			fprintf(stderr, "text.sizes file format error\n");
			continue;
		}
		for(otp = botp; otp->mn; otp++) {
			if(equal(otp->mn, omn)) {
				otp->mts = atoi(mtsize);
				break;
			}
		}
		if (otp->mn)
			continue;
		if (!ov) {
			for(otp = &ssovt; otp->mn; otp++) {
				if(equal(otp->mn, omn)) {
					otp->mts = atoi(mtsize);
					break;
				}
			}
			if (otp->mn)
				continue;
		}
		if (network)
			for (otp = notp; otp->mn; otp++) {
				if (equal(otp->mn, omn)) {
					otp->mts = atoi(mtsize);
					break;
				}
			}
	}
	fclose(fi);
/* 4. */
	for(i=0; i<16; i++) {
		ovdtab[i].nentry = 0;
		ovdtab[i].size = 0;
	}
/* 5. */
/* 6. */
	for(p=table; p->name; p++) {
		if(p->count <= 0)
			continue;
		if(equal(p->name, "hm") || equal(p->name, "hj"))
			continue;
		if(equal(p->name, "dl"))
			continue;
		if(!ov && equal(p->name, "kl"))
			continue;
		if(equal(p->name, "dzv"))
			c = "dz";
		else if(equal(p->name, "uhv"))
			c = "uh";
		else if(equal(p->name, "ptc") || equal(p->name, "pts"))
			c = "pty";
		else
			c = p->name;
		for(otp=botp; otp->mn; otp++)
			if(equal(c, otp->mn)) {
				if(otp->mc == 0)
					otp->mc = 2;
				break;
			}
		if (!otp->mn) {
			fprintf(stderr, "\n%s not in overlay table\n", c);
			exit(1);
		}
	}

	/*
	 * If hp, hk, ra, or rl configured, must include dsort
	 */

	if (ov) {
		for (otp=botp; otp->mn; otp++) {
			if ((otp->mc == 2) &&
			    (equal(otp->mn, "hp") || equal(otp->mn, "hk") ||
			     equal(otp->mn, "ra") || equal(otp->mn, "rl")))
				break;
		}
		if (otp->mn)
			for (otp=botp; otp->mn; otp++)
				if (equal(otp->mn, "dsort")) {
					otp->mc = 2;
					break;
				}
	}

	/*
	 * If dh configured without dhdm, must include dhfdm
	 */

	for (otp=botp; otp->mn; otp++)
		if (equal(otp->mn, "dh"))
			break;
	if (otp->mc != 0) {
		for (otp=botp; otp->mn; otp++)
			if (equal(otp->mn, "dhdm"))
				break;
		if (otp->mc == 0) {
			for (otp=botp; otp->mn; otp++)
				if (equal(otp->mn, "dhfdm")) {
					otp->mc = 2;
					break;
				}
		}
	}

	/*
	 * optional sys modules
	 */
	for (otp = ov ? botp : ssovt; otp->mn; otp++) {
		if ((kfpsim && strcmp("fpsim", otp->mn) == 0) ||
		    (mesg && strcmp("msg", otp->mn) == 0) ||
		    (ipc && strcmp("ipc", otp->mn) == 0) ||
		    (sema && strcmp("sem", otp->mn) == 0) ||
		    (maus && strcmp("maus", otp->mn) == 0) ||
		    (flock && strcmp("flock", otp->mn) == 0) ||
		    (shuffle && strcmp("shuffle", otp->mn) == 0)) {
			otp->mc = 3;
		}
	}
/* 7. */
	if(ov) {
		for (otp=botp; otp->mn; otp++) {
			if ((kfpsim && strcmp("fpsim", otp->mn) == 0) ||
			    (!network && strcmp("fakenet", otp->mn) == 0) ||
			    (ubmap && strcmp("ubmap", otp->mn) == 0))
				otp->mc = 3;
		}
	}
/* 8. */
	if (checksizes(botp) ||
	    (!ov && checksizes(ssovt)) ||
	    (network && checksizes(notp)))
		exit(1);
/* 9. */
	if (ov) {
		loadup(botp, PASS0);
		loadup(botp, PASS1);
	}
	if (network)
		loadup(notp, PASS1);
	loadup(botp, PASS2);
	loadup(ssovt, PASS2);
	if (network)
		loadup(notp, PASS2);
/* 10. */
	if(freopen("ovload", "w", stdout) == NULL) {
		fprintf(stderr, "\nCan't open `ovload' file !\n");
		exit(1);
	}
	if(!ov) {
		printf("(cd ../dev; ar x LIB2_id");
		for(otp=botp; otp->mn; otp++)
			if (otp->mc && strncmp(otp->mn, "if_", 3))
				printf(" %s.o", otp->mn);
		printf(")\n");
	}
	if(ov)
		puke(strovh);
	else
		puke(strovh1);
	for(i=1; i<16; i++) {
		ovdp = &ovdtab[i];
		if(ovdp->nentry) {
			puke(strovz);
			for (n = 0; n < ovdp->nentry; n++)
				printf("%s\n", ovdp->omns[n]);
		}
	}
	if(ov)
		puke(strovl);
	else
		puke(strovl1);
	if(!ov) {
		printf("status=$?\nrm -f");
		for (otp=botp; otp->mn; otp++)
			if (otp->mc && strncmp(otp->mn, "if_", 3))
				printf("%s\n", otp->mpn);
		printf("\nexit ${status}\n");
	}
	fclose(stdout);
	system(cmcmd);
	exit(0);
}
/*
 * Load an array of modules into overlays.
 * Pass 1 loads specific modules into specific overlays. If it can't
 * fit them all, then it quits.
 * Pass 2 also loads specific modules into specific overlays. However,
 * if they don't fit, it will mark them to be put in in Pass 3, and
 * maybe print a warning.
 * Pass 3 loads anything that is left into any overlays that they will
 * fit into. If it can't fit it in, then it quits.
 */
loadup(otp, pass)
register struct ovtab *otp;
int pass;
{
	register int i,n;

	switch (pass) {
	case PASS0:
		for (; otp->mn; otp++)
			if ((otp->mc == 1) && (otp->ovno < 16) &&
			    !ovload(otp, otp->ovno)) {
				fprintf(stderr, "\noverlay %d size overflow\n",
								otp->ovno);
				exit(1);
			}
		break;
	case PASS1:
		for (; otp->mn; otp++)
			if ((otp->mc > 1) && (otp->ovno < 16)
			    && !ovload(otp, otp->ovno)) {
/*
 * REMOVED: 1/21/86 -- Fred Canter
				if (otp->mc != 3)
				    fprintf(stderr,
					"Warning: %s won't fit in overlay %d\n",
					otp->mn, otp->ovno);
*/
				otp->ovno = 16;
			}
		break;
	case PASS2:
		for (; otp->mn; otp++) {
			if (otp->ovno < 16 || otp->mc == 0)
				continue;
			for (n = 0, i = 1; i < 16; i++) {
				if (ovdtab[i].size == 0) {
					if (n == 0) /* remember empty overlay */
						n = i;
				} else if (ovload(otp, i))
					break;
			}
			if (i < 16)	/* did it get loaded? */
				continue;
			if (n && ovload(otp, n))
				continue;
			fprintf(stderr,
				"%s will not fit in overlay %d\n", otp->mn, n);
			exit(1);
		}
		break;
	}
}

checksizes(otp)
register struct ovtab *otp;
{
	register int fatal;

	for (fatal = 0; otp->mn; otp++) {
	    if (otp->mts < 0) {
		fprintf(stderr, "\n%s.o object file size not found", otp->mn);
		if (otp->mc > 0) {
		    fprintf(stderr, ": FATAL\n");
		    fatal++;
		} else
		    fprintf(stderr, ": (warning)\n");
	    }
	}
	return(fatal);
}
