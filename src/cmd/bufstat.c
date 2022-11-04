
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

static char Sccsid[] = "@(#)bufstat.c	3.0	4/21/86";
#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <a.out.h>
#define	UCB_BHASH	/* for buf.h */
#define	MAPMOUNT	/* for buf.h */
#include <sys/buf.h>

FILE *dirf;
struct nlist nli[] = {
	"_buf", 0, 0,		/* 0 */
	"_io_info", 0, 0,	/* 1 */
	"_nbuf", 0, 0,		/* 2 */
	0, 0, 0,
 };

struct iostat {
	int	nbuf;
	long	nread;
	long	nreada;
	long	ncache;
	long	nwrite;
	long	bufcount[1];
}*io_info;
int infosize;

struct buf **bhdr;
struct direct dir;
struct stat sts;

struct {
	dev_t bdev;
	char bname[DIRSIZ];
}btype[257];

int mem, interval;
long addr;
char *infile = "/unix";
char *coref = "/dev/mem";
char *usage = "bufstat: usage  bufstat [ifn] [interval] [corefile] [namelist]\n";
char *flgs[14] = {
	"|DN",
	"|ERR",
	"|BSY",
	"|PHY",
	"|MAP",
	"|WNT",
	"|AGE",
	"|ASY",
	"|DLW",
	"|TAP",
	"|PBY",
	"|PAC",
	"|MNT",
	0
};

char	flgstr[40];
char devnam[64];
int	wflag = 0;

main(argc, argp)
int argc;
char *argp[];
{
	int cnt, cnt1;
	char *ap;

	argp++;
	for(ap = *argp++; ap && *ap; ap++)
		switch(*ap){
			case 'f':
				coref = *argp++;
				break;
			case 'i':
				interval = atoi(*argp++);
				break;
			case 'n':
				infile = *argp++;
				break;
			case '-':
				break;
			case 'w':
				++wflag;
				break;
			default:
				fprintf(stderr, "%s", usage);
				exit(1);
				break;
		}
	initmem();
	while(1){
		lbuf();
		bufsts();
		if(interval)
			sleep(interval);
		else
			exit(0);
	}
}

lbuf()
{
	int cnt;

	lseek(mem, (long)nli[1].n_value, 0);
	read(mem, io_info, infosize);
	lseek(mem, (long)nli[0].n_value, 0);
	for(cnt = 0; cnt < io_info->nbuf; cnt++)
		read(mem, bhdr[cnt], sizeof(struct buf));
}

bufsts()
{
	int cnt, fcnt, devpnt;
	struct buf *bufoff;

	printf("\nBuffer Pool Status\n\n");
	if (wflag)
		printf("        ");
	printf("Buffer    I/O ops  Byte    Logic\n");
	if (wflag)
		printf("Bufaddr ");
	printf("Address   per buf  count   blkno  err   ");
	printf("device         ");
	if (wflag)
		printf("b_forw  b_back  av_forw av_back ");
	printf("flags\n");
	printf("----------------------------------------");
	printf("---------------");
	if (wflag)
		printf("---------------------------------------");
	printf("------------------------\n");
	bufoff = (struct buf *)nli[0].n_value;
	for(cnt = 0; cnt < io_info->nbuf; cnt++){
		if (wflag)
			printf("0%-6o ", bufoff++);
		addr = (long)bhdr[cnt]->b_xmem << 16;
		addr += (unsigned)bhdr[cnt]->b_un.b_addr;
		printf("0%-8lo ", addr);
		printf("%-8ld ", io_info->bufcount[cnt]);
		printf("0%-6o ", bhdr[cnt]->b_bcount);
		addr = bhdr[cnt]->b_blkno;
		printf("0%-6D ", addr);
		printf("%-4o ", (unsigned)bhdr[cnt]->b_error);
		if((devpnt = find(cnt)) >= 0)
			printf("%-14s ", btype[devpnt].bname);
		else{
			if(bhdr[cnt]->b_dev < 0)
				printf("%-14s ", "system");
			else
				printf("%-2d %-11d ", major(bhdr[cnt]->b_dev),
				   minor(bhdr[cnt]->b_dev));
		}
		if((bhdr[cnt]->b_flags&B_READ) == 0)
			strcpy(flgstr, "WRT");
		else
			strcpy(flgstr, "RD");
		for(fcnt = 0; flgs[fcnt]; fcnt++){
			if(bhdr[cnt]->b_flags&(2<<fcnt))
				strcat(flgstr, flgs[fcnt]);
		}
		flgstr[25] = 0;
		if (wflag) {
			printf("0%-6o 0%-6o 0%-6o 0%-6o ",
				bhdr[cnt]->b_forw,
				bhdr[cnt]->b_back,
				bhdr[cnt]->av_forw,
				bhdr[cnt]->av_back);
		}
		printf("%-25s\n", flgstr);
			
	}
}

