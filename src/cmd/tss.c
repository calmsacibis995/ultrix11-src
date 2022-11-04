
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * UTLRIX-11 terminal structure show program (tss.c)
 * namelist is /unix
 * core is /dev/mem
 *
 * 	Fred Canter
 *
 */

static char Sccsid[] = "@(#)tss.c 3.0 4/22/86";
#include <sys/param.h>	/* Don't matter which one */
#include <sys/tty.h>
#include <sys/devmaj.h>
#include <stdio.h>
#include <a.out.h>
#include <sys/stat.h>

struct nlist nl[] =
{
	{ "_nkl11" },
	{ "_ndl11" },
	{ "_kl11" },
	{ "_ndh11" },
	{ "_dh11" },
	{ "_dz_cnt" },
	{ "_dz_tty" },
	{ "_ntty" },
	{ "_tty_ts" },
	{ "_nuh11" },
	{ "_uh11" },
	{ "" }
};

int	nkl11;
int	ndl11;
struct tty *kl11[32];
int	ndh11;
struct tty *dh11[128];
int	nuh11;
struct tty *uh11[128];
int	dz_cnt;
struct tty *dz_tty[128];
int	ntty;
int	tt_used[128];

char	*fcore = "/dev/mem";
char	*fnlist = "/unix";
char	*fttys = "/etc/ttys";
int	fc;
char	tsn[30];

/*
 * TTY special file name array,
 * major device = -1 ends array.
 */

struct {
	char	tsfn[16];	/* tty special file name `/dev/tty??' */
	int	tmaj;		/* tty major device number */
	int	tmin;		/* tty minor device number */
} tsf[128+1];

struct	stat	statb;

char	tt_dev[40];	/* TTY device name, i.e, DH DZ DL KL */
int	tt_sn;		/* TTY structure number */
int	tt_un;		/* TTY device unit number */
int	tt_ln;		/* TTY device line number */

