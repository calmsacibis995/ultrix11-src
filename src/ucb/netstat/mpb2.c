static char *Sccsid = "@(#)mpb2.c	3.0	(ULTRIX-11)	4/22/86";

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#include <sys/param.h>
#include <a.out.h>
#include <stdio.h>
#include <sys/mbuf.h>

struct nlist nl[] = {
#define	N_MBBASE	0
	{ "_mbbase" },
#define	N_MBSIZE	1
	{ "_mbsize" },
	"",
};

char	*system = "/unix";
char	*kmemf = "/dev/mem";
int	kmem;
char	usage[] = "[ system ] [ core ]";

main(argc, argv)
	int argc;
	char *argv[];
{
	int i;
	char *cp, *name;
	long	mbuf1;
	int	mbbase, mbsize;

	name = argv[0];
	argc--, argv++;
  	while (argc > 0 && **argv == '-') {
		for (cp = &argv[0][1]; *cp; cp++)
		switch(argv[0][1]) {

		default:
use:
			printf("usage: %s %s\n", name, usage);
			exit(1);
		}
		argv++, argc--;
	}
	if (argc > 0) {
		system = *argv;
		argv++, argc--;
	}
	nlist(system, nl);
	if (nl[0].n_type == 0) {
		fprintf(stderr, "%s: no namelist\n", system);
		exit(1);
	}
	if (argc > 0)
		kmemf = *argv;
	kmem = open(kmemf, 0);
	if (kmem < 0) {
		fprintf(stderr, "cannot open ");
		perror(kmemf);
		exit(1);
	}
	lseek(kmem, (long)nl[N_MBBASE].n_value, 0);
	read(kmem, &mbbase, sizeof(mbbase));
	lseek(kmem, (long)nl[N_MBSIZE].n_value, 0);
	read(kmem, &mbsize, sizeof(mbsize));
	mbuf1 = ctob((long)mbbase);
	for (i = 0; i < mbsize/MSIZE; i++) {
		struct {
			char *backp;
			int	ref;
		} mbxs;
		lseek(kmem, mbuf1, 0);
		read(kmem, &mbxs, sizeof(mbxs));
		printf("%O: backp = %o, ref = %o\n", mbuf1, mbxs.backp, mbxs.ref);
		mbuf1 += MSIZE;
	}
}
