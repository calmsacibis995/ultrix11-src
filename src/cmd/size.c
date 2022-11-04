
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)size.c	3.0	4/22/86";
#include	<stdio.h>
#include 	<a.out.h>
#include	<ar.h>
/*
 *	size -- determine object size
 *	Modified 8/2/84 to allow size to search archives and
 *	give the sizes of the individual modules in them. -Dave Borman
 */
int	gorp;
#define	NAMSIZ	100
int a_magic[] = {
	A_MAGIC1, A_MAGIC2, A_MAGIC3,
	A_MAGIC4, A_MAGIC6, A_MAGIC7,
	A_MAGIC8, A_MAGIC9, SA_MAGIC, 0
};
main(argc, argv)
char **argv;
{
	int magic;
	struct ar_hdr arhdr;
	FILE *f;
	char	errname[NAMSIZ];	/* "size: filename" */
	if (argc==1) {
		*argv = "a.out";
		argc++;
		--argv;
	}
	gorp = argc;
	while(--argc) {
		++argv;
		if ((f = fopen(*argv, "r"))==NULL) {
			strcpy(errname, "size: ");
			strcat(errname, *argv);
			perror(errname);
			/* printf("size: %s not found\n", *argv); */
			continue;
		}
		if (fread((char *)&magic, sizeof(magic), 1, f) == NULL) {
			printf("size: %s not an object file or archive\n", *argv);
			fclose(f);
			continue;
		}
		if(magic != ARMAG) {
			/* could be a regular object file */
			fseek(f, 0L, 0);
			dosize(f, *argv, NULL);
			fclose(f);
			continue;
		}
		/* dealing with an archive */
		while (fread((char *)&arhdr, sizeof(arhdr), 1, f) != NULL) {
			long loc;
			loc = ftell(f);
			arhdr.ar_name[14] = 0;
			if (strcmp(arhdr.ar_name, "__.SYMDEF") != 0)
				dosize(f, *argv, arhdr.ar_name);
			fseek(f, loc+(arhdr.ar_size+1)&~1, 0);
		}
		fclose(f);
	}
}
			
dosize(f, name, subname)
FILE *f;
char *name, *subname;
{
	struct exec buf;
	long sum, coresize;
	int i;

	if (fread((char *)&buf, sizeof(buf), 1, f) == NULL) {
	err:
		printf("size: %s", name);
		if (subname)
			printf(": %s", subname);
		printf(" not an object file\n");
		return;
	}
	for(i=0;a_magic[i];i++)
		if(a_magic[i] == buf.a_magic) break;
	if(a_magic[i] == 0)
		goto err;
	if (gorp>2)
		printf("%s: ", name);
	if (subname)
		printf("%s: ", subname);
	printf("%u+", buf.a_text);
/* wnj added */
	coresize = buf.a_text;
	if (buf.a_magic >= 0430) {
		unsigned sizes[16];
		fread(sizes, sizeof sizes, 1, f);
		printf("(");
		for (i = 1; i < ((buf.a_magic >= 0450) ? 16 : 8); i++)
			if (sizes[i]) {
				coresize += sizes[i];
				if (i > 1)
					printf(",");
				printf("%u", sizes[i]);
			}
		printf(")+");
	}
/* end wnj added */
	printf("%u+%u = ", buf.a_data,buf.a_bss);
	sum = (long) buf.a_text + (long) buf.a_data + (long) buf.a_bss;
	printf("%Db = 0%Ob", sum, sum);
	if (coresize != buf.a_text)
		printf(" (%D total text)", coresize);
	printf("\n");
}
