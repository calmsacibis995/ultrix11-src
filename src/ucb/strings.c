
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * Base on @(#)strings.c	4.1 (Berkeley) 10/1/80
 */
static char Sccsid[] = "@(#)strings.c	3.0	4/22/86";
#include <stdio.h>
#include <a.out.h>
#include <ctype.h>

long	ftell();

/*
 * strings
 */

struct	exec header;

char	*infile = "Standard input";
int	oflg;
int	asdata;
long	offset;
int	minlength = 4;

main(argc, argv)
	int argc;
	char *argv[];
{

	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		register int i;
		if (argv[0][1] == 0)
			asdata++;
		else for (i = 1; argv[0][i] != 0; i++) switch (argv[0][i]) {

		case 'o':
			oflg++;
			break;

		case 'a':
			asdata++;
			break;

		default:
			if (!isdigit(argv[0][i])) {
				fprintf(stderr, "Usage: strings [ -a ] [ -o ] [ -# ] [ file ... ]\n");
				exit(1);
			}
			minlength = argv[0][i] - '0';
			for (i++; isdigit(argv[0][i]); i++)
				minlength = minlength * 10 + argv[0][i] - '0';
			i--;
			break;
		}
		argc--, argv++;
	}
	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				perror(argv[0]);
				exit(1);
			}
			infile = argv[0];
			argc--, argv++;
		}
		fseek(stdin, (long) 0, 0);
		if (asdata ||
		    fread((char *)&header, sizeof header, 1, stdin) != 1 || 
		    N_BADMAG(header)) {
			fseek(stdin, (long) 0, 0);
			find((long) 100000000L);
			continue;
		}
		if (header.a_magic >= 0430) {
			/* overlay file; need to skip the overlays. */
			int	os[8];
			long	offset;
			fseek(stdin, (long)sizeof(header), 0);
			fread((char *)os, sizeof(os), 1, stdin);
			offset = (long)sizeof(header) + sizeof(os) +
				header.a_text + os[1] + os[2] + os[3] +
				os[4] + os[5] + os[6] + os[7];
			if (header.a_magic >= 0450) {
				fread((char *)os, sizeof(os), 1, stdin);
				offset += sizeof(os) + os[0] + os[1] + os[2]
					+ os[3] + os[4] + os[5] + os[6] + os[7];
			}
			fseek(stdin, (long)offset, 1);
		} else
			fseek(stdin, (long) N_TXTOFF(header)+header.a_text, 1);
		find((long) header.a_data);
	} while (argc > 0);
	exit(0);
}

find(cnt)
	long cnt;
{
	static char buf[BUFSIZ];
	register char *cp;
	register int c, cc;

	cp = buf, cc = 0;
	for (; cnt != 0; cnt--) {
		c = getc(stdin);
		if (c == '\n' || dirt(c) || cnt == 0) {
			if (cp > buf && cp[-1] == '\n')
				--cp;
			*cp++ = 0;
			if (cp > &buf[minlength]) {
				if (oflg)
					printf("%7D ", ftell(stdin) - cc - 1);
				printf("%s\n", buf);
			}
			cp = buf, cc = 0;
		} else {
			if (cp < &buf[sizeof buf - 2])
				*cp++ = c;
			cc++;
		}
		if (ferror(stdin) || feof(stdin))
			break;
	}
}

dirt(c)
	int c;
{

	switch (c) {

	case '\n':
	case '\f':
		return (0);

	case 0177:
		return (1);

	default:
		return (c > 0200 || c < ' ');
	}
}
