/*
 *	SCCSID: @(#)lintfile.c	3.1	9/17/87
 *
 *	This program contains errors for testing lint.
 *	John Dustin	3/15/84
 */

chr[] = "some string";
int	i;

main()
{
	int	i;

	if (lockup() < 0)
		exit(-1);

	if (fillarr() < 0)
		exit;

prompt:
	switch(chr[0]) {
	case 'w':
		if (checkok("w") == 0)
			break;
		break;
		break;
	case 'q':
		cleanup(0);
		exit(0);
		break;
	case '?':
		break;
	default:
		clrregs(2);
		printf("\nFor help, type '?'\n");
		break;

	case 'N':
		printf("\nshould never happen!");
		break;
	}
		goto prompt;
}

/*
 * this routine never gets called
 */
askcorr(a)
int a;
{
	for (;;) {
		return(1);
	}
	return;
}

interr()
{
	printf("\ninterrupt!");
}

fillarr()
{ 	
	do {
		printf("*** This could mean trouble ! ***\n");
	} while (1 != 1);
	return;
	printf("\nWarning: couldn't close %s.");
	return(0);
}

lockup()
{
	int 	i;
	int	arg1, arg2;

	if ((i = interr()) < 0) {
		printf("\ngarbage.");
		return;
	}
	else if (cleanup(arg1, arg2) == 0)
		return(-1);

	mktemp(1);
	return(mktemp());
	return;
}

cleanup()
{
	printf("\ncleanup !");
}

clrregs()
{
	int	i;
	char	*rname[20];

	for (i=0; i < 10; rname[i++]=0);		/* zero out rname */
}

checkok(p)
char *p;
{
	register char *q,*s;
	int	i;

	for (q=s=p; *s = *q; q++) {
		s++;
	}
	i = strlen(p);
}
