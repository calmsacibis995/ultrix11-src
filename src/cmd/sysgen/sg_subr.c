
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)sg_subr.c	3.1	8/6/87";
/*
 * Subroutines used with sysgen.
 *
 * Fred Canter
 */

#include "sysgen.h"
#include <sgtty.h>

char	*smgsect =
  "\nRefer to Section 2.7 of the ULTRIX-11 System Management Guide for help!\n";

char	line[140];
struct	sgttyb	tsgtty;

struct	nlist	nl1[] =
{
	{ "_mb_end" },
	{ "_ub_end" },
	{ "" },
};

char	syscmd[50];
char	mkcmd[40] = "mkconf < \0";
char	mvcmd[40] = "mv \0";

/*
 * Make the unix operating system.
 */

mkunix()
{
	register char *p;
	FILE	*cf;
	register int ovsys, s;
	int	magic;
	unsigned txtsiz, datsiz, bsssiz ;
	long ovsize;
	unsigned	ovsiz[16];
	int	ove;
	unsigned ubs;
	long	totsiz;

	p = cname();		/* get config filename */
	if((p == 0) || (access(p, 0) != 0)) {	/* see if it exists */
		printf("\nConfiguration file does not exist!\n");
		return;
	}
	if((cf = fopen(p, "r")) == NULL) {
		printf("\nCan't open %s file\n", p);
		return;
	}
	ovsys = 0;
	while(fgets(&cfline, 40, cf) != NULL)
		if(strcmp("ov\n", &cfline) == 0) {
			ovsys = 1;
			break;
		}
	mkcmd[9] = '\0';
	strcat(mkcmd, &config);
      printf("\n****** CREATING ULTRIX-11 CONFIGURATION AND VECTOR TABLES ******\n");
	s = system(mkcmd);	/* mkconf */
	if (s != 0)
		goto cantconfig;
	printf("\n****** MAKING KERNEL FOR ");
	if(ovsys)
		printf("NON ");
	printf("SEPARATE I & D SPACE PROCESSORS ******\n\n");
	if(ovsys)
		s = system("make unix23");
	else
		s = system("make unix70");
	p = &config;
	while(*p++ != '.');
	if(s != 0) {	/* check exit status of `make unix' */
		*--p = '\0';
	cantconfig:
		printf("\n\7\7\7Can't make kernel from configuration `%s'!\n",
			&config);
		return;
	}
	*p++ = 'o';
	*p++ = 's';
	mvcmd[3] = '\0';
	if(ovsys)
		strcat(&mvcmd, "unix_ov ");
	else
		strcat(&mvcmd, "unix_id ");
	strcat(&mvcmd, &config);
	system(&mvcmd);
	printf("\nNew kernel is now named `%s'!\n", &config);
	printf("\n****** CHECKING SIZE OF NEW ULTRIX-11 OPERATING SYSTEM ******\n");
	if((cf = fopen(&config, "r")) == NULL) {
		printf("\nCan't open %s file\n", &config);
		return;
	}
	magic = getw(cf);
	txtsiz = getw(cf);
	datsiz = getw(cf);
	bsssiz = getw(cf);
	getw(cf); getw(cf); getw(cf); getw(cf);
	if (magic >= 0430)
		for(s=0; s<8; s++)
			ovsiz[s] = getw(cf);
	if (magic >= 0450)
		for (; s < 16; s++)
			ovsiz[s] = getw(cf);
	fclose(cf);
	if(ovsys && (magic != 0430) && (magic != 0450)) {
		printf("\n\7\7\7`%s' is not an overlay kernel!\n", &config);
		goto fserr;
	}
	if(!ovsys && (magic != 0411) && (magic != 0431) && (magic != 0451)) {
		printf("\n\7\7\7`%s' is not separate I & D space kernel!\n",
			&config);
		goto fserr;
	}
	if((txtsiz == 0) || (datsiz == 0) || (bsssiz == 0)) {
		printf("\n\7\7\7Size of zero detected!\n");
		goto fserr;
	}
	switch(magic) {
	case 0411:
	case 0431:
	case 0451:
		if((datsiz + bsssiz) > 49088) {
			printf("\n\7\7\7DATA+BSS ");
			printf("size = %u, maximum is 49088 bytes!\n",
				(datsiz+bsssiz));
			goto fserr;
		}
		if(magic == 0411)	/* text size checked above (0 = 65536) */
			goto sok;
		if((txtsiz <= 49152) || (txtsiz > 57344)) {
			printf("\n\7\7\7ROOT TEXT size = %u, ", txtsiz);
			printf("must be > 49152 and <= 57344 bytes!\n");
			goto fserr;
		}
		ovsize = 0;
		for(s=1, ove=0; s < ((magic == 0431) ? 8 : 16); s++) {
			if(ovsiz[s] == 0)
				continue;
			ovsize += ovsiz[s];
			if(ovsiz[s] > 8192) {
				printf("\n\7\7\7Overlay %d size = %u, ",
					s, ovsiz[s]);
				printf("maximum is 8192 bytes!\n");
				ove++;
			}
		}
		if(ove)
			goto fserr;
		/*
		 * Following size check allows for 512 bytes boot stack -> 192kb
		 * 430, 450, 411, and 431 type kernels will always be below
		 * this value, only 451 can overflow (at around 11 overlays)
		 */
		totsiz = (long)datsiz+(long)bsssiz+(long)txtsiz+ovsize;
		if (totsiz > ((192L*1024L) - 512L)) {
			printf("\n\7\7\7Total kernel size = %D ", totsiz);
			printf("maximum is %D bytes!\n", ((192L*1024L) - 512L));
			goto fserr;
		}
		break;
	case 0430:
	case 0450:
		if((txtsiz <= 8192) || (txtsiz > 16384)) {
			printf("\n\7\7\7ROOT TEXT size = %u, ", txtsiz);
			printf("must > 8192 and <= 16384 bytes!\n");
			goto fserr;
		}
		if((datsiz+bsssiz) > 24576) {
			printf("\n\7\7\7DATA+BSS size = %u, ", datsiz+bsssiz);
			printf("maximum is 24576 bytes!\n");
			goto fserr;
		}
		for(s=1, ove=0; s < ((magic == 0430) ? 8 : 16); s++) {
			if(ovsiz[s] == 0)
				continue;
			if(ovsiz[s] > 8192) {
				printf("\n\7\7\7Overlay %d size = %u, ",
					s, ovsiz[s]);
				printf("maximum is 8192 bytes!\n");
				ove++;
			}
		}
		if(ove)
			goto fserr;
		break;
	}
/*
 * Check symbol _mb_end to insure
 * that no protected data structures fall within the
 * forbidden zone.
 */
	if(nlist(&config, nl1) < 0) {
		printf("\nCan't access namelist in %s\n", &config);
		goto fserr;
	}
	if((nl1[0].n_value == 0) || (nl1[0].n_value > 0120000)) {
		printf("\n\7\7\7MAPPED BUFFERS - forbidden zone violation!");
		printf("\n_mb_end = %o", nl1[0].n_value);
mb_err:
		printf("%s", smgsect);
		goto fserr;
	}
/*
 * If CPU has a unibus map, make sure all data
 * structures that require NPR access fall within
 * the range of the first map register.
 */
	ubs = datsiz + 8192;
	if(ovsys)
		ubs += 060000;
	if(nl1[1].n_value > ubs) {
		printf("\n\7\7\7UNIBUS MAP - forbidden zone violation!");
		printf("\n_ub_end = %o", nl1[1].n_value);
		printf("%s", smgsect);
		printf("\nDO NOT attempt to install `%s' ", &config);
		printf("on any processor with a unibus map!\n");
		return;
	}
sok:
	printf("\n`%s' within limits, SYSGEN successful!\n", &config);
	return;
fserr:
	printf("\n\7\7\7FATAL size error `%s' should not be installed!\n",
		&config);
	return;
}


