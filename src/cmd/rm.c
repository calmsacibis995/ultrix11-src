
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)rm.c	3.0	4/22/86";
int	errcode;

#include <sys/localopts.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

char	*sprintf();

main(argc, argv)
char *argv[];
{
	register char *arg;
	int fflg, iflg, rflg;

	fflg = 0;
	if (isatty(0) == 0)
		fflg++;
	iflg = 0;
	rflg = 0;
	while( argc > 1 && *(arg = *++argv) == '-' ) {
		argc--;
		if( *++arg == '\0' ) {	/* end of options */
			++argv;
			break;
		}
		while(*arg != '\0')
			switch(*arg++) {
			case 'f':
				fflg++;
				break;
			case 'i':
				iflg++;
				break;
			case 'r':
				rflg++;
				break;
			default:
				printf("rm: unknown option %s\n", *argv);
				exit(1);
			}
	}
	while(--argc > 0) {
		if(!strcmp(*argv, "..")) {
			fprintf(stderr, "rm: cannot remove `..'\n");
			continue;
		}
		rm(*argv++, fflg, rflg, iflg, 0);
	}

	exit(errcode);
}

rm(arg, fflg, rflg, iflg, level)
char arg[];
{
	struct stat buf;
	struct direct direct;
	char name[100];
	int d;

#ifdef	UCB_SYMLINKS
	if(lstat(arg, &buf))
#else
	if(stat(arg, &buf))
#endif
	{
		if (fflg==0) {
			printf("rm: %s nonexistent\n", arg);
			++errcode;
		}
		return;
	}
	if ((buf.st_mode&S_IFMT) == S_IFDIR) {
		if(rflg) {
			if (access(arg, 02) < 0) {
				if (fflg==0)
					printf("%s not changed\n", arg);
				errcode++;
				return;
			}
			if(iflg && level!=0) {
				printf("directory %s: ", arg);
				if(!yes())
					return;
			}
			if((d=open(arg, 0)) < 0) {
				printf("rm: %s: cannot read\n", arg);
				exit(1);
			}
			while(read(d, (char *)&direct, sizeof(direct)) == sizeof(direct)) {
				if(direct.d_ino != 0 && !dotname(direct.d_name)) {
					sprintf(name, "%s/%.14s", arg, direct.d_name);
					rm(name, fflg, rflg, iflg, level+1);
				}
			}
			close(d);
			errcode += rmdir(arg, iflg);
			return;
		}
		printf("rm: %s directory\n", arg);
		++errcode;
		return;
	}

	if(iflg) {
		printf("%s: ", arg);
		if(!yes())
			return;
	}
	else if(!fflg) {
#ifdef	UCB_SYMLINKS
		if ((buf.st_mode&S_IFMT) != S_IFLNK && access(arg, 02)<0) {
#else
		if (access(arg, 02)<0) {
#endif
			printf("rm: %s %o mode ", arg, buf.st_mode&0777);
			if(!yes())
				return;
		}
	}
	if(unlink(arg) && (fflg==0 || iflg)) {
		printf("rm: %s not removed\n", arg);
		++errcode;
	}
}

dotname(s)
char *s;
{
	if(s[0] == '.')
		if(s[1] == '.')
			if(s[2] == '\0')
				return(1);
			else
				return(0);
		else if(s[1] == '\0')
			return(1);
	return(0);
}

rmdir(f, iflg)
char *f;
{
	int status, i;

	if(dotname(f))
		return(0);
	if(iflg) {
		printf("%s: ", f);
		if(!yes())
			return(0);
	}
	while((i=fork()) == -1)
		sleep(3);
	if(i) {
		wait(&status);
		return(status);
	}
	execl("/bin/rmdir", "rmdir", f, 0);
	execl("/usr/bin/rmdir", "rmdir", f, 0);
	printf("rm: can't find rmdir\n");
	exit(1);
}

yes()
{
	int i, b;

	i = b = getchar();
	while(b != '\n' && b != EOF)
		b = getchar();
	return(i == 'y');
}
