
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)eli.c	3.0	4/21/86";
/*
 * ULTRIX-11 error log initialization program (eli).
 *
 * Fred Canter 10/2/82
 *
 * Usage:
 *     eli [-d] [-i] [-f] [-e] [-u] [-c] [file]
 *
 *	-d	disable error logging
 *	-i	initialize the error log file (previous contents lost)
 *	-f	FAST error log init, don't ask if you mean it !!!
 *	-e	enable error logging
 *	-u	print number of error log blocks used
 *	-c	copy contents of the error log to [ file ]
 *
 * note - error logging is disabled by the [-i] option and must
 *	  be reenabled by the [-e] option.
 *
 * note - only one [-opt] can be specified for each 
 *	  execution of eli.
 */

#include <sys/param.h>	/* does not matter which one */
#include <sys/errlog.h>
#include <a.out.h>

#define	R	0
#define	W	1

/*
 * System error log information,
 * obtained from unix via errlog system call (EL_INFO).
 */

struct	el_data	el_data;
daddr_t	el_sb;		/* error log starting block number */
int	el_nb;		/* error log length in blocks */

/* disk read/write buffer */

int	buf[256+1];

/* various character string arrays */

char	line[20];
char	*errdev = "/dev/errlog";

main (argc, argv)

char	*argv[];
int	argc;

{

	register struct elrhdr *ehp;
	register char *ibp;
	register i;
	int fi, fo, mem;
	char *p;
	char c;

	if((argc <= 1) || (argc > 3)) {
	usage:
	     printf("\neli: usage  eli [-d] [-i] [-f] [-e] [-u] [-c] [file]\n");
		exit(1);
		}

/*
 * Get the error log start block and length
 * from /unix kernel via errlog system call.
 */
	errlog(EL_INFO, &el_data);
	el_sb = el_data.el_sb;
	el_nb = el_data.el_nb;

	p = argv[1];		/* decode [-opt] and act accordingly */
	while(*p == '-')
		*p++;
	switch(*p) {
	case 'd':		/* disable error logging */
		errlog(EL_OFF, 0);
		break;
	case 'e':		/* enable error logging */
		errlog(EL_ON, 0);
		break;
	case 'i':
		printf("\nError log can be saved via [-c] option\n");
		printf("\nReally zero the error log ?\n");
		p = line;
		i = 0;
		while((c = getchar()) != '\n') {
			*p++ = c;
			if(++i > 3)
				goto no;
		}
		*p++ = 0;
		if((line[0] != 'y') ||
		(line[1] != 'e') ||
		(line[2] != 's')) {
		no:
			printf("\nError log not zeroed\n");
			break;
		}
/* disable error logging, init kernel buffer, don't log shutdown */
	case 'f':	/* FAST initialize */
		errlog(EL_INIT, 0);
		for(i=0; i<256; i++)	/* zero disk buffer */
			buf[i] = 0;
		if((fo = open(errdev, W)) < 0) {
		opnerr:
			printf("\neli: Can't open %s\n", errdev);
			exit(1);
		}
		lseek(fo, (long)(el_sb * 512), 0);
		for(i=el_sb; i<(el_sb+el_nb); i++) {
			if(write(fo, (char *)&buf, 512) != 512) {
				printf("\neli: write error, bn = %d\n", i);
				exit(1);
				}
			}
		close(fo);
		printf("\nError log zeroed\n");
		break;
	case 'c':	/* copy the error log to [file] */
		p = argv[2];			/* file name */
		if((argc != 3) || (*p == '-'))
			goto usage;
		if((fo = creat(p, 0644)) <0) {
			printf("\neli: Can't create %s\n", p);
			exit(1);
			}
		if((fi = open(errdev, R)) < 0)
			goto opnerr;
		lseek(fi, (long)(el_sb * 512), 0);
		for(i=el_sb; i<(el_sb+el_nb); i++) {
			if(read(fi, (char *)&buf, 512) != 512)
				printf("\neli: read error, bn = %d\n", i);
			if(write(fo, (char *)&buf, 512) != 512) {
				printf("\neli: write error\n");
				exit(1);
				}
			if(buf[0] == 0)	/* EOF check */
				break;
		}
		close(fi);
		close(fo);
		break;
	case 'u':	/* print number of error log blocks used */
		if((fi = open(errdev, R)) < 0)
			goto opnerr;
		lseek(fi, (long)(el_sb * 512), 0);
		for(i=0; i<el_nb; i++) {
			if(read(fi, (char *)&buf, 512) != 512) {
				printf("\neli: read error, bn = %D\n",i+el_sb);
				exit(1);
				}
			ibp = &buf;
		loop:
			ehp = ibp;
			if(ehp->e_type == E_EOF)
				break;		/* end of error log */
			if(ehp->e_type == E_EOB)
				continue;	/* end of block */
			ibp += ehp->e_size;
/*
 * This covers the rare case where an error log
 * record ends exactly at the end of block.
 */
			if((ibp - &buf) >= 512)
				continue;
			goto loop;
			}
		close(fi);
		if(i >= el_nb)
			--i;
		printf("\n\07\07\07ERROR LOG has - ");
		printf("%d of %d blocks used\n",++i, el_nb);
		break;
	default:
		goto usage;
	}
	exit(0);
}
