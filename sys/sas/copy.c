/*
 * SCCSID: @(#)copy.c	3.0	4/21/86
 */
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 standalone copy program (copy)
 *
 * This program is something like the DD command.
 * Its main use is for copying the RL02 distribution media
 * onto a backup pack to guard agaist disaster.
 * CAUTION, the record size must divide evenly
 * into the total size of the copy from and the copy to
 * devices, or the entire media may not be copied.
 *
 * Fred Canter 3/11/83
 */

#define	MAXBS	16384

char	buf[MAXBS];

char	ifile[50];
char	ofile[50];
char	line[50];

int	fi, fo;
int	rs;
int	rcnt;
long	boff;
long	i;

main()
{

	printf("\n\nULTRIX-11 Standalone Copy Program\n");
loop:
	printf("\nInput File: ");
	gets(ifile);
	printf("\nOutput File: ");
	gets(ofile);
getrs:
	printf("\nRecord Size <%d MAX> : ", MAXBS);
	gets(line);
	rs = atoi(line);
	if((rs <= 0) || (rs > MAXBS))
		goto getrs;
getrc:
	printf("\nNumber of Records: ");
	gets(line);
	rcnt = atoi(line);
	if(rcnt <= 0)
		goto getrc;
rtc:
	printf("\nReady to copy %s to %s <y or n> ? ", ifile, ofile);
	gets(line);
	if(line[0] == 'n')
		goto exloop;
	if(line[0] == 'y') {
		if((fi = open(ifile, 0)) < 0) {
			printf("\nCan't open %s\n", ifile);
			exit(1);
		}
		if((fo = open(ofile, 2)) < 0) {
			printf("\nCan't open %s\n", ofile);
			exit(1);
		}
		for(i=0; i<rcnt; i++) {
			boff = i * rs;
			lseek(fi, (long)boff, 0);
			if(read(fi, buf, rs) < 0) {
				printf("\nread error\n");
				exit(1);
			}
			lseek(fo, (long)boff, 0);
			if(write(fo, buf, rs) < 0) {
				printf("\nwrite error\n");
				exit(1);
			}
		}
		close(fi);
		close(fo);
		printf("\nCopy complete\n");
	} else
		goto rtc;
exloop:
	printf("\nMore files to copy <y or n> ? ");
	gets(line);
	if(line[0] == 'y')
		goto loop;
	else if(line[0] == 'n')
		exit(0);
	else
		goto exloop;
}
