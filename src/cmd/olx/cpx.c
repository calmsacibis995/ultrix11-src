
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)cpx.c	3.0	4/22/86";
/*
 * ULTRIX-11 CPU exerciser program (cpx).
 *
 * Fred Canter 10/3/82
 * Bill Burns 4/84
 *	fixed argument return problem
 *	added event flags
 *
 *	*********************************************************
 *	*							*
 *	* MUST be compiled with -f, uses floating point !	*
 *	*							*
 *	*********************************************************
 *
 * This program is a compute bound process, which is
 * to make sure that the CPU is busy.
 * This program is a CPU exerciser NOT a diagnostic,
 * error checking is minimal and cryptic.
 *
 * This exerciser has four major functions:
 *
 * 1.	Use as many as possible of the functions of
 *	the C programming language.
 *
 * 2.	Force dynamic memory reallocation via the
 *	calloc() function.
 *
 * 3.	Force dynamic growth of the user stack,
 *	by expanding the stack size to greater than the
 *	20 64-byte segments initially allocated.
 *
 * 4.	Relocate thru memory via a fork/exec at the
 *	end of each pass. The pass count is passed to
 *	the next copy of cpx as an argument, every NPASS
 *	(20) passes the `end of pass' message is printed.
 *
 * USAGE:
 *		cpx -z # # [pass count]
 *		    ^	   ^
 *		    |      pass count from exec of new copies of cpx
 *		    event flag bit positon
 *
 *	
 *
 */

#include <sys/param.h>	/* Does not matter which one ! */
#include <stdio.h>
#include <signal.h>

char	ca = 077;
int	ia = 23145;
unsigned int ua = 65000;
long	la = 1235563;
float	fa = 123.1;
double	da = 56899.713;

unsigned int smask[] =
{
	01,02,04,010,020,040,0100,0200,0400,01000,02000,04000,
	010000,020000,040000,0100000
};

int	a[10] [10];
int	b[10] [10];

struct s
{
	char	sc;
	int	si;
	unsigned int su;
	long	sl;
	double	sd;
};

char csa[]="abcdefghijklmnopqrstuvwxzy0123456789`'[]{}|=+-_~()*&^%$#@!;:><?.,";
char csb[100];

/* function return definitions */
char 		cf();
char 		*cpf();
unsigned	uf();
long		lf();
double		df();

time_t timbuf;
long randx;

#define	NPASS	20	/* number of fork/exec's to make a pass */

int	fcount;
char	np[20];

#ifdef EFLG
#include <sys/eflg.h>
char	*efpis;
char	*efids;
int	efbit;
int	efid;
long	evntflg();
int	zflag;
#else
char	*killfn = "cpx.kill";
#endif

main(argc, argv)
char *argv[];
int argc;
{
	int	intr(), stop();
	FILE	*argf;
	register int i;

	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, intr);
	signal(SIGQUIT, stop);
	if((argc == 4) || (argc == 5)) {
#ifdef EFLG
		zflag++;
		efpis = argv[2];
		efbit = atoi(efpis);
		efids = argv[3];
		efid = atoi(efids);
#else		
		killfn = argv[2];
#endif
	}
	time(&timbuf);
	randx = timbuf & 0777;
#ifdef EFLG
	if((argc == 4) || (argc == 1)) {
		if(!zflag) {
			if(isatty(2)) {
		    		fprintf(stderr, "cpx: detaching... type \"sysxstop\" to stop\n");
		    		fflush(stderr);
			}
			if((i = fork()) == -1) {
				printf("cpx: Can't fork new copy !\n");
				exit(1);
			}
			if(i != 0)
				exit(0);
		}
		setpgrp(0, 31111);
		printf("\n\nCPU exerciser started - %s", ctime(&timbuf));
		if(zflag) {
			evntflg(EFCLR, efid, (long)efbit);
		}
	} else {
		if(argc == 5)
			fcount = atoi(argv[4]);
		else
			fcount = atoi(argv[1]);
	}
#else
	if((argc == 3) || (argc == 1)) {
		printf("\n\nCPU exerciser started - %s", ctime(&timbuf));
		unlink(killfn);		/* tell SYSX cpx started */
	} else
		fcount = atoi(argv[3]);	/* pass count from previous copy */
