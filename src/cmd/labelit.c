
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * labelit command from SYSTEM V
 *
 * Changes for ULTRIX-11:
 *
 *	Removed 3b processor and RT dependencies.
 *	Removed 1024 byte block file system dependencies.
 *	Added as many sanity checks as reasonable to do.
 *	Only write back the super-block to disk (not 10 blocks)!
 *	Ask user to comfirm label change before write.
 *
 * Fred Canter
 */
static char Sccsid[] = "@(#)labelit.c	3.0	4/21/86";
#include <stdio.h>
#include <sys/param.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/filsys.h>
#include <sys/ino.h>
#include <sys/devmaj.h>
#include <sys/mount.h>
#include <a.out.h>
#include <errno.h>

#define DEV 1
#define FSNAME 2
#define VOLUME 3
 /* write fsname, volume # on disk superblock */
struct {
	char fill1[BSIZE];
	union {
		char fill2[BSIZE];
		struct filsys fs;
	} f;
} super;

struct {
	char	t_magic[8];
	char	t_volume[6];
	char	t_reels,
		t_reel;
	long	t_time,
		t_length,
		t_dens;
	char	t_fill[484];
} Tape_hdr;

short	nflag;
short	magtape;
struct	stat	statb;

#define	LINESIZE	10
char	line[LINESIZE+1];

#define	NMNT	0
#define	MNT	1
struct	nlist	nl[] = {
	{ "_nmount" },
	{ "_mount" },
	{ "" },
};
int	nmount;
struct	mount	*mp;
struct	mount	*mpp;
char	*coref = "/dev/mem";

sigalrm()
{
	signal(SIGALRM, sigalrm);
}

