/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985.	      *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/include/COPYRIGHT" for applicable restrictions.  *
 **********************************************************************/

/*
 * Chung-Wu Lee, Jul-30-85
 *
 *	Ported from 2.9 BSD
 */

#ifndef lint
static	char	*sccsid = "@(#)mt.c	3.0	4/21/86";
#endif lint

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>

#define	equal(s1,s2)	(strcmp(s1, s2) == 0)

struct commands {
	char *c_name;
	int c_code;
	int c_ronly;
} com[] = {
	{ "weof",	MTWEOF,	0 },
	{ "eof",	MTWEOF,	0 },
	{ "fsf",	MTFSF,	1 },
	{ "bsf",	MTBSF,	1 },
	{ "fsr",	MTFSR,	1 },
	{ "bsr",	MTBSR,	1 },
	{ "rewind",	MTREW,	1 },
	{ "offline",	MTOFFL,	1 },
	{ "rewoffl",	MTOFFL,	1 },
	{ "status",	MTNOP,	1 },
	{ "cache",	MTCACHE,	1 },
	{ "nocache",	MTNOCACHE,	1 },
	{ "cse",	MTCSE,	1 },
	{ "clx",	MTCLX,	1 },
	{ "cls",	MTCLS,	1 },
	{ "async",	MTASYNC,	1 },
	{ "noasync",	MTNOASYNC,	1 },
	{ "enaeot",	MTENAEOT,	1 },
	{ "diseot",	MTDISEOT,	1 },
	{ 0 }
};

int mtfd;
struct mtop mt_com;
struct mtget mt_status;
char *tape;

main(argc, argv)
	char **argv;
{
	char line[80], *getenv();
	register char *cp;
	register struct commands *comp;

	if (argc > 2 && (equal(argv[1], "-t") || equal(argv[1], "-f"))) {
		argc -= 2;
		tape = argv[2];
		argv += 2;
	} else
		if ((tape = getenv("TAPE")) == NULL)
			tape = DEFTAPE;
	if (argc < 2) {
		fprintf(stderr, "usage: mt [ -f device ] command [ count ]\n");
		exit(1);
	}
	cp = argv[1];
	for (comp = com; comp->c_name != NULL; comp++)
		if (strncmp(cp, comp->c_name, strlen(cp)) == 0)
			break;
	if (comp->c_name == NULL) {
		fprintf(stderr, "mt: don't grok \"%s\"\n", cp);
		exit(1);
	}
	if (strcmp(comp->c_name,"clx") == 0 || strcmp(comp->c_name,"cls") == 0) {
		if ((mtfd = open(tape, O_NDELAY)) < 0) {
			perror(tape);
			exit(1);
		}
	}
	else {
		if ((mtfd = open(tape, comp->c_ronly ? 0 : 2)) < 0) {
			perror(tape);
			exit(1);
		}
	}
	if (comp->c_code != MTNOP) {
		mt_com.mt_op = comp->c_code;
		mt_com.mt_count = (argc > 2 ? atoi(argv[2]) : 1);
		if (mt_com.mt_count < 0) {
			fprintf(stderr, "mt: negative repeat count\n");
			exit(1);
		}
		if (ioctl(mtfd, MTIOCTOP, &mt_com) < 0) {
			fprintf(stderr, "%s %s %d ", tape, comp->c_name,
				mt_com.mt_count);
			perror("failed");
			exit(2);
		}
	} else {
		if (ioctl(mtfd, MTIOCGET, (char *)&mt_status) < 0) {
			perror("mt");
			exit(2);
		}
		status(&mt_status);
	}
}

struct tape_desc {
	short	t_type;		/* type of magtape device */
	char	*t_name;	/* printing name */
	char	*t_dsbits;	/* "drive status" register */
	char	*t_erbits;	/* "error" register */
} tapes[] = {
	{ MT_ISTS,	"ts11/tu80/tsv05/tsu05/tk25",	0,	TSXS0_BITS },
	{ MT_ISHT,	"tm02/tm03",		HTDS_BITS,	HTER_BITS },
	{ MT_ISTM,	"tm11",			0,		TMER_BITS },
	{ MT_ISTK,	"tk50/tu81",		0,		0 },
	{ 0 }
};

/*
 *	Follows are not supported by ULTRIX-11.
 *
 *	{ MT_ISMT,	"tu78",		MTDS_BITS,	0 },
 *	{ MT_ISUT,	"tu45",		UTDS_BITS,	UTER_BITS },
 *
 */

/*
 * Interpret the status buffer returned
 */
status(bp)
	register struct mtget *bp;
{
	register struct tape_desc *mt;

	for (mt = tapes; mt->t_type; mt++)
		if (mt->t_type == bp->mt_type)
			break;
	if (mt->t_type == 0) {
		printf("unknown tape drive type (%d)\n", bp->mt_type);
		return;
	}
	printf("%s tape drive, residual=%d\n", mt->t_name, bp->mt_resid);
	if (mt->t_type == MT_ISTK) {
		printf("flags=0%o, ", (bp->mt_dsreg >> 8) & 0377);
		printf("endcode=0%o, ", bp->mt_dsreg & 0377);
		printf("status=0%o", bp->mt_erreg);
	}
	else {
		printreg("ds", bp->mt_dsreg, mt->t_dsbits);
		printreg("\ner", bp->mt_erreg, mt->t_erbits);
	}
	printf("\nsoftstat=<");
	if (bp->mt_softstat&MT_EOT)
		printf("EOT,");
	if (bp->mt_softstat&MT_DISEOT)
		printf("DISEOT");
	else
		printf("ENAEOT");
	if (bp->mt_softstat&MT_CACHE)
		printf(",CACHE");
	printf(">\n");
}

/*
 * Print a register a la the %b format of the kernel's printf
 */
printreg(s, v, bits)
	char *s;
	register char *bits;
	register unsigned short v;
{
	register int i, any = 0;
	register char c;

	if (bits && *bits == 8)
		printf("%s=%o", s, v);
	else
		printf("%s=%x", s, v);
	bits++;
	if (v && bits) {
		putchar('<');
		while (i = *bits++) {
			if (v & (1 << (i-1))) {
				if (any)
					putchar(',');
				any = 1;
				for (; (c = *bits) > 32; bits++)
					putchar(c);
			} else
				for (; *bits > 32; bits++)
					;
		}
		putchar('>');
	}
}
