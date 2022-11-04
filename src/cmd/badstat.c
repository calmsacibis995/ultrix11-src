
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 Online Bad Sector status program
 *
 * Jerry Brenner 1/10/83
 * Fred Canter (modified for HP general MASSBUS disk driver)
 *
 */

static char Sccsid[] = "@(#)badstat.c 3.0 4/21/86";
#define DKBAD

#include <sys/param.h>
#include <time.h>
#include <sys/timeb.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/hp_info.h>
#include <sys/bads.h>
#include <sys/hkbad.h>
#include <sys/hpbad.h>

/*
 *	BAD144 info for disk bad blocking. A zero entry in
 *	di_size indicates that disk type has no bad blocking.
 *	Borrowed from stand-alone DSKINIT, not all info used here.
 */

#define	NP	-1
#define	HP	1
#define	HM	2
#define	HJ	3

struct dkinfo {
	char	*di_type;	/* type name of disk */
	int	di_flag;	/* prtdsk() flags */
	char	*di_name;	/* ULTRIX-11 disk name */
	long	di_size;	/* size of entire volume in blocks */
	int	di_nsect;	/* sectors per track */
	int	di_ntrak;	/* tracks per cylinder */
	int	di_wcpat[2];	/* worst case pattern */
} dkinfo[] = {
	"rk06",	0,	"hk",	22L*3L*411L,	22, 3, 0135143, 072307,
	"rk07",	0,	"hk",	22L*3L*815L,	22, 3, 0135143, 072307,
	"rm02",	NP,	"hp",	32L*5L*823L,	32, 5, 0165555, 0133333,
	"rm02_0", HP,	"hp",	32L*5L*823L,	32, 5, 0165555, 0133333,
	"rm02_1", HM,	"hm",	32L*5L*823L,	32, 5, 0165555, 0133333,
	"rm02_2", HJ,	"hj",	32L*5L*823L,	32, 5, 0165555, 0133333,
	"rm03",	NP,	"hp",	32L*5L*823L,	32, 5, 0165555, 0133333,
	"rm03_0", HP,	"hp",	32L*5L*823L,	32, 5, 0165555, 0133333,
	"rm03_1", HM,	"hm",	32L*5L*823L,	32, 5, 0165555, 0133333,
	"rm03_2", HJ,	"hj",	32L*5L*823L,	32, 5, 0165555, 0133333,
	"rm05",	NP,	"hp",	32L*19L*823L,	32, 19, 0165555, 0133333,
	"rm05_0", HP,	"hp",	32L*19L*823L,	32, 19, 0165555, 0133333,
	"rm05_1", HM,	"hm",	32L*19L*823L,	32, 19, 0165555, 0133333,
	"rm05_2", HJ,	"hj",	32L*19L*823L,	32, 19, 0165555, 0133333,
	"rp04",	NP,	"hp",	22L*19L*411L,	22, 19, 0165555, 0133333,
	"rp04_0", HP,	"hp",	22L*19L*411L,	22, 19, 0165555, 0133333,
	"rp04_1", HM,	"hm",	22L*19L*411L,	22, 19, 0165555, 0133333,
	"rm04_2", HJ,	"hj",	22L*19L*411L,	22, 19, 0165555, 0133333,
	"rp05",	NP,	"hp",	22L*19L*411L,	22, 19, 0165555, 0133333,
	"rp05_0", HP,	"hp",	22L*19L*411L,	22, 19, 0165555, 0133333,
	"rp05_1", HM,	"hm",	22L*19L*411L,	22, 19, 0165555, 0133333,
	"rp05_2", HJ,	"hj",	22L*19L*411L,	22, 19, 0165555, 0133333,
	"rp06",	NP,	"hp",	22L*19L*815L,	22, 19, 0165555, 0133333,
	"rp06_0", HP,	"hp",	22L*19L*815L,	22, 19, 0165555, 0133333,
	"rp06_1", HM,	"hm",	22L*19L*815L,	22, 19, 0165555, 0133333,
	"rp06_2", HJ,	"hj",	22L*19L*815L,	22, 19, 0165555, 0133333,
	0,
};

struct nlist nli[] =
{
	{ "_nhk" },
	{ "_nhp" },
	{ "_hp_inde" },
	{ "_hk_bads" },
	{ "_hp_bads" },
	{ "_hk_dt" },
	{ "_hp_dt" },
	{ "" },
};