main(argc, argv)
char	*argv[];
int	argc;
{
	register FILE *tc;
	register int i;
	register char *p;
	int	n, nttys;

	if(argc != 1) {
		printf("\nUsage: tss\n");
		exit(1);
	}
	if((fc = open(fcore, 0)) < 0) {
		printf("\nCan't open %s\n", fcore);
		exit(1);
	}
	nlist(fnlist, nl);
	if(nl[0].n_value == 0) {
		printf("\nCan't access namelist in /unix\n");
		exit(1);
	}
	lseek(fc, (long)nl[0].n_value, 0);
	read(fc, (char *)&nkl11, sizeof(nkl11));
	if(nkl11 == 0) {
		printf("\ntss: Configuration error, no console tty !\n");
		exit(1);
	}
	if(nl[1].n_value) {
		lseek(fc, (long)nl[1].n_value, 0);
		read(fc, (char *)&ndl11, sizeof(ndl11));
	}
	if(nl[3].n_value) {
		lseek(fc, (long)nl[3].n_value, 0);
		read(fc, (char *)&ndh11, sizeof(ndh11));
	}
	if(nl[9].n_value) {
		lseek(fc, (long)nl[9].n_value, 0);
		read(fc, (char *)&nuh11, sizeof(nuh11));
	}
	if(nl[5].n_value) {
		lseek(fc, (long)nl[5].n_value, 0);
		read(fc, (char *)&dz_cnt, sizeof(dz_cnt));
	}
	if(nl[7].n_value) {
		lseek(fc, (long)nl[7].n_value, 0);
		read(fc, (char *)&ntty, sizeof(ntty));
	} else {
		printf("\ntss: Configuration error, _ntty missing\n");
		exit(1);
	}
	if(nl[8].n_value == 0) {
		printf("\ntss: Configuration error, _tty_ts missing\n");
		exit(1);
	}
	lseek(fc, (long)nl[2].n_value, 0);
	read(fc, (char *)&kl11, (sizeof(int)*(nkl11+ndl11)));
	if(ndh11) {
		lseek(fc, (long)nl[4].n_value, 0);
		read(fc, (char *)&dh11, (sizeof(int)*ndh11));
	}
	if(nuh11) {
		lseek(fc, (long)nl[10].n_value, 0);
		read(fc, (char *)&uh11, (sizeof(int)*nuh11));
	}
	if(dz_cnt) {
		lseek(fc, (long)nl[6].n_value, 0);
		read(fc, (char *)&dz_tty, (sizeof(int)*dz_cnt));
	}
	if((tc = fopen(fttys, "r")) == NULL) {
		printf("\nCan't open %s\n", fttys);
		exit(1);
	}
	for(i=0; i<128; i++) {	/* get tty node names & maj/min device #'s */
		if((fgets(&tsn, 29, tc)) == NULL) {
			tsf[i].tmaj = -1;	/* ends tsf array */
			fclose(tc);
			break;
		}
		p = &tsn[0];
		while((*p != '\n') && (*p != 0))
			p++;
		*p = 0;
		sprintf(&tsf[i].tsfn, "/dev/%s", &tsn[2]);
		if(stat(tsf[i].tsfn, &statb) < 0) {
			i--;	/* special file does not exist */
			continue;
		}
		if((statb.st_mode & S_IFMT) != S_IFCHR) {
			i--;
			continue;
		}
		tsf[i].tmaj = major(statb.st_rdev);
		tsf[i].tmin = minor(statb.st_rdev);
	}
	printf("\n%3d KL  lines", nkl11);
	printf("\n%3d DL  lines", ndl11);
	printf("\n%3d DH  lines", ndh11);
	printf("\n%3d UH  lines", nuh11);
	printf("\n%3d DZ  lines", dz_cnt);
	printf("\n%3d TTY structures\n", ntty);
	printf("\nDevice\tUnit\tLine\tTTY\tSpecial");
	printf("\nName\tNumber\tNumber\tStruct\tFile\n");
	for(i=0; i<(nkl11+ndl11); i++) {
		if(kl11[i] == 0)
			continue;
		if(i < nkl11)
			printf("\nKL\t");
		else
			printf("\nDL\t");
		printf("%2d\t\t",(i < nkl11) ? i : (i - nkl11));
		printf("%2d\t",ttfind(kl11[i]));
		psfn(CO_RMAJ, i);
	}
	for(i=0; i<ndh11; i++) {
		if(dh11[i] == 0)
			continue;
		printf("\nDH\t");
		printf("%2d\t", i>>4);
		printf("%2d\t", i&017);
		printf("%2d\t", ttfind(dh11[i]));
		psfn(DH_RMAJ, i);
	}
	for(i=0; i<nuh11; i++) {
		if(uh11[i] == 0)
			continue;
		printf("\nUH\t");
		printf("%2d\t", i>>4);
		printf("%2d\t", i&017);
		printf("%2d\t", ttfind(uh11[i]));
		psfn(UH_RMAJ, i);
	}
	for(i=0; i<dz_cnt; i++) {
		if(dz_tty[i] == 0)
			continue;
		printf("\nDZ\t");
		printf("%2d\t", i>>3);
		printf("%2d\t", i&07);
		printf("%2d\t", ttfind(dz_tty[i]));
		psfn(DZ_RMAJ, i);
	}
	printf("\n");
	exit(0);
}

/*
 * Return the number of the TTY structure that
 * matches the tty pointer.
 */

ttfind(tp)
struct tty *tp;
{
	register int i;

	for(i=0; i<ntty; i++)
		if(tp == (nl[8].n_value + (sizeof(struct tty)*i)))
			return(i);
	return(-1);
}

/*
 * Search the tty special file name array and
 * find the node name that matches the maj/min device.
 * Then print that name or ? of no match is found.
 */

psfn(maj, min)
{
	register int i;

	for(i=0; i<128; i++) {
		if((tsf[i].tmaj == maj) && (tsf[i].tmin == min)) {
			printf("%s", tsf[i].tsfn);
			return;
		}
	}
	printf("?");
}
