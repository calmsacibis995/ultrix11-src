
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 Line printer mode set command (lpset)
 *
 * Allows the user to change the line printer parameters:
 *
 *	flag
 *	indent
 *	lines per page
 *	column width
 *
 * Fred Canter 7/8/83
 */

static char Sccsid[] = "@(#)lpset.c 3.0 4/21/86";
#include <sgtty.h>
#include <a.out.h>

#define	OPEN	04
#define	FFCLOSE	010
#define	CAP	020
#define	NOCR	040

struct	nlist	nl[] =
{
	{ "_lp_dt" },
	{ "" }
};

struct lpmode {
	char	lpm_flag;
	char	lpm_ind;
	int	lpm_line;
	int	lpm_col;
} lp_mode;

char	*lock = "/usr/spool/lpd/lock";

int	rflag;

main(argc, argv)
char	*argv[];
int	argc;
{
	register struct lpmode *lpm;
	int fd;

	if((argc == 2) && (argv[1][0] == '-') && (argv[1][1] == 'r'))
		rflag++;
	if((rflag == 0) && (argc != 1) && (argc != 5)) {
		printf("lpset: arg count\n");
		exit(1);
	}
	nlist("/unix", nl);
	if(nl[0].n_type == 0) {
		printf("lpset: /unix not configured for line printer\n");
		exit(1);
	}
	lpm = &lp_mode;
	if(access(lock, 0) == 0) {
		printf("lpset: sorry LP spooler active\n");
		exit(1);
	}
	fd = open("/dev/lp", 1);
	if(fd < 0) {
		printf("lpset: can't open /dev/lp\n");
		exit(1);
	}
	if(rflag) {
		lpm->lpm_flag = 0;
		lpm->lpm_ind = 0;
		lpm->lpm_line = 66;
		lpm->lpm_col = 132;
		ioctl(fd, TIOCSETP, &lp_mode);
		exit(0);
	}
	if(argc == 1) {
		ioctl(fd, TIOCGETP, &lp_mode);
		printf("\nCurrent LP parameters:\n\n");
		printf("\t%03o  Flags ( ", lpm->lpm_flag);
		if(lpm->lpm_flag&OPEN)
			printf("OPEN ");
		if(lpm->lpm_flag&FFCLOSE)
			printf("FFCLOSE ");
		if(lpm->lpm_flag&CAP)
			printf("CAP ");
		if(lpm->lpm_flag&NOCR)
			printf("NOCR ");
		printf(")\n");
		printf("\t%3d  Indent\n", lpm->lpm_ind);
		printf("\t%3d  Lines per page\n", lpm->lpm_line);
		printf("\t%3d  Column width\n", lpm->lpm_col);
		exit(0);
	}
	lpm->lpm_flag = onum(argv[1]);
	if(lpm->lpm_flag & 0307) {
		printf("lpset: bad flag bits\n");
		exit(1);
	}
	lpm->lpm_ind = atoi(argv[2]);
	if(lpm->lpm_ind >= 16) {
		printf("lpset: max indent is 15\n");
		exit(1);
	}
	lpm->lpm_line = atoi(argv[3]);
	if(lpm->lpm_line==0) {
		printf("lpset: lines per page can't be zero\n");
		exit(1);
	}
	lpm->lpm_col = atoi(argv[4]);
	if((lpm->lpm_col==0) || (lpm->lpm_col > 132)) {
		printf("lpset: column width out of range\n");
		exit(1);
	}
	ioctl(fd, TIOCSETP, &lp_mode);
}

onum(s)
char	*s;
{
	register int n;
	n = 0;
	while(*s) {
		n = n << 3;
		n |= (*s & 7);
		s++;
	};
	return(n&0377);
}