char	*bdhdr[3] = {
"Bad			   Replace		     Revector",
"Block #   Cyl  Trk  Sec    Block #   Cyl  Trk  Sec   Count",
"----------------------------------------------------------"
};



struct dkinfo *dip;
struct bt_bad *bt;
long	bn, rbn, atob(), atol();
union {
	long	serl;		/* pack serial number as a long */
	int	seri[2];	/* serial number as two int's */
}dsk;

struct {
	int t_cn;
	int t_tn;
	int t_sn;
}da;
char *usage="badstat: usage  badstat [ifn] [interval] [corefile] [namelist]\n";
char *coref = "/dev/mem";
char *infile = "/unix";

char	hp_index[MAXRH];	/* where to find info in HP structures */
char	hpn[MAXRH];		/* number units per RH controller */
int nhk, nhp, nhm, nhj, mem, interval, fd;
struct hkbad *hk_bads[8];
int	hk_dt[8];
struct hpbad *hp_bads[8];
char	hp_dt[8];
struct hpbad *hm_bads[8];
char	hm_dt[8];
struct hpbad *hj_bads[8];
char	hj_dt[8];
struct dtype{
	int	dtyp;
	char	*dname;
}d_typ[]  = {
	0,	"rk06",
	02000,	"rk07",
	RP04,	"rp04",
	RP05,	"rp05",
	RP06,	"rp06",
	RM03,	"rm03",
	RM02,	"rm02",
	RM05,	"rm05",
	-1,	"",
};

char	dev[30];
char	hk_av, hp_av, hm_av, hj_av;

long timbuf;
char *ap, *timezone(), *ctime(), *asctime();
struct tm *localtime();
extern char *ctime();


	struct dkbad *bp;
	struct bt_bad *bt;
	int cnt, bcnt;


main(argc, argp)
int argc;
char *argp[];
{
	int cnt, cnt1;
	char *ap;

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
		lmem();
		display();
		if(interval)
			sleep(interval);
		else
			exit(0);
	}
}

