
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

# include	"../hdr/defines.h"
# include	"sys/dir.h"

static char Sccsid[] = "@(#)dofile.c 3.0 4/22/86";

int	nfiles;
char	had_dir;
char	had_standinp;


do_file(p,func)
register char *p;
int (*func)();
{
	extern char *Ffile;
	char str[FILESIZE];
	char ibuf[FILESIZE];
	char	dbuf[BUFSIZ];
	FILE *iop;
	struct direct dir[2];
	register char *s;
	int fd;

	if (p[0] == '-') {
		had_standinp = 1;
		while (gets(ibuf) != NULL) {
			if (sccsfile(ibuf)) {
				Ffile = ibuf;
				(*func)(ibuf);
				nfiles++;
			}
		}
	}
	else if (exists(p) && (Statbuf.st_mode & S_IFMT) == S_IFDIR) {
		had_dir = 1;
		Ffile = p;
		if((iop = fopen(p,"r")) == NULL)
			return;
		setbuf(iop,dbuf);
		dir[1].d_ino = 0;
		fread(dir,sizeof(dir[0]),1,iop);   /* skip "."  */
		fread(dir,sizeof(dir[0]),1,iop);   /* skip ".."  */
		while(fread(dir,sizeof(dir[0]),1,iop) == 1) {
			if(dir[0].d_ino == 0) continue;
			sprintf(str,"%s/%s",p,dir[0].d_name);
			if(sccsfile(str)) {
				Ffile = str;
				(*func)(str);
				nfiles++;
			}
		}
		fclose(iop);
	}
	else {
		Ffile = p;
		(*func)(p);
		nfiles++;
	}
}
