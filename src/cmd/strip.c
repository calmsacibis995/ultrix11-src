
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)strip.c 3.0 4/22/86";
#include <stdio.h>
#include <a.out.h>
#include <signal.h>
#define LINELEN	132
char	*tname;
char	*mktemp();
struct exec head, nhead;
int	a_magic[] = {A_MAGIC1, A_MAGIC2, A_MAGIC3, A_MAGIC4,
		     A_MAGIC5, A_MAGIC7, A_MAGIC8, A_MAGIC9, SA_MAGIC, 0};
int	status;
int	tf;
main(argc, argv)
char *argv[];
{
	register i;
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	tname = mktemp("/tmp/sXXXXX");
	close(creat(tname, 0600));
	tf = open(tname, 2);
	if(tf < 0) {
		printf("cannot create temp file\n");
		exit(2);
	}
	for(i=1; i<argc; i++) {
		strip(argv[i]);
		if(status > 1)
			break;
		close(tf);			/* clear the tmpfile */
		close(creat(tname, 0600));
		tf = open(tname, 2);
	}
	close(tf);
	unlink(tname);
	exit(status);
}
strip(name)
char *name;
{
	char line[LINELEN];
	char *p, *q;
	register f;
	long size;
	int i, cc;
	f = open(name, 0);
	if(f < 0) {
		printf("cannot open %s\n", name);
		status = 1;
		goto out;
	}
	read(f, (char *)&head, sizeof(head));
	lseek(f, 0l, 0);
	read(f, (char *)&nhead, sizeof(nhead));
	for(i=0;a_magic[i];i++)
		if(a_magic[i] == head.a_magic) break;
	if(a_magic[i] == 0) {
		printf("%s not in a.out format\n", name);
		status = 1;
		goto out;
	}
	if(head.a_syms == 0 && (head.a_flag&1) != 0) {
		printf("%s already stripped\n", name);
		goto out;
	}
	/*
	 * Check if name contains 'unix' anywhere.
	 * If so, warn about stripping unix kernel.
	 */
	p=name;
tip2:
	for( ; *p!='u'; p++) {
		if ((*p=='\0') || (*p=='\ '))		/* end of word */
			break;
	}
	if (*p++=='u') {
		q=p;					/* save our place */
		if ((*p++=='n') && (*p++=='i') && (*p++=='x')) {
			printf("\nWarning: 'strip' will remove the namelist from '%s'\n",name);
			printf("A UNIX kernel will not boot if the namelist is missing.\n\n");
			printf("'strip %s' : Are you sure? < no > ",name);
			if (fgets(line, LINELEN, stdin) == NULL) {
						/* ctrl-D - return */
				printf("\n");
				goto out;
			}
			else if (line[0] == 'y')	/* strip file */
				;
			else
				goto out;		/* else assume 'no' */
		}
		else {
			p=q;		/* restore p, skipped current letter */
			goto tip2;
		}
	}
	size = (long)head.a_text + head.a_data;
	head.a_syms = 0;
	head.a_flag |= 1;
	lseek(tf, (long)0, 0);
	write(tf, (char *)&head, sizeof(head));
	if (head.a_magic == 0431 || head.a_magic == 0430) {
		unsigned sizes[8];
		int i;
		read(f, (char *)&sizes, sizeof sizes);
		write(tf, (char *)&sizes, sizeof sizes);
		for (i = 1; i < 8; i++)
			copy(name, f, tf, (long) sizes[i]);
	}
	if(copy(name, f, tf, size)) {
		status = 1;
		goto out;
	}
	lseek(f, (long)nhead.a_syms, 1);
	if (nhead.a_flag == 0)
		lseek(f, (long)head.a_text + (long)head.a_data, 1);
	qcopy(name, f, tf);
	size += sizeof(head);
	close(f);
	f = creat(name, 0666);
	if(f < 0) {
		printf("%s cannot recreate\n", name);
		status = 1;
		goto out;
	}
	lseek(tf, (long)0, 0);
	if(copy(name, tf, f, size))
		status = 2;
	qcopy(name, tf, f);
out:
	close(f);
}
copy(name, fr, to, size)
char *name;
long size;
{
	register s, n;
	char buf[1024];
	while(size != 0) {
		s = 1024;
		if(size < 1024)
			s = size;
		n = read(fr, buf, s);
		if(n != s) {
			printf("%s unexpected eof\n", name);
			return(1);
		}
		n = write(to, buf, s);
		if(n != s) {
			printf("%s unexpected write eof\n", name);
			return(1);
		}
		size -= s;
	}
	return(0);
}
qcopy(name, fr, to)
char *name;
{
	register s, n;
	char buf[1024];
	for(;;) {
		n = read(fr, buf, 1024);
		if(n == 0)
			return;
		if(n < 0) {
			perror("strip");
			return;
		}
		s = write(to, buf, n);
		if(n != s) {
			printf("%s unexpected write eof\n", name);
			return(1);
		}
	}
}