#endif
	fflush(stdout);
loop:
	intr();		/* poll for kill */
	t_if_gt(1);	/* if & goto */
	t_wh_bc(2);	/* while, break, & continue */
	t_dow_bc(3);	/* do-while, break, & continue */
	t_for_bc(4);	/* for, break, & continue */
	t_sw_b(5);	/* switch & break */
	t_shft(6);	/* shift */
	t_ro_eo(7);	/* relational/equality operators */
	t_baox(8);	/* bitwise AND OR XOR operators */
	t_laoc(9);	/* logical AND, OR, conditional operator */
	t_ao(10);	/* assignment operators */
	t_fap(11, ca, ia, ua, la, fa, da);	/* function argument passing */
	t_far(12);	/* function argument return */
	t_st(13);	/* string move */
/*	intr();	*/
	t_cid(14);	/* crunch integers - data space */
	t_sid(15);	/* sort integers - data space, calloc() */
	t_csds(16);	/* crunch structure data - stack */
			/* force dynamic stack growth */
	t_un(17);	/* union - stack */
	if(++fcount >= NPASS)	{	/* end of pass */
		fcount = 0;
		time(&timbuf);
		printf("\nCPU exerciser end of pass - %s", ctime(&timbuf));
	}
	fflush(stdout);
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	if((i = fork()) == 0) {
		sprintf(&np, "%d", fcount); /* pass fcount as arg to next copy */
#ifdef EFLG
		if(zflag)
			execl("cpx", "cpx", "-z", efpis, efids, &np, (char *)0);
		else
			execl("cpx", "cpx", &np, (char *)0);
#else
		execl("cpx", "cpx", "-r", killfn, &np, (char *)0);
#endif
		fprintf(stderr, "\ncpx: Can't exec new copy of cpx !\n");
		exit(1);
	}
	if(i == -1) {
		fprintf(stderr, "\ncpx: Can't fork new copy of cpx !\n");
		signal(SIGTERM, intr);
		signal(SIGQUIT, stop);
		goto loop;
	}
	exit(0);
}

intr()
{
	signal(SIGTERM, intr);
#ifdef EFLG
	if(zflag) {
		if(!checkflg())
			return;
	} else
		return;
#else
	if(access(killfn, 0) != 0)
		return;
#endif
	stop();
}

stop()
{
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	time(&timbuf);
	printf("\n\nCPU exerciser stopped - %s\n", ctime(&timbuf));
	fflush(stdout);
#ifdef EFLG
	if(zflag)
		evntflg(EFCLR, efid, (long)efbit);
#else
	unlink(killfn);
#endif
	exit(0);
}

rng()
{
	return(((randx = randx * 1103515245 + 12345) >> 16) & 0177777);
}

em(tn, stn)
{
	printf("\n\n****** CPU EXERCISER ERROR ******");
	printf("\n****** TEST %02d SUBTEST %03d ******", tn, stn);
	fflush(stdout);
}

t_if_gt(tn)
{
	register int i;
	register char j;

	i = 0;
	if(i != 0)
		em(tn, 1);
	i++;
	if(i <= 0)
		em(tn, 2);
	i = -1;
	if(i >= 0)
		em(tn, 3);
	i = (rng() & 037) + 1;
	j = 0;
loop:
	j &= 7;
	if(j == 0)
		goto j0;
	else if(j == 1)
		goto j1;
	else if(j == 2)
		goto j2;
	else if(j == 3)
		goto j3;
	else if(j == 4)
		goto j4;
	else if(j == 5)
		goto j5;
	else if(j == 6)
		goto j6;
	else if(j == 7)
		goto j7;
	else
		em(tn, 4);
next:
	if(++j <= 7)
		goto loop;
	if(--i <= 0)
		return;
	goto loop;
j0:
	if(j != 0)
		em(tn, 5);
	goto next;
j1:
	if(j != 1)
		em(tn, 6);
	goto next;
j2:
	if(j != 2)
		em(tn, 7);
	goto next;
j3:
	if(j != 3)
		em(tn, 8);
	goto next;
j4:
	if(j != 4)
		em(tn, 9);
	goto next;
j5:
	if(j != 5)
		em(tn, 10);
	goto next;
j6:
	if(j != 6)
		em(tn, 11);
	goto next;
j7:
	if(j != 7)
		em(tn, 12);
	goto next;
}

