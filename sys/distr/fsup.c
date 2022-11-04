static	char	Sccsid[] = "@(#)fsup.c	3.0	4/21/86";

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 Binary version file system update (fsup).
 *
 * Fred Canter
 *
 * This program updates the ULTRIX-11 binary version file system
 * prototypes (bvroot & bvusr), by comparing the last modified
 * date/time of each file and getting a new copy of the file from
 * the master system disk if necessary.
 *
 * The program also reports on any differences between the prototype
 * file systems and the master system disk, such as changed owner,
 * protection codes, and missing/extra files.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

struct	stat	sb_from;
struct	stat	sb_to;
char	fi_from[100];
char	fi_to[100];
char	syscmd[100];

char	*bvroot = "/dev/hp10";
char	*bvusr = "/dev/hp11";
char	*bvos = "/dev/hp14";
char	*bvr_dir = "/bvroot";
char	*bvu_dir = "/bvusr";
char	*bvr_df = "./bvroot.dir";
char	*bvu_df	= "./bvusr.dir";
char	*bvo_df	= "./bvos.dir";
char	line[150];
char	buf[512];

int	stopflag;
int	allup;
int	noup;
int	iflag;
int	dflag;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	intr();
	char	*p;
	int	i;

	for(i=1; i<argc; i++) {
		if(argv[i][0] != '-') {
	argerr:
			fprintf(stderr, "\nfsup: bad argument!\n");
			exit(1);
		}
		switch(argv[i][1]) {
		case 'a':
			allup++;
			break;
		case 'i':
			iflag++;
			break;
		case 'n':
			noup++;
			break;
		case 'd':
			noup++;
			allup = 0;
			dflag++;
			break;
		default:
			goto argerr;
		}
	}
	fprintf(stderr, "\n\nULTRIX-11 ");
	fprintf(stderr, "BVROOT/BVUSR/BVOS file system update program\n");
	if(iflag)
		goto ud_go;
ud_rdy:
	fprintf(stderr, "\nReady to begin update <y or n> ? ");
	p = gets(line);
	if((strcmp(p, "y")==0) || (strcmp(p, "yes")==0))
		goto ud_go;
	else if((strcmp(p, "n")==0) || (strcmp(p, "no")==0)) {
		printf("\n\n###### BVROOT/BVUSR/BVOS NOT UPDATED ######\n");
		exit(0);
	} else
		goto ud_rdy;
ud_go:
	signal(SIGINT, intr);
	fs_update(bvroot, bvr_dir, bvr_df);
	fs_update(bvusr, bvu_dir, bvu_df);
	fs_update(bvos, bvu_dir, bvo_df);
}

/*
 * Do the actual update.
 * sf  = special file to mount
 * dir = directory to mount on
 * df  = file system proto directory file
 */

