
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

#include "spell.h"
static char Sccsid[] = "@(#)spellin.c	3.0	4/22/86";
/* add entries to hash table for use by spell
   preexisting hash table is first argument
   words to be added are standard input
   if no hash table is given, create one from scratch
*/

main(argc,argv)
char **argv;
{
	register i, j;
	long h;
	register long *lp;
	char word[NW];
	register char *wp;

	if(!prime(argc,argv)) {
		fprintf(stderr,
		    "spellin: cannot initialize hash table\n");
		exit(1);
	}
	while (fgets(word, sizeof(word), stdin)) {
		for (i=0; i<NP; i++) {
			for (wp = word, h = 0, lp = pow2[i];
				 (j = *wp) != '\0'; ++wp, ++lp)
				h += j * *lp;
			h %= p[i];
			set(h);
		}
	}
#ifdef gcos
	freopen((char *)NULL, "wi", stdout);
#endif
	if (fwrite((char *)tab, sizeof(*tab), TABSIZE, stdout) != TABSIZE) {
		fprintf(stderr,
		    "spellin: trouble writing hash table\n");
		exit(1);
	}
	return(0);
}
