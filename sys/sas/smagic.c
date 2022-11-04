static char Sccsid[] = "@(#)smagic.c	3.0	4/21/86";

/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 * ULTRIX-11 smagic program
 *
 * Changes the magic number of a stand alone program
 * from 0407 to 0401, identifies program as stand alone.
 * Jams the flag bits into the a_unused element of the
 * a.out header, flags help boot load the program.
 * SEE ALSO: a.out.h
 *
 * Usage:
 *	  smagic prog
 *
 * Fred Canter
 */

#include <a.out.h>

struct	exec	header;

main(argc, argv)
int	argc;
char	*argv[];
{
	register int fd, flags;
	register struct exec *hp;

	if(argc != 2) {
		printf("\nsmagic: bad arg count!\n");
		exit(1);
	}
	fd = open(argv[1], 2);
	if(fd < 0) {
		printf("\nsmagic: can't open %s!\n", argv[2]);
		exit(1);
	}
	if(read(fd, (char *)&header, sizeof(header)) != sizeof(header)) {
		printf("\nsmagic: read error!\n");
		exit(1);
	}
	hp = &header;
	if(hp->a_magic != 0407) {
		printf("\nsmagic: not a 0407 file!\n");
		exit(1);
	}
	if(hp->a_unused != 0) {
		printf("\nsmagic: a_unused not zero!\n");
		exit(1);
	}
	hp->a_magic = SA_MAGIC;
	if(strncmp("boot", argv[1], 4) == 0)
		flags = SA_BOOT;
	else if(strcmp("sdload", argv[1]) == 0)
		flags = SA_SDLOAD;
	else if(strcmp("mtload", argv[1]) == 0)
		flags = SA_SDLOAD;
	else if(strcmp("rlload", argv[1]) == 0)
		flags = SA_SDLOAD;
	else if(strcmp("rcload", argv[1]) == 0)
		flags = SA_SDLOAD;
	else if(strcmp("rxload", argv[1]) == 0)
		flags = SA_SDLOAD;
	else if(strcmp("rabads", argv[1]) == 0)
		flags = SA_RABADS;
	else if(strcmp("syscall", argv[1]) == 0)
		flags = SA_SYSCALL;
	else
		flags = SA_SCSEG;	/* all others (need syscall segment) */
	hp->a_unused = flags;
	lseek(fd, 0L, 0);
	if(write(fd, (char *)&header, sizeof(header)) != sizeof(header)) {
		printf("\nsmagic: write error!\n");
		exit(1);
	}
	close(fd);
}
