
/**********************************************************************
 *   Copyright (c) Digital Equipment Corporation 1984, 1985, 1986.    *
 *   All Rights Reserved. 					      *
 *   Reference "/usr/src/COPYRIGHT" for applicable restrictions.      *
 **********************************************************************/

/*
 *	Dmesg - write system out the system message buffer.
 *
 *	Usage: dmesage [-] [ corefile [ namelist ] ]
 *
 *	If the '-' flag is given, the it only prints out new
 *	stuff since the last time it was run with the '-' flag.
 *	Use alternate 'corefile' and 'namelist' if specified.
 */

static char Sccsid[] = "@(#)dmesg.c	3.0	4/21/86";
#include <stdio.h>
#include <sys/types.h>
#include <a.out.h>

char	*msgbuf;
char	*msgbufp;
int	msgbufs;
int	sflag;
int	of;
char	MSGBUF[] = "/usr/adm/msgbuf";	/* place for 'memory' between runs */

int	omindex;
char	*omsgbuf;
struct nlist nl[4] = {
	{"_msgbuf"},
#define			X_BUF	0
	{"_msgbufp"},
#define			X_BUFP	1
	{"_msgbufs"}
#define			X_BUFS	2
};

main(argc, argv)
char **argv;
{
	int mem;
	register char *mp, *omp, *mstart;

	if (argc>1 && argv[1][0] == '-') {
		sflag++;
		argc--;
		argv++;
		if ((of = open(MSGBUF, 2)) == -1) {
			close(creat(MSGBUF,664));
			of = open(MSGBUF, 2);
		}
		if (of == -1)
			error("open of %s failed", MSGBUF);
	}
	nlist(argc>2? argv[2]:"/unix", nl);
	if (nl[X_BUF].n_type==0)
		error("No namelist in %s", argc>2 ? argv[2] : "/unix");
	if ((mem = open((argc>1? argv[1]: "/dev/mem"), 0)) < 0)
		error("Cannot open %s", argc>1 ? argv[1] : "/dev/mem");

	/*
	 * Get the size of the system message buffer, and
	 * initalize the size dependent structures/arrays.
	 */
	lseek(mem, (long)nl[X_BUFS].n_value, 0);
	read(mem, &msgbufs, sizeof(msgbufs));
	omsgbuf = malloc(msgbufs);
	msgbuf = malloc(msgbufs);
	if (omsgbuf == NULL || msgbuf == NULL)
		error("can't malloc enough space for system message buffer");

	/*
	 * Grabbed the saved copies of
	 * omindex & omsgbuf if we need to.
	 */
	if (sflag) {
		lseek(of, 2L, 0);	/* for backwards compatability */
		read(of, (char *)&omindex, sizeof(omindex));
		read(of, omsgbuf, msgbufs);
	}

	/*
	 * Get the system message buffer and where
	 * in the system message buffer we are, and
	 * make sure the value returned is kosher.
	 */
	lseek(mem, (long)nl[X_BUF].n_value, 0);
	read(mem, msgbuf, msgbufs);
	lseek(mem, (long)nl[X_BUFP].n_value, 0);
	read(mem, (char *)&msgbufp, sizeof(msgbufp));
	if (msgbufp < (char *)nl[X_BUF].n_value ||
	    msgbufp >= (char *)nl[X_BUF].n_value+msgbufs)
		error("Namelist mismatch");
	msgbufp += msgbuf - (char *)nl[X_BUF].n_value;

	/*
	 * Get our starting point in the buffer.
	 */
	if (!sflag) {
		/*
		 * We don't have any memory without the sflag, so
		 * just blindly start where the kernel left off.
		 */
		mstart = msgbufp;
		pdate();
		printf("...\n");
	} else {
		/*
		 * Compare everything between where the kernel is now,
		 * and where we last left off.  If it's different, then
		 * the kernel wrapped around before we ran, so print out
		 * the three dots and set our starting point for the whole
		 * buffer.  If it's the same and where we last left off is
		 * where the kernel last left off, then nothing has been
		 * added so exit; otherwise we just start where we last
		 * left off.
		 */
		int samef = 1;

		mstart = &msgbuf[omindex];
		omp = &omsgbuf[msgbufp-msgbuf];
		mp = msgbufp;
		do {
			if (*mp++ != *omp++) {
				mstart = msgbufp;
				samef = 0;
				pdate();
				printf("...\n");
				break;
			}
			if (mp == msgbuf+msgbufs)
				mp = msgbuf;
			if (omp == omsgbuf+msgbufs)
				omp = omsgbuf;
		} while(mp != mstart);
		if (samef && mstart == msgbufp)
			exit(0);
	}

	/*
	 * Print out the date (if we haven't already)
	 * and all the new stuff in the buffer.
	 */
	pdate();
	mp = mstart;
	do {
		if (*mp)
			putchar(*mp);
		mp++;
		if (mp == msgbuf+msgbufs)
			mp = msgbuf;
	} while (mp != msgbufp);

	/*
	 * Write back out the updated info if we have to.
	 */
	if (sflag) {
		omindex = msgbufp - msgbuf;
		lseek(of, 2L, 0);	/* for backwards compatability */
		write(of, (char *)&omindex, sizeof(omindex));
		write(of, msgbuf, msgbufs);
	}
	exit(0);
}

/*
 * error - something went wrong.  If the sflag was set we are
 * probably being called from cron, so try to print the error
 * at the console; otherwise just tell Joe Blow about it.
 */

error(s, a1, a2, a3)
char *s;
char *a1, a2, a3;
{
	register int console;
	if (sflag && (console = open("/dev/console", 1)) != -1) {
		dup2(console, 1);
		close(console);
	}
	pdate();
	printf("dmesg: ");
	printf(s, a1, a2, a3);
	printf("\n");
	exit(1);
}

/*
 * Print the date the first time we are called, just
 * a null-op after that.
 */

pdate()
{
	extern char *ctime();
	static firstime;
	time_t tbuf;

	if (firstime==0) {
		firstime++;
		time(&tbuf);
		printf("\n%.12s\n", ctime(&tbuf)+4);
	}
}