initmem()
{
	int cnt, cnt1;

	nlist(infile, nli);
	if((mem = open(coref, 0)) <= 0){
		printf("badstat: FATAL : Could not open %s\n", coref);
		exit(1);
	}
	if(nli[0].n_value){	/* Check for nhk */
		lseek(mem, (long)nli[0].n_value, 0);
		read(mem, &nhk, sizeof(nhk));
	}
	if((nli[2].n_value) && (nli[1].n_value)){	/* Check for nhp */
		lseek(mem, (long)nli[2].n_value, 0);
		read(mem, &hp_index, MAXRH);
		lseek(mem, (long)nli[1].n_value, 0);
		read(mem, &hpn, MAXRH);
		nhp = hpn[0]; nhm = hpn[1]; nhj = hpn[2];
	}
	if(nhk == 0 && nhp == 0 && nhm == 0 && nhj == 0){
		printf("badstat: W : No bad sector devices on system\n");
		exit(0);
	}
	if(nhk){
		if(nli[3].n_value == 0)
			nhk = 0;
		else{

			for(cnt = 0; cnt < nhk; cnt++){
				for(cnt1 = 0; cnt1 < 8;cnt1++){
				  sprintf(dev, "/dev/hk%d%d", cnt,cnt1);
				  fd = open(dev, 0);
				  if(fd > 0){
					if(read(fd, &dev, 1) == 1)
						hk_av |= (1<<cnt);
					close(fd);
					break;
				  }
				}
			}
			for(cnt = 0; cnt < nhk; cnt++){
				if((hk_bads[cnt] = malloc(sizeof(struct dkbad)))
				    <= 0){
					printf("badstat: F : malloc failed");
					printf(" for hk %d\n", cnt);
					exit(1);
				}
			}
			lseek(mem, (long)nli[5].n_value, 0);
			for(cnt = 0; cnt < nhk; cnt++)
				read(mem, &hk_dt[cnt], sizeof(int));
		}
	}
	if(nhp){
		if(nli[4].n_value == 0)
			nhp = 0;
		else{

			for(cnt = 0; cnt < nhp; cnt++){
				for(cnt1 = 0; cnt1 < 8;cnt1++){
				  sprintf(dev, "/dev/hp%d%d", cnt,cnt1);
				  fd = open(dev, 0);
				  if(fd > 0){
					if(read(fd, &dev, 1) == 1)
						hp_av |= (1<<cnt);
					close(fd);
					break;
				  }
				}
			}

			for(cnt = 0; cnt < nhp; cnt++){
				if((hp_bads[cnt] = malloc(sizeof(struct dkbad)))
				    <= 0){
					printf("badstat: F : malloc failed");
					printf(" for hp %d\n", cnt);
					exit(1);
				}
			}
			lseek(mem, (long)nli[6].n_value + hp_index[0], 0);
			for(cnt = 0; cnt < nhp; cnt++)
				read(mem, &hp_dt[cnt], sizeof(char));
		}
	}
	if(nhm){
		if(nli[4].n_value == 0)
			nhm = 0;
		else{

			for(cnt = 0; cnt < nhm; cnt++){
				for(cnt1 = 0; cnt1 < 8;cnt1++){
				  sprintf(dev, "/dev/hm%d%d", cnt,cnt1);
				  fd = open(dev, 0);
				  if(fd > 0){
					if(read(fd, &dev, 1) == 1)
						hm_av |= (1<<cnt);
					close(fd);
					break;
				  }
				}
			}

			for(cnt = 0; cnt < nhm; cnt++){
				if((hm_bads[cnt] = malloc(sizeof(struct dkbad)))
				    <= 0){
					printf("badstat: F : malloc failed");
					printf(" for hm %d\n", cnt);
					exit(1);
				}
			}
			lseek(mem, (long)nli[6].n_value + hp_index[1], 0);
			for(cnt = 0; cnt < nhm; cnt++)
				read(mem, &hm_dt[cnt], sizeof(char));
		}
	}
	if(nhj){
		if(nli[4].n_value == 0)
			nhj = 0;
		else{

			for(cnt = 0; cnt < nhj; cnt++){
				for(cnt1 = 0; cnt1 < 8;cnt1++){
				  sprintf(dev, "/dev/hj%d%d", cnt,cnt1);
				  fd = open(dev, 0);
				  if(fd > 0){
					if(read(fd, &dev, 1) == 1)
						hj_av |= (1<<cnt);
					close(fd);
					break;
				  }
				}
			}

			for(cnt = 0; cnt < nhj; cnt++){
				if((hj_bads[cnt] = malloc(sizeof(struct dkbad)))
				    <= 0){
					printf("badstat: F : malloc failed");
					printf(" for hj %d\n", cnt);
					exit(1);
				}
			}
			lseek(mem, (long)nli[6].n_value + hp_index[2], 0);
			for(cnt = 0; cnt < nhj; cnt++)
				read(mem, &hj_dt[cnt], sizeof(char));
		}
	}
	if(hk_av == 0 && hp_av == 0 && hm_av == 0 && hj_av == 0){
		printf("badstat: F : No bad block info found\n");
		exit(0);
	}
}

lmem()
{
	int cnt, i;

	if(nhk){
		lseek(mem, (long)nli[3].n_value, 0);
		for(cnt = 0; cnt < nhk; cnt++){
			read(mem, hk_bads[cnt], sizeof(struct hkbad));
		}
	}
	if(nhp){
		i = hp_index[0] * sizeof(struct hpbad);
		lseek(mem, (long)nli[4].n_value + i, 0);
		for(cnt = 0; cnt < nhp; cnt++){
			read(mem, hp_bads[cnt], sizeof(struct hpbad));
		}
	}
	if(nhm){
		i = hp_index[1] * sizeof(struct hpbad);
		lseek(mem, (long)nli[4].n_value + i, 0);
		for(cnt = 0; cnt < nhm; cnt++){
			read(mem, hm_bads[cnt], sizeof(struct hpbad));
		}
	}
	if(nhj){
		i = hp_index[2] * sizeof(struct hpbad);
		lseek(mem, (long)nli[4].n_value + i, 0);
		for(cnt = 0; cnt < nhj; cnt++){
			read(mem, hj_bads[cnt], sizeof(struct hpbad));
		}
	}
}