main(argc, argv)
int	argc;
char	*argv[];
{
	int fsi, fso;
	long curtime;
	int i, j, mem;

	signal(SIGALRM, sigalrm);

	if(argc!=4 && argc!=2 && argc!=5)  {
usage:
		fprintf(stderr,"Usage: labelit /dev/r??? [fsname volume [-n]]\n");
		exit(2);
	}
	nflag = 0;
	if(stat(argv[DEV], &statb) < 0) {
		fprintf(stderr, "labelit: cannot stat %s\n", argv[DEV]);
		exit(2);
	}
	i = (statb.st_rdev >> 8) & 0377;
	if(i < RK_RMAJ) {
		fprintf(stderr, "labelit: %s not RAW disk or tape\n", argv[DEV]);
		exit(2);
	}
	switch(i) {
	case HT_RMAJ:
	case TS_RMAJ:
	case TM_RMAJ:
	case TK_RMAJ:
		magtape = 1;
		break;
	default:
		magtape = 0;
		break;
	}
	if(argc != 2)
		if((strlen(argv[FSNAME]) > 6) || (strlen(argv[VOLUME]) > 6)) {
			fprintf(stderr, "labelit: fsname/volname six characters maximum\n");
			exit(2);
		}
	if((argc != 2) && !magtape) {
		nlist("/unix", nl);
		if((nl[NMNT].n_value==0) || (nl[MNT].n_value==0)) {
			fprintf(stderr, "labelit: cannot access /unix namelist\n");
			exit(2);
		}
		mem = open(coref, 0);
		if(mem < 0 ) {
			fprintf(stderr, "labelit: cannot open %s\n", coref);
			exit(2);
		}
		lseek(mem, (long)nl[NMNT].n_value, 0);
		read(mem, (char *)&nmount, sizeof(nmount));
		mp = malloc(sizeof(struct mount) * nmount);
		lseek(mem, (long)nl[MNT].n_value, 0);
		read(mem, (char *)mp, sizeof(struct mount) * nmount);
		mpp = mp;
		for(i=0; i<nmount; i++, mpp++) {
			j = (statb.st_rdev >> 8) & 0377;
			j -= RK_RMAJ;	/* convert to block major */
			j <<= 8;
			j |= (statb.st_rdev & 0377);
			if((mpp->m_bufp == 0) || (mpp->m_dev != j))
				continue;
			fprintf(stderr, "\7\7\7labelit: SORRY - device mounted!\n");
			exit(2);
		}
	}
	if(argc==5) {
		if(strcmp(argv[4], "-n")!=0)
			goto usage;
		nflag++;
		if(magtape) {
			printf("Skipping label check!\n");
			goto do_it;
		}
	}

	if((fsi = open(argv[DEV],0)) < 0) {
		fprintf(stderr, "labelit: cannot open device (%s)\n", argv[DEV]);
		exit(2);
	}

	if(magtape) {
		alarm(5);
		if (read(fsi, &Tape_hdr, sizeof(Tape_hdr)) != sizeof(Tape_hdr)) {
			fprintf(stderr,"labelit: tape read error %d\n",errno);
			exit(2);
		}
		alarm(0);
		if(!(equal(Tape_hdr.t_magic, "Volcopy", 7)||
		    equal(Tape_hdr.t_magic,"Finc",4))) {
			fprintf(stderr, "labelit: tape not labeled!\n");
			exit(2);
		}
		printf("\n%s tape volume: %.6s, reel %d of %d reels\n",
			Tape_hdr.t_magic, Tape_hdr.t_volume, Tape_hdr.t_reel, Tape_hdr.t_reels);
		printf("Written: %s", ctime(&Tape_hdr.t_time));
		if(argc==2 && Tape_hdr.t_reel>1)
			exit(0);
	}
	if((i=read(fsi, &super, sizeof(super))) != sizeof(super))  {
		fprintf(stderr, "labelit: cannot read ");
		if(magtape)
			fprintf(stderr, "tape label\n");
		else
			fprintf(stderr, "superblock\n");
		exit(2);
	}

#define	S	super.f.fs
	printf("\nCurrent fsname: [%.6s], Current volname: [%.6s],\n",
		S.s_fname, S.s_fpack);
	printf("Blocks: %ld, Inodes: %u, FS Units: 1024b,\n",
		(long)S.s_fsize, (S.s_isize - 2) * INOPB);
	printf("Date last mounted: %s", ctime(&S.s_time));
	if(argc==2)
		exit(0);
do_it:
	printf("\nNEW fsname = [%.6s], NEW volname = [%.6s]\n",
		argv[FSNAME], argv[VOLUME]);
ask_it:
	if(nflag == 0) {
		printf("\nAre the new labels correct <y or n> ? ");
		for(i=0; i<LINESIZE; i++) {
			line[i] = getchar();
			if(line[i] == '\n') {
				line[i] = '\0';
				break;
			}
		}
		if((strcmp("y", line) == 0) || (strcmp("yes", line) == 0))
			goto ok_it;
		else if((strcmp("n", line) == 0) || (strcmp("no", line) == 0)) {
			fprintf(stderr, "labels not changed!\n");
			exit(0);
		} else
			goto ask_it;
	}
ok_it:
	sprintf(super.f.fs.s_fname, "%.6s", argv[FSNAME]);
	sprintf(super.f.fs.s_fpack, "%.6s", argv[VOLUME]);

	close(fsi);
	fso = open(argv[DEV],1);
	if(fso < 0) {
		fprintf(stderr, "labelit: cannot open %s for writing\n",
			argv[DEV]);
		exit(2);
	}
	if(magtape) {
		strcpy(Tape_hdr.t_magic, "Volcopy");
		sprintf(Tape_hdr.t_volume, "%.6s", argv[VOLUME]);
		if(write(fso, &Tape_hdr, sizeof(Tape_hdr)) < 0)
			goto cannot;
	}
	if(write(fso, &super, sizeof(super)) < 0) {
cannot:
		fprintf(stderr, "labelit: cannot write label\n");
	exit(2);
	}
	exit(0);
}
equal(s1, s2, ct)
char *s1, *s2;
int ct;
{
	register i;

	for(i=0; i<ct; ++i) {
		if(*s1 == *s2) {;
			if(*s1 == '\0') return(1);
			s1++; s2++;
			continue;
		} else return(0);
	}
	return(1);
}