find(pnt)
{
	int cnt;

	for(cnt = 0; btype[cnt].bdev >= 0; cnt++){
		if(strcmp("swap", btype[cnt].bname) == 0)
			continue;
		if(btype[cnt].bdev == bhdr[pnt]->b_dev)
			return(cnt);
	}
	return(-1);
}

initmem()
{
	int cnt;

	nlist(infile, nli);
	for(cnt = 0; nli[cnt].n_name[0]; cnt++){
		if(nli[cnt].n_value == 0){
	printf("bufstat : FATAL : nlist failed for entry %s\r", nli[cnt].n_name);
			exit(0);
		}
	}
	if((mem = open(coref, 0)) <= 0){
		printf("bufstat: FATAL : could not open %s\r", coref);
		exit(0);
	}
	lseek(mem, (long)nli[2].n_value, 0);
	read(mem, &cnt, sizeof(cnt));		/* get nbuf... */
	infosize = sizeof(*io_info)+(cnt-1)*sizeof(long);
	io_info = malloc(infosize);
	if (io_info == NULL) {
		printf("bufstat : FATAL : io_info malloc failed\n");
		exit(1);
	}
	lseek(mem, (long)nli[1].n_value, 0);
	read(mem, io_info, infosize);	/* get nbuf, etc */

	bhdr = (struct buf **)malloc((io_info->nbuf+1) * sizeof(*bhdr));
	if (bhdr == NULL) {
		printf("bufstat : FATAL : bhdr malloc failed\n");
		exit(1);
	}
	/* allocate buffer header space */
	for(cnt = 0; cnt < io_info->nbuf; cnt++){
		if((bhdr[cnt] = malloc(sizeof(struct buf))) == 0){
	printf("bufstat : FATAL : buf malloc failed for entry %d\n", cnt);
			exit(1);
		}
	}

	/* get block device info */
	for(cnt=0; cnt<257; cnt++)
		btype[cnt].bdev = -1;
	if((dirf = fopen("/dev", "r")) != NULL){
		strcpy(&devnam, "/dev/");
		for(cnt = 0;;){
			if(cnt >= 256){
			printf("bufstat : WARNING : Too many block devices\n");
				break;
			}
			if(fread((char *)&dir, sizeof(dir), 1, dirf) != 1)
				break;
			if(dir.d_ino == 0 || strcmp(dir.d_name, ".") == 0
			  || strcmp(dir.d_name, "..") == 0)
				continue;
			strcpy(&devnam[5], dir.d_name);
			if(stat(&devnam, &sts)){
				continue;
			}
			if((sts.st_mode&S_IFMT) != S_IFBLK)
				continue;
			btype[cnt].bdev = sts.st_rdev;
			strcpy(btype[cnt++].bname, dir.d_name);
		}
	}else{
		printf("bufstat : WARNING : Could not open /dev\n");
	}
	fclose(dirf);
}