t_wh_bc(tn)
{
	register int j;
	char	ci;
	int	ii;
	unsigned int ui;
	long	li;

	j = (rng() & 037) + 1;
loop:
	ci = -10;
	while(++ci < 10) ;
	if(ci != 10)
		em(tn, 1);
	ii = -20;
	while(++ii < 20) ;
	if(ii != 20)
		em(tn, 2);
	li = -50;
	while(++li < 50) ;
	if(li != 50)
		em(tn, 3);
	ui = 100;
	while(--ui);
	if(ui != 0)
		em(tn, 4);
	ii = -5;
	while(++ii <10) {
		if(ii == 2)
			break;
	}
	if(ii != 2)
		em(tn, 5);
	ii = -5;
	while(++ii != 3000) {
		if(ii < 20)
			continue;
		break;
	}
	if(ii != 20)
		em(tn, 6);
	if(--j <= 0)
		return;
	goto loop;
}

t_dow_bc(tn)
{
	register int j, k;
	char	ci;
	int	ii;
	long	li;
	unsigned int ui;

	j = (rng() & 037) + 1;
loop:
	ci = -10;
	k = -10;
	do
		++ci;
	while(++k < 10);
	if((k != 10) || (ci != 10))
		em(tn, 1);
	ii = -20;
	k = -20;
	do
		ii++;
	while(++k < 20);
	if((k != 20) || (ii != 20))
		em(tn, 2);
	li = -63;
	k = -63;
	do
		li++;
	while(++k < 63);
	if((k != 63) || (li != 63))
		em(tn, 3);
	ui = 56;
	k = 56;
	do
		--ui;
	while(--k);
	if((k != 0) || (ui != 0))
		em(tn, 4);
	k = 0;
	do {
		k++;
		if(k == 23)
			break;
	} while(k < 30);
	if(k != 23)
		em(tn, 5);
	k = 0;
	do {
		k++;
		if(k < 27)
			continue;
		break;
	} while(k < 3000);
	if(k != 27)
		em(tn, 6);
	if(--j <= 0)
		return;
	goto loop;
}

t_for_bc(tn)
{
	register int i, j, k;
	char	ci;
	int	ii;
	unsigned int ui;
	long	li;

	i = (rng() & 037) + 1;
loop:
	k = -10;
	for(ci = -10; ci < 10; ci++)
		k++;
	if((k != 10) || (ci != 10))
		em(tn, 1);
	k = -52;
	for(ii = -52; ii < 46; ii++)
		k++;
	if((k != 46) || (ii != 46))
		em(tn, 2);
	k = -78;
	for(li = -78; li < 93; li++)
		k++;
	if((k != 93) || (li != 93))
		em(tn, 3);
	k = 0;
	for(ui = 0; ui < 34; ui++)
		k++;
	if((k != 34) || (ui != 34))
		em(tn, 4);
	k = -36;
	for(j = -36; j < 14; j++) {
		k++;
		if(k == 5)
			break;
	}
	if(k != 5)
		em(tn, 5);
	k = 0;
	for(j=0; j<3000; j++) {
		k++;
		if(k < 24)
			continue;
		break;
	}
	if(k != 24)
		em(tn, 6);
	if(--i <= 0)
		return;
	goto loop;
}