/*
 * Handle yes or no responses.
 * If only return typed take default response.
 * y or yes is the YES response, n or no is NO !
 * def = 1, default answer is yes - return 1 on yes
 * def = 0, default answer is no - return 0 on no
 * hlp = 1, help available - return -1 on ?
 * hlp = 0, no help - print "type yes or no" on ?
 */

yes(def, hlp)
{
	char	resp[10];
	int	cc;

	buflag = 0;
	if(def)
		printf(" <yes> ? ");
	else
		printf(" <no> ? ");
yn:
	fflush(stdout);
	cc = read(0, (char *)&resp, 10);
	if(cc == 0) {	/* ^D - cancel script line */
		buflag++;
		return(NO);
	}
	if(cc > 4)
		goto ynerr;
	if(cc == 1)
		return(def);
	if((cc == 2) && (resp[0] == '?')) {
		if(hlp == HELP)
			return(-1);
		else {
		ynerr:
			ioctl(0, TIOCFLUSH, &tsgtty);	/* flush stdin */
			printf("\nPlease answer yes or no!\n\n? ");
			goto yn;
		}
	}
	if(resp[0] == 'y') {
		if(cc == 2)
			return(YES);
		if((cc == 4) && (resp[1] == 'e') && (resp[2] == 's'))
			return(YES);
	} else if(resp[0] == 'n') {
		if(cc == 2)
			return(NO);
		if((cc == 3) && (resp[1] == 'o'))
			return(NO);
	}
	goto ynerr;
}

