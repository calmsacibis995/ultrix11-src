
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)fpx.c	3.0	4/22/86";
/*
 * ULTRIX-11 Floating Point exerciser program (fpx).
 *
 * Fred Canter 5/12/83
 * Bill Burns  4/23/84
 *	added event flag code
 *
 *	********************************************
 *	*					   *
 *	* This program will not function correctly *
 *	* unless the current unix monitor file is  *
 *	* named "unix".				   *
 *	*					   *
 *	********************************************
 *
 * This program exercises the FP11-A/B/C/E/F floating point
 * processors, and the FPF11 11/23 & 11/24 floating point,
 * the 11/40 FIS is not supported by Unix.
 * The fpx program crunches floating point numbers and
 * tests the floating point exception mechanism.
 *
 * USAGE:
 *
 *	fpx [-n #] [-e #]
 *
 *	-n #	Limit the number of data mismatch errors to be 
 *		printed to # for each error occurrence.
 *
 *	-e #	Set the data error limit to #, default = 100 maximum = 1000.
 *		If the number of error occurrences exceeds # terminate fpx.
 *		An error occurrence can represent upto [-n #] error printouts.
 *
 */

#include <sys/param.h>	/* Does not matter which one ! */
#include <stdio.h>
#include <a.out.h>
#include <signal.h>

#define	ITER	99999L
long	iter = ITER;	/* will be lowered for fp emulation */

#define	NPASS	12	/* number of forks to make a pass */
int	npass = NPASS;	/* will be lowered for fp emulation */

int	fcount;

struct nlist nl[] =
{
	{ "fpp" },
	{ "_fpemulation" },
	{ "" }
};
int	fpp;		/* floating point present */
int	fpx;		/* expected FP exception error code, 0 if not */
			/* status returned by fperr() system call */
int	fpemulation;
int	fpstat[3];	/* FP status register */
			/* FP error code */
			/* FP error address */

double	a, b;

int	tn;
time_t timbuf;
long randx;

/*
 * 16 doubles sorted in assending order,
 * no magic in these numbers.
 * Don't use zero !!!
 */

double	n_sort[] =
{
	-6522261.8,
	-333156.29743,
	-22416.15,
	-7522.5482,
	-321.42,
	-42.6,
	-1.1,
	-0.0021,
	0.00014,
	2.2,
	34.156,
	752.8,
	9561.42865,
	17291.64,
	156242.849,
	9651422.35,
};

/*
 * Empty array,
 * a place to put the answer.
 */

double	n_ans[16];

/*
 * Two arrays of 16 numbers for the
 * add/subtract test and two arrays of the
 * expected answers.
 * again, no magic !
 */

double	n_as1[] =
{
	879321.45,
	-5432543.197,
	435678.0001,
	-1000000.4,
	0.00002,
	-0.12345,
	1.9999,
	-3.459011,
	348943.1,
	-999999.999,
	4562999.9003,
	-9999321.0009,
	4589.12006,
	-43999.00009,
	12002.5,
	-55000.0003,
};

double	n_as2[] =
{
	0.29,
	-1.4,
	-0.123456,
	0.000001,
	4.0003,
	-0.5,
	-2.25,
	0.04999,
	3965135.0004,
	-99368.00032,
	-54999.9,
	99111346.12345,
	45832.01,
	-22479.0245,
	-4765.1,
	345.9999,
};

double	n_plus[] =
{
	879321.74,
	-5432544.597,
	435677.876644,
	-1000000.399999,
	4.00032,
	-0.62345,
	-0.2501,
	-3.409021,
	4314078.1004,
	-1099367.99932,
	4508000.0003,
	89112025.12255,
	50421.13006,
	-66478.02459,
	7237.4,
	-54654.0004
};

double	n_min[] =
{
	879321.16,
	-5432541.797,
	435678.123556,
	-1000000.400001,
	-4.00028,
	0.37655,
	4.2499,
	-3.509001,
	-3616191.9004,
	-900631.99868,
	4617999.8003,
	-109110667.12435,
	-41242.88994,
	-21519.97559,
	16767.6,
	-55346.0002
};

char	c_exp[100], c_act[100];

/*
 * Two arrays of 16 numbers for the
 * multiply/divide test.
 */