t_sw_b(tn)
{
	register int i, j, k;
	char	ci;
	int	ii;
	unsigned int ui;

	i = (rng() & 037) + 1;
loop:
	ci = 0143;
	switch(ci) {
	case 0143:
		break;
	default:
		em(tn, 7);
		break;
	}
	ii = 30111;
	switch(ii) {
	case 30111:
		break;
	default:
		em(tn, 8);
		break;
	}
	ui = 5000;
	switch(ui) {
	case 5000:
		break;
	default:
		em(tn, 9);
		break;
	}
	for(j=0; j<10; j++) {
		k = -20;
	switch(j) {
	case 0:
		k = 0;
		break;
	case 1:
		k = 1;
		break;
	case 2:
		k = 2;
		break;
	case 3:
		k = 3;
		break;
	case 4:
		k = 4;
		break;
	case 5:
		k = 5;
		break;
	case 6:
		k = 6;
		break;
	case 7:
		k = 7;
		break;
	case 8:
		k = 8;
		break;
	case 9:
		k = 9;
		break;
	default:
		break;
		}
	if(k != j)
		em(tn, 10);
	}
	if(--i <= 0)
		return;
	goto loop;
}

t_shft(tn)
{
	register int i, j, k;
	char	ci;
	int	ii;
	unsigned int ui;
	long	li;

	i = (rng() & 037) + 1;
loop:
	k = 1;
	for(j=0; j<16; j++) {
		if(k != smask[j])
			em(tn, 1);
		k = k << 1;
	}
	k = 0100000;
	k = k >> 15;
	if(k != -1)
		em(tn, 2);
	ci = 1;
	for(j=0; j<7; j++) {
		if(ci != smask[j])
			em(tn, 3);
		ci = ci << 1;
	}
	ii = 1;
	for(j=0; j<16; j++) {
		if(ii != smask[j])
			em(tn, 4);
		ii = ii << 1;
	}
	li = 0100000L;
	for(j=0; j<16; j++) {
		if(li != smask[15-j])
			em(tn, 5);
		li = li >> 1;
	}
	li = 1;
	li = li << 16;
	if(li != 0200000)
		em(tn, 6);
	ui = 1;
	for(j=0; j<16; j++) {
		if(ui != smask[j])
			em(tn, 7);
		ui = ui << 1;
	}
	for(j=0; j<16; j++) {
		k = (1 << j);
		if(k != smask[j])
			em(tn, 8);
	}
	k = 1;
	k <<= 16;
	if(k != 0)
		em(tn, 9);
	if(--i <= 0)
		return;
	goto loop;
}

t_ro_eo(tn)
{
	register int i, j, k;

	i = (rng() & 037) + 1;
loop:
	k = 30000;
	if(k < 30000)
		em(tn, 1);
	if(k > 30000)
		em(tn, 2);
	if(k <= 30000)
		goto l1;
	em(tn, 3);
l1:
	if(k >= 30000)
		goto l2;
	em(tn, 4);
l2:
	if(k > 234)
		goto l3;
	em(tn, 5);
l3:
	if(k < 30001)
		goto l4;
	em(tn, 6);
l4:
	if(k <= 456)
		em(tn, 7);
	if(k >= 30003)
		em(tn, 8);
	k = 97;
	if(k != 97)
		em(tn, 9);
	if(k == 33)
		em(tn, 10);
	if(k == 97)
		goto l5;
	em(tn, 11);
l5:
	if(k != 76)
		goto l6;
	em(tn, 12);
l6:
	if(--i <= 0)
		return;
	goto loop;
}

t_baox(tn)
{
	register int i, j, k;

	i = (rng() & 037) + 1;
loop:
	for(j=0; j<16; j++) {
		k = 0177777;
		k &= (1 << j);
		if( k != smask[j])
			em(tn, 1);
	}
	k = 0177777;
	if((k & 0) != 0)
		em(tn, 2);
	if((k & 0177777) != 0177777)
		em(tn, 3);
	k = 0;
	for(j=0; j<16; j++)
		k |= (1 << j);
	if(k != 0177777)
		em(tn, 4);
	k = 0;
	if((k | 0) != 0)
		em(tn, 5);
	k = 0177777;
	if((k | 0) != 0177777)
		em(tn, 6);
	k = 0;
	if((k | 0177777) != 0177777)
		em(tn, 7);
	k = 1;
	j = 2;
	if(j ^ k)
		goto l1;
	em(tn, 8);
l1:
	if(j ^ 2)
		em(tn, 9);
	if(--i <= 0)
		return;
	goto loop;
}