/*
 * Get a line of text form the terminal,
 * replace the new line character with 0
 * and return the character count.
 * hlp = 0, print `no help available'
 * hlp > 0, print help message if `?' typed
 */

getline(hlp)
char	*hlp;
{
	register int	cc, i;

	buflag = 0;
loop:
	fflush(stdout);
	cc = read(0, (char *)&line, 82);
	if(cc == 82) {
		printf("\nLine too long, try again!\n");
		if(line[81] == '\n')
			goto glxit;	/* got entire line */
		while(line[0] != '\n')
			read(0, (char *)&line, 1);	/* flush input */
		goto glxit;
	}
	for(i=0; i<cc; i++) {
		if((line[i] >= 'A') && (line[i] <= 'Z'))
			line[i] |= 040;	/* force lower case */
		if((line[i] == '\r') || (line[i] == '\n')) {
			line[i] = 0;
			break;
		}
	}
	if(cc == 0) {	/* ^D - cancel script line */
		putchar('\n');
	glxit:
		return(0);
	}
	if((cc == 2) && (line[0] == '?')) {
		cc = -1;
		if(hlp == NOHELP)
			printf("\nSorry no help available!\n\n? ");
		else if (strcmp(hlp,"sg_mtu") == 0)
			pmsg(sg_mtu);
		else if (strcmp(hlp,"sg_dstarea") == 0) /* sg_help overflows */
			pmsg(sg_dstarea);
		else
			phelp(hlp);
	}
	return(cc);
}

/*
 * Read the configuration name from the terminal
 * and load it into the array `config'.
 * The default config name is always `unix'.
 * The configuration name can be up to 11 characters.
 */

cname()
{
	register char *p;
	register int cc;

gsn:
	do
		printf("\nConfiguration name <unix> ? ");
	while((cc = getline("sg_cn")) < 0);
	if (cc == 0)
		return(0);
	if(cc > 9) {
		printf("\nName too long, 8 characters maximum!\n");
		goto gsn;
	}
	if(cc == 1)
		p = strcpy(&config, "unix.cf");
	else {
		p = strcpy(&config, line);
		p = strcat(p, ".cf");
	}
	return(p);
}

intr()
{
	signal(SIGINT, intr);
	longjmp(savej, 1);
}

acon(n)
char	*n;
{
	register int num;
	register char *p;

	p = n;
	num = 0;
	while((*p != '\n') && (*p != '\0')) {
		if((*p < '0') || (*p > '7'))
			return(-1);
		num = num << 3;
		num |= (*p++ & 7);
	}
	return(num);
}

/*
 * Check the unix controller name in (line) to
 * see that it is configured, return 0 if not,
 * otherwise return a character pointer to its name.
 */

gdname(dn)
char	*dn;
{
	register int i;

	for(i=0; dcd[i].dctyp; i++)
		if((strcmp(dn, dcd[i].dcname) == 0) && dcd[i].dcnd)
			return(dcd[i].dcname);
	printf("\n`%s' not configured!\n", dn);
	return(0);
}

pmsg(s)
char **s;
{
	register int i;

	for(i=0; s[i]; i++) {
		if(s[i] == -1) {
			printf("\n\nPress <RETURN> for more:");
			while(getchar() != '\n') ;
		} else
			printf("\n%s", s[i]);
	}
}

/*
 * Print help message,
 * call sg_help and pass it help message name.
 */

phelp(str)
char *str;
{
	register int i;

	i = fork();
	if(i == -1) {
		printf("\nCan't call sg_help (fork failed)\n");
		return;
	}
	if(i == 0) {
		execl("sg_help", "sg_help", str, (char *)0);
		exit();
	}
	while(wait(0) != -1) ;
}