double	n_md1[] =
{
	0.12345,
	-0.589,
	0.00001,
	-0.6,
	12345.0,
	-4.0,
	2001.0,
	-99000.0,
	3896.003,
	-10999.98,
	44367.5489,
	-2003.478,
	1000043.59,
	-99345.999,
	87003.5,
	-832002.123
};

double	n_md2[] =
{
	0.58999,
	-0.1,
	-0.9,
	0.3401,
	23.0,
	-567.0,
	-1234.0,
	9.0,
	4899.003,
	-1234.4321,
	-9984.02,
	45903.12,
	1.003,
	-13.6702,
	-3.002,
	5.0034
};

double	n_mul[] =
{
	0.072834,
	0.0589,
	-0.000009,
	-0.204060,
	283935.0,
	2268.0,
	-2469234.0,
	-891000.0,
	19086530.385009,
	13578728.411358,
	-442966495.568578,
	-91965891.05136,
	1003043.72077,
	1358079.675530,
	-261184.507,
	-4162839.422218
};

double	n_div[] =
{
	0.209241,
	5.89,
	-0.000011,
	-1.764187,
	536.739130,
	0.007055,
	-1.621556,
	-11000.0,
	0.795264,
	8.910964,
	-4.443856,
	-0.043646,
	997052.432702,
	7267.340566,
	-28981.845436,
	-166287.349203
};

int	ndep = 5;
int	ndrop = 100;
int	ndec;

char	*dex = "\n\n[ data error print limit exceeded ]";
int	ntec;

#ifdef EFLG
#include <sys/eflg.h>
char	*efpis;
char	*efids;
int	efbit;
int	efid;
long	evntflg();
long	efret;
int	zflag;
#else
char	*killfn = "fpx.kill";
#endif

