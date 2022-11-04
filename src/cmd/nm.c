
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
**	print symbol tables for
**	object or archive files
**
**	nm [-goprun] [name ...]
*/
static char Sccsid[] = "@(#)nm.c	3.0	4/21/86";
#include	<ar.h>
#include	<a.out.h>
#include	<stdio.h>
#include	<ctype.h>
#define	MAGIC	exp.a_magic
#define	SELECT	arch_flg ? arp.ar_name : *argv
int	numsort_flg;
int	undef_flg;
int	revsort_flg = 1;
int	globl_flg;
int	nosort_flg;
int	arch_flg;
int	prep_flg;
struct	ar_hdr	arp;
struct	exec	exp;
FILE	*fi;
long	off;
long	ftell();
char	*malloc();
char	*realloc();
main(argc, argv)
char **argv;
{
	int narg;
	int  compare();
	if (--argc>0 && argv[1][0]=='-' && argv[1][1]!=0) {
		argv++;
		while (*++*argv) switch (**argv) {
		case 'n':		/* sort numerically */
			numsort_flg++;
			continue;
		case 'g':		/* globl symbols only */
			globl_flg++;
			continue;
		case 'u':		/* undefined symbols only */
			undef_flg++;
			continue;
		case 'r':		/* sort in reverse order */
			revsort_flg = -1;
			continue;
		case 'p':		/* don't sort -- symbol table order */
			nosort_flg++;
			continue;
		case 'o':		/* prepend a name to each line */
			prep_flg++;
			continue;
		default:		/* oops */
			fprintf(stderr, "nm: invalid argument -%c\n", *argv[0]);
			exit(1);
		}
		argc--;
	}
	if (argc == 0) {
		argc = 1;
		argv[1] = "a.out";
	}
	narg = argc;
	while(argc--) {
		fi = fopen(*++argv,"r");
		if (fi == NULL) {
			fprintf(stderr, "nm: cannot open %s\n", *argv);
			continue;
		}
		off = sizeof(exp.a_magic);
		fread((char *)&exp, 1, sizeof(MAGIC), fi);	/* get magic no. */
		if (MAGIC == ARMAG)
			arch_flg++;
		else if (BADMAG(exp)) {
			fprintf(stderr, "nm: %s-- bad format\n", *argv);
			continue;
		}
		fseek(fi, 0L, 0);
		if (arch_flg) {
			nextel(fi);
			if (narg > 1)
				printf("\n%s:\n", *argv);
		}
		do {
			long o;
			register int i, n;
			register char c;
			struct nlist *symp = NULL;
			struct nlist sym;
			unsigned ovsizes[8];
			unsigned ov2sizes[8];
			int domore = 0;

			fread((char *)&exp, 1, sizeof(struct exec), fi);
			if (BADMAG(exp))	/* archive element not in  */
				continue;	/* proper format - skip it */
			if (MAGIC >= 0430) {
			    fread((char *)ovsizes, 1, sizeof ovsizes, fi);
			    if (MAGIC >= 0450)
				fread((char *)ov2sizes, 1, sizeof ov2sizes, fi);
			    for (i = 1; i < 8; i++)
				fseek(fi, (long)ovsizes[i], 1);
			    if (MAGIC >= 0450)
				for (i = 0; i < 8; i++)
				    fseek(fi, (long)ov2sizes[i], 1);
			}
			o = (long)exp.a_text + exp.a_data;
			if ((exp.a_flag & 01) == 0)
				o *= 2;
			fseek(fi, o, 1);
			n = exp.a_syms / sizeof(struct nlist);
			if (n == 0) {
				fprintf(stderr, "nm: %s-- no name list\n", SELECT);
				continue;
			}
			i = 0;
			while (--n >= 0) {
				fread((char *)&sym, 1, sizeof(sym), fi);
				if (globl_flg && (sym.n_type&N_EXT)==0)
					continue;
				switch (sym.n_type&N_TYPE) {
				case N_UNDF:
					c = 'u';
					if (sym.n_value)
						c = 'c';
					break;
				default:
				case N_ABS:
					c = 'a';
					break;
				case N_TEXT:
					c = 't';
					break;
				case N_DATA:
					c = 'd';
					break;
				case N_BSS:
					c = 'b';
					break;
				case N_FN:
					c = 'f';
					break;
				case N_REG:
					c = 'r';
					break;
				}
				if (undef_flg && c!='u')
					continue;
				if (sym.n_type&N_EXT)
					c = toupper(c);
				sym.n_type &= ~0377;
				sym.n_type |= c;
			somemore:
				if (symp==NULL)
					symp = (struct nlist *)malloc(sizeof(struct nlist));
				else {
					struct nlist *t;
					t = symp;
					symp = (struct nlist *)realloc(symp, (i+1)*sizeof(struct nlist));
					if (symp == NULL && nosort_flg) {
						symp = (struct nlist *)realloc(t, i*sizeof(struct nlist));
						domore = n;
						break;
					}
				}
				if (symp == NULL) {
					fprintf(stderr, "nm: out of memory on %s\n", *argv);
					fprintf(stderr, "Try using '-p' (or '-g') option.\n");
					exit(2);
				}
				symp[i++] = sym;
			}
			if (nosort_flg==0)
				qsort(symp, i, sizeof(struct nlist), compare);
			if ((arch_flg || narg>1) && prep_flg==0)
				printf("\n%s:\n", SELECT);
			for (n=0; n<i; n++) {
				if (prep_flg) {
					if (arch_flg)
						printf("%s:", *argv);
					printf("%s:", SELECT);
				}
				c = symp[n].n_type&0377;
				if (!undef_flg) {
					if (c=='u' || c=='U')
						printf("      ");
					else
						printf(FORMAT, symp[n].n_value);
					printf(" %c ", c);
				}
				if (symp[n].n_type&0177400)
					printf("%-8.8s %d", symp[n].n_name,
					   (symp[n].n_type>>8)&0377);
				else
					printf("%.8s", symp[n].n_name);
				printf("\n");
			}
			if (symp)
				free((char *)symp);
			if (domore) {
				symp = NULL;
				n = domore;
				i = domore = 0;
				goto somemore;
			}
		} while(arch_flg && nextel(fi));
		fclose(fi);
	}
	exit(0);
}
compare(p1, p2)
struct nlist *p1, *p2;
{
	register i;
	if (numsort_flg) {
		if (p1->n_value > p2->n_value)
			return(revsort_flg);
		if (p1->n_value < p2->n_value)
			return(-revsort_flg);
	}
	for(i=0; i<sizeof(p1->n_name); i++)
		if (p1->n_name[i] != p2->n_name[i]) {
			if (p1->n_name[i] > p2->n_name[i])
				return(revsort_flg);
			else
				return(-revsort_flg);
		}
	return(0);
}
nextel(af)
FILE *af;
{
	register r;
	fseek(af, off, 0);
	r = fread((char *)&arp, 1, sizeof(struct ar_hdr), af);  /* read archive header */
	if (r <= 0)
		return(0);
	if (arp.ar_size & 1)
		++arp.ar_size;
	off = ftell(af) + arp.ar_size;	/* offset to next element */
	return(1);
}