t_laoc(tn)
{
	register int i, j, k;

	i = (rng() & 037) + 1;
loop:
	j = 1;
	k = 2;
	if(k && j)
		goto l1;
	em(tn, 1);
l1:
	if((k==0) && j)
		em(tn, 2);
	j = 0;
	if(k && j)
		em(tn, 3);
	k = 0;
	if((k==0) && (j==0))
		goto l2;
	em(tn, 4);
l2:
	j = 0;
	k = 0;
	if(j || k)
		em(tn, 4);
	j++;
	if(j || k)
		goto l3;
	em(tn, 5);
l3:
	k++;
	if(k || j)
		goto l4;
	em(tn, 6);
l4:
	if(!k || !j)
		em(tn, 7);
	k = 0;
	j = !k ? 12: 13;
	if(j != 12)
		em(tn, 8);
	j = k ? 12: 13;
	if( j != 13)
		em(tn, 9);
	if(--i <= 0)
		return;
	goto loop;
}

t_ao(tn)
{
	register int i, j, k;

	i = (rng() & 037) + 1;
loop:
	j = 123;
	if(j != 123)
		em(tn, 1);
	k = 1;
	if(k != 1)
		em(tn, 2);
	j += k;
	k = 2;
	k += 43;
	if(k != 45)
		em(tn, 3);
	k = 77;
	k -= 15;
	if(k != 62)
		em(tn, 4);
	k = 2;
	j = 24;
	j *= k;
	if(j != 48)
		em(tn, 5);
	j = 66;
	k = 33;
	j /= k;
	if(j != 2)
		em(tn, 6);
	k = 515;
	j = k%512;
	if(j != 3)
		em(tn, 7);
	k %= 512;
	if(k != 3)
		em(tn, 8);
	j = 1;
	j <<= 1;
	if(j != 2)
		em(tn, 9);
	k = 2;
	k >>= 1;
	if(k != 1)
		em(tn, 10);
	k = 0177777;
	k &= 4;
	if(k != 4)
		em(tn, 11);
	k = 1;
	j = 2;
	k ^= j;
	if(j == 0)
		em(tn, 12);
	j = 0;
	j |= 1;
	if(j != 1)
		em(tn, 13);
	if(--i <= 0)
		return;
	goto loop;
}

t_fap(tn, cap, iap, uap, lap, fap, dap)
char	cap;
int	iap;
unsigned int uap;
long	lap;
float	fap;
double	dap;
{
	float	fb = 123.1;

	if(cap != 077)
		em(tn, 1);
	if(iap != 23145)
		em(tn, 2);
	if(uap != 65000)
		em(tn, 3);
	if(lap != 1235563)
		em(tn, 4);
	if(fap != fb)
		em(tn, 5);
	if(dap != 56899.713)
		em(tn, 6);
}

t_cid(tn)
{
	register int i, j, k;

	k = (rng() & 37) + 1;
loop:
	for(i=0; i<10; i++)
		for(j=0; j<10; j++) {
			a[i] [j] = (i*10) + j;
			b[i] [j] = (i*10) + j;
		}
	for(i=0; i<10; i++)
		for(j=0; j<10; j++) {
			if(a[i] [j] != b[i] [j]) {
				em(tn, 1);
				goto l1;
			}
			if(a[i] [j] != ((i*10)+j)) {
				em(tn, 2);
				goto l1;
			}
		}
l1:
	for(i=0; i<10; i++)
		for(j=0; j<10; j++)
			if((a[i] [j] * b[i] [j]) != (((i*10)+j)*((i*10)+j))) {
				em(tn, 3);
				goto l2;
			}
l2:
	if(--k <= 0)
		return;
	goto loop;
}

t_sid(tn)
{
	extern char *calloc();
	register int *n;
	register int *nb;
	register i;
	int j, k;

	nb = calloc(1001, sizeof(int));
	if(nb == 0)
		em(tn, 1);
	n = nb;
	for(i=0; i<1000; i++)
		*n++ = rng();
	*n = 32767;
loop:
	k = 0;
	n = nb;
	for(i=0; i<1000; i++) {
		if(*n > *(n+1)) {
			k++;
			j = *n;
			*n = *(n+1);
			*(n+1) = j;
		}
		n++;
	}
	if(k)
		goto loop;
	n = nb;
	for(i=0; i<1000; i++) {
		if(*n > *(n+1)) {
			em(tn, 2);
			break;
		}
		n++;
	}
}