main(argc, argv)
char *argv[];
int argc;
{
	int	intr(), stop();
	register int i, j;
	int fpxerr();
	FILE	*argf;
	long	li;
	int	mem;

	signal(SIGTTOU, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, intr);
	signal(SIGQUIT, stop);
/*
	if(((argc & 1) == 0) || (argc > 7)) {
	argerr:
		fprintf(stderr, "\nfpx: bad arg\n");
		fprintf(stderr, "\nUsage: fpx [-n #] [-e #]\n\n");
		exit(1);
	}
*/
	for(i=1; i<argc; i++) {
		if(argv[i] [0] != '-') {
		argerr:
			exit(1);
		}
#ifdef EFLG
		if(argv[i] [1] == 'z') {
			zflag++;
			i++;
			efpis = argv[i];
			i++;
			efids = argv[i];
		}
#else
		if(argv[i] [1] == 'r') {
			i++;
			killfn = argv[i];
		}
#endif
		if(argv[i] [1] == 'n') {
			i++;
			if((argv[i] [0] < '0') || (argv[i] [0] > '9'))
				goto argerr;
			ndep = atoi(argv[i]);
		}
		if(argv[i] [1] == 'e') {
			i++;
			if((argv[i] [0] < '0') || (argv[i] [0] > '9'))
				goto argerr;
			ndrop = atoi(argv[i]);
			if((ndrop < 0) || (ndrop > 1000)) {
			    fprintf(stderr, "\nfpx bad [-e #], using 100 !\n");
				ndrop = 100;
			}
		}
	}
	if(!zflag) {
		if(isatty(2)) {
		    fprintf(stderr, "fpx: detaching... type \"sysxstop\" to stop\n");
		    fflush(stderr);
		}
		if((i = fork()) == -1) {
			printf("fpx: Can't fork new copy !\n");
			exit(1);
		}
		if(i != 0)
			exit(0);
	}
	setpgrp(0, 31111);
	nlist("/unix", nl);
	if(nl[1].n_value == 0) {
	    fprintf(stderr,"\nfpx: ULTRIX-11 not configured for Floating Point\n");
	    exit(1);
	}
	if((mem = open("/dev/mem", 0)) < 0) {
		fprintf(stderr, "\nfpx: Can't open /dev/mem\n");
		exit(1);
	}
	lseek(mem, (long)nl[0].n_value, 0);
	read(mem, (char *)&fpp, sizeof(fpp));
	lseek(mem, (long)nl[1].n_value, 0);
	read(mem, (char *)&fpemulation, sizeof(fpemulation));
	close(mem);
	if(fpp == 0) {
		iter = 999;
		npass = 6;
	}
	if((fpp == 0) && (fpemulation != 1)) {
		fprintf(stderr, "\nfpx: No Floating Point present, ");
		if(fpemulation == 2)
			fprintf(stderr, "and FP Emulation not in kernel !\n");
		else
			fprintf(stderr, "and FP Emulation turned off !\n");
		exit(0);
	}
	time(&timbuf);
	randx = timbuf & 0777;
	printf("\n\nFloating Point exerciser started - %s", ctime(&timbuf));
	fflush(stdout);
#ifdef EFLG
	if(zflag) {
		efbit = atoi(efpis);
		efid = atoi(efids);
		evntflg(EFCLR, efid, (long)efbit);
	}
#else
	unlink(killfn);		/* tell sysx fpx started */
#endif
	signal(SIGFPE, fpxerr);
loop:
/*
 * TEST 1 - increment/decrement
 *
 *	Two floating point numbers starting at 0.0 are incremented
 *	by 0.1 iter (see above define) times. Each time they are
 *	compared to make sure they match. They are then decremented
 *	by 0.1 iter times and checked for a match each time,
 *	and check for a 0.0 result after all iterations are done.
 *
 * note - The checking of a & b for >= 0.00001 is really a test
 *	for equal to zero, because with floating point numbers
 *	zero is not always really zero when it gets out the the
 *	Nth decimal place.
 */

	tn = 1;
	ndec = 0;
	a = 0.0;
	b = 0.0;
	for(li=0; li<iter; li++) {
		a += 1.00001;
		b += 1.00001;
		if((a != b) || (a < 0.00001) || (b < 0.00001)) {
			em(tn);
			printf("\nIncrement a & b by 1.00001, %D times,", iter);
			printf("\na & b should match");
			printf("\ncount\t= %D", li);
			printf("\na\t= %f\nb\t= %f\n", a, b);
			if(++ndec >= ndep) {
				printf("%s", dex);
				break;
			}
		}
	}
	if(ndec)
		ntec++;
	ndec = 0;
	for(li=0; li<iter; li++) {
		if((a < 0.00001) || (b < 0.00001))
			goto err1;
		a -= 1.00001;
		b -= 1.00001;
		if(a != b) {
	err1:
			em(tn);
			printf("\nDecrement a & b by 1.00001, %D times,", iter);
			printf("\na & b should match");
			printf("\ncount\t= %D", li);
			printf("\na\t= %f\nb\t= %f\n", a, b);
			if(++ndec >= ndep) {
				printf("%s", dex);
				break;
			}
		}
	}
	if(ndec)
		ntec++;
	if((a >= 0.00001) || (b >= 0.00001)) {
		em(tn);
		printf("\nAfter test %02d, a & b should equal zero", tn);
		printf("\na\t= %f\nb\t= %f\n",a,b);
		ntec++;
	}
	elcheck();	/* check for error limit exceeded */
/*
 * TEST 2 - exceptions
 *
 *	Test as many of the floating point exceptions as possible
 *	by attempting illegal operations like divide by zero and
 * 	catching the SIGFPE signal. The fperr() system call is used
 *	to obtain the floating point exception status.
 *	NOTE -	As it turns out, divide by zero is the only floating
 *		point exception that can be tested.
 */

	tn = 2;
	fpx = 4;	/* divide by zero */
	a = 1.1/0.0;
	fpxck();
	elcheck();
/*
 * TEST 3 - sort
 *
 *	Copy a presorted array of 16 numbers to an empty array
 *	in randum order, then sort them and compare the result
 *	against the original array.
 */

	tn = 3;
	ndec = 0;
	for(i=0; i<16; i++)
		n_ans[i] = 0.0;	/* answer set to all zeroes */
	for(i=0; i<16; i++) {	/* copy to answer, out of order */
		j = rng() & 017;
		while(n_ans[j] != 0.0)
			if(++j >= 16)
				j = 0;
		n_ans[j] = n_sort[i];
	}
t3_sl:			/* sort */
	j = 0;
	for(i=0; i<15; i++)
		if(n_ans[i] > n_ans[i+1]) {
			j++;
			a = n_ans[i];
			n_ans[i] = n_ans[i+1];
			n_ans[i+1] = a;
		}
	if(j)
		goto t3_sl;
	j = 0;			/* compare */
	for(i=0; i<16; i++)
		if(n_sort[i] != n_ans[i])
			j++;
	if(j) {			/* print any errors */
		em(tn);
		printf("\nSort 16 numbers, arrays should match");
		printf("\n\nExpected\tActual\n");
		for(i=0; i<16; i++) {
			printf("\n%15.6f\t%15.6f", n_sort[i], n_ans[i]);
			if(++ndec >= ndep) {
				printf("%s", dex);
				break;
			}
		}
	printf("\n");
	}
	if(ndec)
		ntec++;
	elcheck();
/*
 * TEST 4 - add
 *
 *	Add two arrays of 16 numbers and
 *	check the result against an array
 *	of the expected answers.
 *	Strings are used to compare the results of
 *	floating point arithmetic operations because of the
 *	inaccuracy of the FP11 in the Nth decimal place.
 *	Printf rounds the results for me.
 *	"Oh silly me !"
 */
	tn = 4;
	ndec = 0;
	for(i=0; i<16; i++) {
		n_ans[i] = n_as1[i] + n_as2[i];
		sprintf(&c_exp, "%.6f", n_plus[i]);
		sprintf(&c_act, "%.6f", n_ans[i]);
		if(strcmp(&c_exp, &c_act) != 0) {
			em(tn);
			printf("\nAddition test");
			printf("\n\n%.6f plus %.7f = ", n_as1[i], n_as2[i]);
			printf("%s S/B %s\n", &c_act, &c_exp);
			if(++ndec >= ndep) {
				printf("%s", dex);
				break;
			}
		}
	}
	if(ndec)
		ntec++;
	elcheck();
/*
 * TEST 5 - subtract
 *
 *	Subtract two arrays of 16 numbers and
 *	check the result against an array
 *	of the expected answers.
 *	Strings are used to compare the results of
 *	floating point arithmetic operations because of the
 *	inaccuracy of the FP11 in the Nth decimal place.
 *	Printf rounds the results for me.
 */
	tn = 5;
	ndec = 0;
	for(i=0; i<16; i++) {
		n_ans[i] = n_as1[i] - n_as2[i];
		sprintf(&c_exp, "%.6f", n_min[i]);
		sprintf(&c_act, "%.6f", n_ans[i]);
		if(strcmp(&c_exp, &c_act) != 0) {
			em(tn);
			printf("\nSubtraction test");
			printf("\n\n%.6f minus %.7f = ", n_as1[i], n_as2[i]);
			printf("%s S/B %s\n", &c_act, &c_exp);
			if(++ndec >= ndep) {
				printf("%s", dex);
				break;
			}
		}
	}
	if(ndec)
		ntec++;
	elcheck();
/*
 * TEST 6 - multiply
 *
 *	Multiply two arrays of 16 numbers and
 *	check the result against an array
 *	of the expected answers.
 *	Strings are used to compare the results of
 *	floating point arithmetic operations because of the
 *	inaccuracy of the FP11 in the Nth decimal place.
 *	Printf rounds the results for me.
 */
	tn = 6;
	ndec = 0;
	for(i=0; i<16; i++) {
		n_ans[i] = n_md1[i] * n_md2[i];
		sprintf(&c_exp, "%.6f", n_mul[i]);
		sprintf(&c_act, "%.6f", n_ans[i]);
		if(strcmp(&c_exp, &c_act) != 0) {
			em(tn);
			printf("\nMultiplication test");
			printf("\n\n%.6f * %.7f = ", n_md1[i], n_md2[i]);
			printf("%s S/B %s\n", &c_act, &c_exp);
			if(++ndec >= ndep) {
				printf("%s", dex);
				break;
			}
		}
	}
	if(ndec)
		ntec++;
	elcheck();
/*
 * TEST 7 - divide
 *
 *	Divide two arrays of 16 numbers and
 *	check the result against an array
 *	of the expected answers.
 *	Strings are used to compare the results of
 *	floating point arithmetic operations because of the
 *	inaccuracy of the FP11 in the Nth decimal place.
 *	Printf rounds the results for me.
 */
	tn = 7;
	ndec = 0;
	for(i=0; i<16; i++) {
		n_ans[i] = n_md1[i] / n_md2[i];
		sprintf(&c_exp, "%.6f", n_div[i]);
		sprintf(&c_act, "%.6f", n_ans[i]);
		if(strcmp(&c_exp, &c_act) != 0) {
			em(tn);
			printf("\nDivision test");
			printf("\n\n%.6f / %.7f = ", n_md1[i], n_md2[i]);
			printf("%s S/B %s\n", &c_act, &c_exp);
			if(++ndec >= ndep) {
				printf("%s", dex);
				break;
			}
		}
	}
	if(ndec)
		ntec++;
	elcheck();
	if(++fcount >= npass) {	/* end of pass */
		fcount = 0;
		time(&timbuf);
	  printf("\nFloating Point exerciser end of pass - %s", ctime(&timbuf));
	}
	fflush(stdout);
	if((i = fork()) == 0)
		goto loop;
	if(i == -1) {
		fprintf(stderr, "\nfpx: Can't fork new copy of fpx !\n");
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
	printf("\n\nFloating Point exerciser stopped - %s\n", ctime(&timbuf));
	fflush(stdout);
#ifdef EFLG
	if(zflag)
		evntflg(EFCLR, efid, (long)efbit);
#else
	unlink(killfn);	/* tell SYSX fpx stopped */
#endif
	exit(0);
}

rng()
{
	return(((randx = randx * 1103515245 + 12345) >> 16) & 0177777);
}

em(tn)
{
	printf("\n\n****** FPP EXERCISER ERROR ******");
	printf("\n****** TEST NUMBER %02d      ******", tn);
}

/*
 * Floating point exception error codes.
 */

char	*fpxec[] =
{
	"",
	"Floating OP code error",
	"Floating divide by zero",
	"Floating (or double) to integer conversion error",
	"Floating overflow",
	"Floating underflow",
	"Floating undefined variable",
	"Maintenance trap",
};

/*
 * Floating point status register bits.
 */

char	*fpsr[] =
{
	"FER","FID","","","FIUV","FIU","FIV","FIC",
	"FD","FL","FT","FMM","FN","FZ","FV","FC",
};

/*
 * Floating point exception handler.
 * The variable fpx is set to the error code if
 * an exception is expected or zero if not.
 */

fpxerr()
{
	register int i;

	fperr(&fpstat);		/* get FP exception status */
	if(fpx && (fpx == fpstat[1]))
		goto xit;		/* expected exception occurred */
	ntec++;
	printf("\n****** Test Number %02d - ", tn);
	if(fpx == 0)
		printf("UNEXPECTED ");
	else
		printf("INCORRECT ");
	printf("Floating Point Exception ******");
	if(fpx) {
		printf("\nExpected - %s", fpxec[fpx >> 1]);
		printf("\nReceived - ");
		if((fpstat[1] == 0) || (fpstat[1] > 14))
			printf("Unknown exception");
		else
			printf("%s", fpxec[fpstat[1] >> 1]);
	}
	printf("\n\nFP status register\t%6o\t", fpstat[0]);
	for(i=0; i<16; i++)
		if(fpstat[0] & (1 << (15 - i)))
			printf("%s ", fpsr[i]);
	printf("\nFP error code\t\t%6o\t", fpstat[1]);
	if((fpstat[1] == 0) || (fpstat[1] > 14))
		printf("Unknown exception");
	else
		printf("%s", fpxec[fpstat[1] >> 1]);
	printf("\nFP error address\t%6o\n", fpstat[2]);
xit:
	fpx = 0;
	signal(SIGFPE, fpxerr);
}

/*
 * Make sure that the floating
 * point exception did occur.
 */

fpxck()
{
	if(fpx == 0)
		return;
	printf("\n\n****** Test number %02d - ", tn);
	printf("MISSED Floating Point Exception ******");
	printf("\nExpected - %s", fpxec[fpx >> 1]);
	printf("\nReceived - No Floating Point exception\n");
	ntec++;
}

elcheck()
{
	
#ifdef EFLG
	if(zflag) {
		if(checkflg())
			stop();
	}
#else
	if(access("fpx.kill", 0) == 0)
		stop();
#endif
	if(ntec >= ndrop) {
		printf("\nTotal error limit exceeded, FPX terminated !\n");
		fflush(stdout);
		for( ;; )
			sleep(3600);
	}
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