display()
{

	printf("\n\tBad Block Status on ");
	date();
	printf("\n");
	if(hk_av){
		for(cnt = 0; cnt < nhk; cnt++){
			if(hk_av & (1<<cnt)){
				bp = hk_bads[cnt];
				bt = &bp->bt_badb;
				if((dip = drvtyp(hk_dt[cnt])) == 0)
					continue;
				prtdev(-1, cnt);
			}
		}
	}
	if(hp_av){
		for(cnt = 0; cnt < nhp; cnt++){
			if(hp_av & (1<<cnt)){
				bp = hp_bads[cnt];
				bt = &bp->bt_badb;
				if((dip = drvtyp(hp_dt[cnt])) == 0)
					continue;
				prtdev(0, cnt);
			}
		}
	}
	if(hm_av){
		for(cnt = 0; cnt < nhm; cnt++){
			if(hm_av & (1<<cnt)){
				bp = hm_bads[cnt];
				bt = &bp->bt_badb;
				if((dip = drvtyp(hm_dt[cnt])) == 0)
					continue;
				prtdev(1, cnt);
			}
		}
	}
	if(hj_av){
		for(cnt = 0; cnt < nhj; cnt++){
			if(hj_av & (1<<cnt)){
				bp = hj_bads[cnt];
				bt = &bp->bt_badb;
				if((dip = drvtyp(hj_dt[cnt])) == 0)
					continue;
				prtdev(2, cnt);
			}
		}
	}
}

prtdev(ctrl, unit)
{
	int cnt;

	if(ctrl >= 0)
		printf("RH %d ", ctrl);
	printf("%s Unit %d",dip->di_type, unit);
	if((bp->bt_csnh+bp->bt_csnl) == 0
	  || bp->bt_flag != 0
	  || bp->bt_mbz){
		printf("  NO VALID BAD SECTOR ");
		printf("INFORMATION\n\n");
		return;
	}
	dsk.seri[0] = bp->bt_csnh;
	dsk.seri[1] = bp->bt_csnl;
	printf("  Cartridge Serial #%D  Pack has ", dsk.serl);
	for(cnt=0, bt= &bp->bt_badb; cnt < dip->di_nsect ;cnt++, bt++){
		if(bt->bt_cyl==-1&&bt->bt_trksec==-1)
			break;
		if(bt->bt_cyl==0&&bt->bt_trksec==0)
			break;
	}
	if(cnt == 0){
		printf("NO bad sectors\n\n");
		return;
	}
	printf("%d bad sectors\n\n", cnt);
	bt = &bp->bt_badb;
	printf("%s\n%s\n%s\n",bdhdr[0], bdhdr[1]
		, bdhdr[2]);
	prtsec(bp, bt,  setvec());
	printf("%s\n\n",bdhdr[2]);
}

prtsec(bp, bt, vec)
struct dkbad *bp;
struct bt_bad *bt;
int	*vec;
{
	int cnt;

	for(cnt=0; cnt < dip->di_nsect ;cnt++, bt++){
		if(bt->bt_cyl==-1&&bt->bt_trksec==-1)
			break;
		if(bt->bt_cyl==0&&bt->bt_trksec==0)
			break;
		bn = atob(bt);
		printf("%7D  %4d  %3d",bn, bt->bt_cyl,bt->bt_trksec>>8);
		printf("  %3d",bt->bt_trksec&0377);
		btoa((rbn-1)-cnt);
		printf("    %7D",(rbn-1)-cnt);
		printf("  %4d  %3d  %3d " ,da.t_cn, da.t_tn, da.t_sn);
		if(vec)
			printf("   %u\n", *(vec++));
	}
}

btoa(bn)
long bn;
{
	da.t_cn = bn/(dip->di_ntrak*dip->di_nsect);
	da.t_sn = bn%(dip->di_ntrak*dip->di_nsect);
	da.t_tn = da.t_sn/dip->di_nsect;
	da.t_sn = da.t_sn%dip->di_nsect;
}

long atob(bt)
struct bt_bad *bt;
{
	long blkn;

	blkn = (long)bt->bt_cyl * (long)(dip->di_ntrak * dip->di_nsect);
	blkn += (long)((bt->bt_trksec >> 8) * dip->di_nsect);
	blkn += ((long)bt->bt_trksec & 0377);
	return(blkn);
}

drvtyp(type)
int type;
{
	register struct dkinfo *dp;
	register struct dtype *dt;

	for(dt = d_typ; dt->dtyp >= 0; dt++)
		if(dt->dtyp == type)
			break;
	for(dp=dkinfo; dp->di_type; dp++)
		if(strcmp(dp->di_type, dt->dname) == 0)
			break;
	if(dp->di_type == 0)
		return(0);
	rbn = dp->di_size - dp->di_nsect;
	return(dp);
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

setvec()
{
	if(strcmp(dip->di_name, "hk") == 0)
		return(&bp->kbt_vec);
	else if(strcmp(dip->di_name, "hp") == 0)
		return(&bp->pbt_vec);
	else
		return(0);
}