t_csds(tn)
{
	register struct s *p;
	struct s d[200];
	register int i;
	double fdc;

	p = &d[0];
	fdc = 1234.456;
	for(i=0; i<200; i++) {
		p->sc = rng() & 0377;
		p->si = rng();
		p->su = rng();
		p->sl = (rng() << 16) | rng();
		p->sd = fdc;
		p++;
		fdc += 45.39;
	}
	p = &d[0];
	fdc = 34.67;
	for(i=0; i<200; i++) {
		p->sc |= (rng() & 0377);
		p->si += rng();
		p->su &= rng();
		p->sl *= rng();
		p->sd /= fdc;
		p++;
		fdc += 23.459;
	}
	fdc = 23.982;
	for(i=0; i<200; i++) {
		d[i].sc += (rng() & 0377);
		d[i].si -= rng();
		d[i].su &= rng();
		d[i].sl |= ((rng() << 16) | rng());
		d[i].sd *= fdc;
		fdc += 12.4;
	}
}

t_st(tn)
{
	register int i, j;
	register char *p;
	char *n;

	i = (rng() & 037) + 1;
loop:
	p = &csb[0];
	for(j=0; j<100; j++)
		*p++ = (rng() & 0377);
	p = &csa[0];
	n = &csb[0];
	do
		*n++ = *p++;
	while(*p);
	p = &csa[0];
	n = &csb[0];
	do {
		if(*p++ != *n++) {
			em(tn, 1);
			break;
		}
	} while(*p);
	if(--i <= 0)
		return;
	goto loop;
}

t_far(tn)
{
	register int i, k;
	long ln;
	char cn;
	double fdn;
	extern double df();


	i = (rng() & 037) + 1;
	fdn = 2397.99;
loop:
	k = rng();
	cn = rng() & 0377;
	ln = (rng() << 16) | rng();
	if(cf(cn) != cn)
		em(tn, 1);
	if(ifn(k) != k)
		em(tn, 2);
	if(lf(ln) != ln)
		em(tn, 3);
	if(uf(k) != k)
		em(tn, 4);
	if(df(fdn) != fdn)
		em(tn, 5);
	if(cpf(&csa[0]) != &csa[0])
		em(tn, 6);
	fdn += 912.65;
	if(--i <= 0)
		return;
	goto loop;
}

char cf(c)
char c;
{
	return(c);
}

ifn(i)
int i;
{
	return(i);
}

long lf(l)
long l;
{
	return(l);
}

unsigned uf(u)
unsigned int u;
{
	return(u);
}

double df(d)
double d;
{
	return(d);
}

char *cpf(cp)
char *cp;
{
	return(cp);
}

t_un(tn)
{
	register int i, j, k;
	long ln;
	union {
		long	la;
		int	ia[2];
		char	ca[4];
	} un;

	i = (rng() & 037) + 1;
loop:
	ln = 0100200401;
	for(j=0; j<8; j++) {
		un.la = ln << j;
		if(un.la != (ln << j))
			em(tn, 1);
		if(un.ia[0] != (0401 << j))
			em(tn, 2);
		if(un.ia[1] != (0401 << j))
			em(tn, 3);
		for(k=0; k<4; k++)
			if((un.ca[k]&0377) != (1 << j))
				em(tn, 4);
	}
	if(--i <= 0)
		return;
	goto loop;
}

/*
 * Check eventflags to stop
 * return 0 for continuation
 * return 1 to stop
 */
extern int errno;
checkflg()
{
	union efrt {
		long	efret;
		struct {
			int	a;
			int	b;
		} retval
	} ef;
	errno = 0;
	ef.efret = evntflg(EFRD, efid, (long)0);
	if(errno && ef.retval.a == -1) {
		zflag = 0;
		return(0);
	}
	if(ef.efret & (1L << efbit))
		return(1);
	return(0);
}