fs_update(sf, dir, df)
char	*sf;
char	*dir;
char	*df;
{
	register FILE *fi;
	register int i;
	int	s_from, s_to;
	int	fin, fout, cc;
	time_t	timeb;

	if(noup)
		i = 1;
	else
		i = 0;
	if(mount(sf, dir, i) != 0) {
		fprintf(stderr, "\nCan't mount %s on %s\n", sf, dir);
		exit(1);
	}
	if((fi = fopen(df, "r")) == NULL) {
		fprintf(stderr, "\nCan't open directory file (%s)\n", df);
	um_xit:
		if(fi > 0)
			fclose(fi);
		if(fin > 0)
			close(fin);
		if(fout > 0)
			close(fout);
		if(umount(sf) != 0)
			fprintf(stderr, "\nCan't dismount %s\n", sf);
		exit(1);
	}
	clearerr(fi);	/* some day, find out if this is really needed */
	time(&timeb);
	printf("\n\n###### STARTING UPDATE OF %s (%s) @ %s",
		dir, sf, ctime(&timeb));
loop:
	if(stopflag) {
		printf("\n\n****** UPDATE ABORTED VIA CTRL/C ******\n");
		goto um_xit;
	}
	if(fgets(line, 100, fi) == NULL) {
		if(ferror(fi)) {
			fprintf(stderr, "\n\nREAD ERROR on %s file\n", df);
			goto um_xit;
		}
		if(feof(fi)) {
			printf("\n\n###### UPDATE OF %s (%s) COMPLETED @ %s",
				dir, sf, ctime(&timeb));
			fclose(fi);
			umount(sf);
			return;
		}
	}
	if(line[0] == '*') {
		printf("\n****** FILE IGNORED ******\n");
		printf("%s", line);
		goto loop;
	}
	for(i=0; i<150; i++)
		if((line[i] == '\n') || (line[i] == ' ')) {
			line[i] = '\0';
			break;
		}
	if(i >= 100) {
		fprintf(stderr, "\nPathname too long!\n");
		goto um_xit;
	}
	strcpy(fi_to, line);
	if(strcmp(dir, "/bvroot") == 0) {
		if(strcmp("/bvroot", line) == 0)
			sprintf(fi_from, "/");
		else
			strcpy(fi_from, &line[7]);
	} else {
		fi_from[0] = '/';
		strcpy(&fi_from[1], &line[3]);
	}
	s_from = stat(fi_from, &sb_from);
	s_to = stat(fi_to, &sb_to);
	if(s_to != 0) {
		fprintf(stderr, "\nCan't stat %s\n", fi_to);
		goto um_xit;
	}
	if(s_from != 0) {
		printf("\n****** MISSING FILE/DIRECTORY ******\n");
		printf("%s\n", fi_from);
		fflush(stdout);
		goto loop;
	}
	if(!dflag)
		goto no_diff;
	if((sb_to.st_mode & S_IFMT) != S_IFDIR)
		goto no_diff;
	if((sb_from.st_mode & S_IFMT) != S_IFDIR) {
		printf("\n** NOT A DIRECTORY: %s\n", fi_from);
		fflush(stdout);
		goto loop;
	}
	sprintf(&syscmd, "ls %s >fsup.dirf; ls %s >fsup.dirt",
		fi_from, fi_to);
	if(system(&syscmd) != 0) {
		printf("\n** Can't execute %s\n", syscmd);
		fflush(stdout);
		goto loop;
	}
	printf("\n** DIRECTORY CHECK: ");
	printf("[ < for %s ]  [ > for %s ]\n", fi_from, fi_to);
	fflush(stdout);
	system("diff fsup.dirf fsup.dirt");
	goto d_stat;
no_diff:
	if((s_to == 0) && ((sb_to.st_mode&S_IFMT) != S_IFREG)) {
		printf("\n****** ONLY REGULAR FILES UPDATED ******\n");
		printf("%s\n", fi_to);
		goto loop;
	}
	if((s_from == 0) && ((sb_from.st_mode&S_IFMT) != S_IFREG)) {
		printf("\n****** ONLY REGULAR FILES UPDATED ******\n");
		printf("%s\n", fi_from);
		goto loop;
	}

d_stat:
	if((sb_from.st_mode != sb_to.st_mode) ||
	    (sb_from.st_nlink != sb_to.st_nlink) ||
	    (sb_from.st_uid != sb_to.st_uid) ||
	    (sb_from.st_gid != sb_to.st_gid)) {
		printf("\n****** FILE STATUS MISMATCH ******\n");
		printf("%s\n", fi_from);
		printf("(mode=%o, nlink=%d, uid=%d, gid=%d)\n",
		  sb_from.st_mode,sb_from.st_nlink,sb_from.st_uid,sb_from.st_gid);
		printf("%s\n", fi_to);
		printf("(mode=%o, nlink=%d, uid=%d, gid=%d)\n",
		  sb_to.st_mode,sb_to.st_nlink,sb_to.st_uid,sb_to.st_gid);
	}
	if(dflag)
		goto loop;
	if((s_to == 0) && (sb_to.st_size == 0)) {
		printf("\n****** ZERO LENGTH FILE NOT UPDATED ******\n");
		printf("%s\n", fi_to);
		goto loop;
	}
	if(allup || (sb_from.st_mtime > sb_to.st_mtime)) {
		if(noup) {
			printf("\n****** FILE WOULD BE UPDATED ******\n");
			printf("FROM:\t%s\nTO:\t%s\n", fi_from, fi_to);
			goto loop;
		}
		if(!allup) {
			printf("\n****** FILE BEING UPDATED ******\n");
			printf("FROM:\t%s\nTO:\t%s\n", fi_from, fi_to);
		}
		if((fin = open(fi_from, 0)) < 0) {
			fprintf(stderr, "\nCan't open %s\n", fi_from);
			goto um_xit;
		}
		if((fout = creat(fi_to, 0666)) < 0) {
			fprintf(stderr, "\nCan't create %s\n", fi_to);
			goto um_xit;
		}
		while(cc = read(fin, buf, 512)) {
			if(cc < 0) {
				fprintf(stderr,"\nFATAL read error\n");
				goto um_xit;
			}
			if(write(fout, buf, cc) != cc) {
				fprintf(stderr,"\n FATAL write error\n");
				goto um_xit;
			}
		}
	close(fin);
	close(fout);
	if(!allup)
		printf("****** FILE UPDATE COMPLETE ******\n");
	}
	goto loop;
}

intr()
{
	signal(SIGINT, intr);
	stopflag = 1;
}
