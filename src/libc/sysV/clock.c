
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*	SCCSID: @(#)clock.c	3.0	4/22/86	*/
/*	(System 5)	1.1	*/
/*LINTLIBRARY*/

#include <a.out.h>
#include <sys/types.h>
#include <sys/times.h>
/* 
#include <sys/param.h>
 * for HZ (clock frequency in Hz)
 * we look at the kernel for _hz now,
 * instead of using param.h for the 
 * value of HZ
 */
#define TIMES(B)	(B.tms_utime+B.tms_stime+B.tms_cutime+B.tms_cstime)

struct	nlist	clk_nl[] =
{
	{ "_hz" },
	{ "" },
};

extern long times();
static long first = 0L;

long
clock()
{
	int	hertz, i, mem;
	struct tms buffer;

	if (times(&buffer) != -1L && first == 0L)
		first = TIMES(buffer);

	if (nlist("/unix", clk_nl) < 0) {
		printf("clock: can't access system namelist in /unix\n");
		return(-1);
	}
	if ((clk_nl[0].n_type == 0) || (clk_nl[0].n_value == 0)) {
		printf("clock: can't find symbol '_hz' in /unix\n");
		return(-2);
	}
	if ((mem = open("/dev/mem", 0)) < 0) {
		printf("clock: can't open memory (/dev/mem)\n");
		return(-3);
	}
	/* get value of hz (really HZ from old <sys/param.h> */
	lseek(mem, (long)clk_nl[0].n_value, 0);
	read(mem, (char *)&hertz, sizeof(hertz));
	return ((TIMES(buffer) - first) * (1000000L/hertz));
}
