
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

# include	"stdio.h"
# include	"sys/types.h"
# include	"macros.h"

static char Sccsid[] = "@(#)what.c 3.0 4/22/86";

char pattern[]  =  "@(#)";
char opattern[]  =  "~|^`";


main(argc,argv)
int argc;
register char **argv;
{
	register int i;
	register FILE *iop;

	if (argc < 2)
		dowhat(stdin);
	else
		for (i = 1; i < argc; i++) {
			if ((iop = fopen(argv[i],"r")) == NULL)
				fprintf(stderr,"can't open %s (26)\n",argv[i]);
			else {
				printf("%s:\n",argv[i]);
				dowhat(iop);
			}
		}
}


dowhat(iop)
register FILE *iop;
{
	register int c;

	while ((c = getc(iop)) != EOF) {
		if (c == pattern[0])
			trypat(iop, &pattern[1]);
		else if (c == opattern[0])
			trypat(iop, &opattern[1]);
	}
	fclose(iop);
}


trypat(iop,pat)
register FILE *iop;
register char *pat;
{
	register int c;

	for (; *pat; pat++)
		if ((c = getc(iop)) != *pat)
			break;
	if (!*pat) {
		putchar('\t');
		while ((c = getc(iop)) != EOF && c && !any(c,"\"\\>\n"))
			putchar(c);
		putchar('\n');
	}
	else if (c != EOF)
		ungetc(c, iop);
}
